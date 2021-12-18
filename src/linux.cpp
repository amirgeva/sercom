#ifdef UNIX

#include "impl.h"
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

namespace sercom
{

	class SerialLinux : public SerialImpl
	{
		int				m_Handle;
		struct termios	m_Params; 

		unsigned get_baud_value(unsigned rate)
		{
#define BAUD(x) case x: return B##x
			switch (rate) {
				BAUD(1200);
				BAUD(2400);
				BAUD(4800);
				BAUD(9600);
				BAUD(19200);
				BAUD(38400);
				BAUD(57600);
				BAUD(115200);
			default:
				throw std::runtime_error(S("Invalid baud rate: " << rate));
			}
#undef BAUD
		}

	public:
		SerialLinux(std::string port, unsigned baud_rate)
		{
			while (true)
			{
				m_Handle = open(port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
				if (m_Handle>=0) break;
				if (errno == EINTR) continue;
				throw std::runtime_error(S("Could not open port " << port));
			}
			if (tcgetattr(m_Handle, &m_Params) == -1)
				throw std::runtime_error(S("Port " << port << " not found"));
			m_Params.c_cflag |= CLOCAL | CREAD;
			m_Params.c_lflag &= ~(IEXTEN | ISIG | ECHO | ECHOE | ICANON | ECHOK | ECHONL);
			m_Params.c_oflag &= ~OPOST;
			m_Params.c_iflag &= ~(IGNCR | INLCR | ICRNL | IGNBRK | IUCLC);
			auto baud=get_baud_value(baud_rate);
			::cfsetispeed(&m_Params, baud);
    		::cfsetospeed(&m_Params, baud);
			m_Params.c_cflag &= ~(CSIZE | CSTOPB | PARENB | PARODD);
			m_Params.c_cflag |= CS8;
			m_Params.c_iflag &= ~(INPCK | ISTRIP);
			m_Params.c_iflag &= ~(IXON | IXOFF);
			m_Params.c_cflag &= ~CRTSCTS;
			m_Params.c_cc[VMIN]=0;
			m_Params.c_cc[VTIME]=0;
			tcsetattr(m_Handle,TCSANOW, &m_Params);
		}

		~SerialLinux()
		{
			if (is_open())
			{
				close(m_Handle);
			}
		}

		virtual bool is_open() const override
		{
			return m_Handle>=0;
		}

		virtual size_t data_available() const override
		{
			if (!is_open()) return 0;
			int n=0;
			if (ioctl (m_Handle, TIOCINQ, &n) < 0)
				throw std::runtime_error("Failed to check port status");
			return n;
		}

		virtual size_t send(const uint8_t* data, size_t len) override
		{
			return write(m_Handle,data,len);
		}
		
		virtual size_t receive(uint8_t* data, size_t max_len) override
		{
			fd_set fds;
			FD_ZERO(&fds);
			FD_SET(m_Handle, &fds);
			timeval timeout;
			timeout.tv_sec=1;
			timeout.tv_usec=0;
			int rc=select(1,&fds,0,0,&timeout);
			if (rc>0)
			{
				size_t available = data_available();
				max_len = std::min(max_len, available);
				return read(m_Handle,data,max_len);
			}
			return 0;
		}
	};

	std::shared_ptr<Serial> Serial::create(std::string port, unsigned baud_rate)
	{
		return std::make_shared<SerialLinux>(port, baud_rate);
	}

} // namespace sercom

#endif // UNIX

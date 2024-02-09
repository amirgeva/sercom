#ifdef UNIX

#include <iostream>
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
				m_Handle = open(port.c_str(), O_RDWR);
				if (m_Handle>=0) break;
				if (errno == EINTR) continue;
				throw std::runtime_error(S("Could not open port " << port));
			}
			if (tcgetattr(m_Handle, &m_Params) != 0)
				throw std::runtime_error(S("Port " << port << " not found"));
			m_Params.c_cflag &= ~(PARENB | CSTOPB | CSIZE | CRTSCTS);
			m_Params.c_cflag |= CS8 | CREAD | CLOCAL;
			m_Params.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHONL | ISIG);
			m_Params.c_iflag &= ~(IXON | IXOFF | IXANY | IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
			m_Params.c_oflag &= ~(OPOST | ONLCR);
			auto baud=get_baud_value(baud_rate);
			if (::cfsetispeed(&m_Params, baud) != 0)
				throw std::runtime_error(S("Failed to set input speed"));
			if (::cfsetospeed(&m_Params, baud) != 0)
				throw std::runtime_error(S("Failed to set output speed"));
			if (tcsetattr(m_Handle,TCSANOW, &m_Params) != 0)
				throw std::runtime_error(S("Failed to set port attributes"));
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
			return read(m_Handle, data, max_len);
		}
	};

	std::shared_ptr<Serial> Serial::create(std::string port, unsigned baud_rate)
	{
		return std::make_shared<SerialLinux>(port, baud_rate);
	}

} // namespace sercom

#endif // UNIX

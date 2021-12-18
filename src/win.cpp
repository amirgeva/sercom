#ifdef WIN32

#include "impl.h"
#include <Windows.h>

namespace sercom
{

	class SerialWindows : public SerialImpl
	{
		HANDLE	m_Handle;
		DCB		m_Params;

		DWORD get_baud_value(unsigned rate)
		{
#define BAUD(x) case x: return CBR_##x
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
		SerialWindows(std::string port, unsigned baud_rate)
		{
			const std::string prefix = "\\\\.\\";
			if (port.substr(0, prefix.size()) != prefix)
				port = prefix + port;
			m_Handle = CreateFileA(port.c_str(), GENERIC_WRITE | GENERIC_READ,
				0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
			if (m_Handle == INVALID_HANDLE_VALUE)
			{
				if (GetLastError() == ERROR_FILE_NOT_FOUND)
					throw std::runtime_error(S("Port " << port << " not found"));
				throw std::runtime_error("Unknown error");
			}
			ZeroMemory(&m_Params, sizeof(m_Params));
			m_Params.DCBlength = sizeof(m_Params);
			if (!GetCommState(m_Handle, &m_Params))
				throw std::runtime_error("Failed to retrieve port settings");
			m_Params.BaudRate = get_baud_value(baud_rate);
			m_Params.ByteSize = 8;
			m_Params.StopBits = 1;
			m_Params.Parity = NOPARITY;
			m_Params.fOutxCtsFlow = false;
			m_Params.fRtsControl = RTS_CONTROL_DISABLE;
			m_Params.fInX = false;
			m_Params.fOutX = false;
			if (!SetCommState(m_Handle, &m_Params))
				throw std::runtime_error("Failed to set port settings");
			COMMTIMEOUTS timeouts;
			timeouts.ReadIntervalTimeout = 10;
			timeouts.WriteTotalTimeoutConstant = 1000;
			timeouts.ReadTotalTimeoutConstant = 1000;
			timeouts.ReadTotalTimeoutMultiplier = 0;
			timeouts.WriteTotalTimeoutMultiplier = 0;
			if (!SetCommTimeouts(m_Handle, &timeouts))
				throw std::runtime_error("Failed to set timeouts");
		}

		~SerialWindows()
		{
			if (is_open())
			{
				CloseHandle(m_Handle);
			}
		}

		virtual bool is_open() const override
		{
			return m_Handle != INVALID_HANDLE_VALUE;
		}

		virtual size_t data_available() const override
		{
			if (!is_open()) return 0;
			COMSTAT stat;
			if (!ClearCommError(m_Handle, 0, &stat))
				throw std::runtime_error("Failed to check port status");
			return stat.cbInQue;
		}

		virtual size_t receive(uint8_t* data, size_t max_len) override
		{
			if (!is_open()) return 0;
			DWORD act;
			if (!ReadFile(m_Handle, data, max_len, &act, 0))
				throw std::runtime_error("Failed to read port");
			return act;
		}

		virtual size_t send(const uint8_t* data, size_t len) override
		{
			if (!is_open()) return 0;
			size_t total = 0;
			while (len > 0)
			{
				DWORD act;
				if (!WriteFile(m_Handle, data, len, &act, 0))
					throw std::runtime_error("Failed to write to port");
				total += act;
				data += act;
				len -= act;
			}
			return total;
		}
	};

	std::shared_ptr<Serial> Serial::create(std::string port, unsigned baud_rate)
	{
		return std::make_shared<SerialWindows>(port, baud_rate);
	}

} // namespace sercom


#endif

#pragma once

#include <sercom.h>
#include <stdexcept>
#include <sstream>

class string_accumulator : public std::string
{
public:
	template<typename T>
	string_accumulator& operator<< (const T& t)
	{
		std::ostringstream os;
		os << t;
		*this += os.str();
		return *this;
	}
};

#define S(x) (string_accumulator() << x)

namespace sercom
{

	class SerialImpl : public Serial
	{
		bool	m_Terminate;
	public:
		virtual void terminate() override
		{
			m_Terminate = true;
		}

		virtual size_t send(const std::vector<uint8_t>& data) override
		{
			return ((Serial*)this)->send(&data[0], data.size());
		}

		virtual size_t receive(std::vector<uint8_t>& data) override
		{
			if (data.empty()) return 0;
			size_t res = ((Serial*)this)->receive(&data[0], data.size());
			data.resize(res);
			return res;
		}

		virtual size_t receive_all(uint8_t* data, size_t len) override
		{
			size_t total = 0;
			while (len > 0 && !m_Terminate)
			{
				size_t act = ((Serial*)this)->receive(data, len);
				total += act;
				data += act;
				len -= act;
			}
			return total;
		}

		virtual size_t receive_all(std::vector<uint8_t>& data) override
		{
			if (data.empty()) return 0;
			return receive_all(&data[0], data.size());
		}
	};

} // namespace sercom

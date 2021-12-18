
SERial COMmunication Library
----------------------------

This is a simple cross platform serial communication library, 
with a minimal interface and most settings set to popular defaults (e.g. N81)


Simple Usage Example:


	#include <iostream>
	#include <stdexcept>
	#include <sercom.h>


	int main(int argc, char* argv[])
	{
		try
		{
			auto ser = sercom::Serial::create("COM5", 115200);
			if (!ser->is_open())
			{
				std::cerr << "Port not open.\n";
			}
			else
			{
				std::string msg = "Hello";
				std::vector<uint8_t> buffer(msg.begin(), msg.end());
				std::cout << "Sent " << ser->send(buffer) << " bytes.\n";
				buffer.resize(64);
				std::cout << "Received " << ser->receive(buffer) << " bytes.\n";
				for (const auto& b : buffer)
				{
					std::cout << int(b) << std::endl;
				}
			}
		}
		catch (const std::runtime_error& e)
		{
			std::cerr << e.what() << std::endl;
		}
		return 0;
	}

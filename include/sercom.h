#pragma once

#include <memory>
#include <string>
#include <vector>

namespace sercom
{

	class Serial
	{
	public:
		virtual ~Serial() = default;

		virtual bool is_open() const = 0;

		virtual size_t data_available() const = 0;

		// Call to abort a waiting receive_all when
		// closing the port
		virtual void terminate() = 0;

		// Send len bytes
		virtual size_t send(const uint8_t* data, size_t len) = 0;
		
		// Send the contents of the vector
		virtual size_t send(const std::vector<uint8_t>& data) = 0;
		
		// Receive at most max_len bytes.
		// Return as soon as some data is received
		virtual size_t receive(uint8_t* data, size_t max_len) = 0;

		// Receive data to fill the vector (caller sets vector size).
		// Return as soon as some data is received, and vector is resized accordingly
		virtual size_t receive(std::vector<uint8_t>& data) = 0;

		// Same as receive, but wait for len bytes
		// Returns len if received before the timeout, 
		// otherwise returns actual received bytes
		virtual size_t receive_all(uint8_t* data, size_t len) = 0;

		// Same as receive, but wait to fill vector
		// Returns size of the vector if received before the timeout, 
		// otherwise returns actual received bytes
		virtual size_t receive_all(std::vector<uint8_t>& data) = 0;

		static std::shared_ptr<Serial> create(std::string port, unsigned baud_rate);
	};



} // namespace sercom


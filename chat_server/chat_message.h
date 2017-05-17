#ifndef CHAT_MESSAGE_HPP
#define CHAT_MESSAGE_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>

class chat_message
{
public:
	enum { header_length = 4 };
	enum { max_body_length = 512 };

	chat_message()
		: body_length_(0)
	{
	}

	const char* data() const
	{
		return data_;
	}

	char* data()
	{
		return data_;
	}

	std::size_t length() const
	{
		return header_length + body_length_ + name_length + 2;
	}

	const char* body() const
	{
		return data_ + header_length;
	}

	char* body()
	{
		return data_ + header_length;
	}

	std::size_t body_length() const
	{
		return body_length_;
	}

	void body_length(std::size_t new_length)
	{
		body_length_ = new_length;
		if (body_length_ > max_body_length)
			body_length_ = max_body_length;
	}

	bool decode_header()
	{
		char header[header_length + 1] = "";
		std::strncat(header, data_, header_length);
		body_length_ = std::atoi(header);
		if (body_length_ > max_body_length)
		{
			body_length_ = 0;
			return false;
		}
		return true;
	}

	void encode_header()
	{
		char header[header_length + 1] = "";
		std::sprintf(header, "%4d", static_cast<int>(body_length_));
		std::memcpy(data_, header, header_length);
	}

	void rewriteData(char* arr1, int sizeName, char* arr2, int sizeBody)
	{
		std::cout << "namesize: " << sizeName << " ";
		name_length = sizeName;
		int sizeNameBody = sizeName + sizeBody + 2;
		char header[header_length + 1] = "";
		char znaki[3] = ": ";

		std::sprintf(header, "%4d", static_cast<int>(sizeNameBody));
		std::memcpy(dataWithName_, header, sizeof(header));
		strncat(dataWithName_, arr1, sizeName);
		strncat(dataWithName_, znaki, 2);

		strncat(dataWithName_, arr2, sizeBody);
		std::cout << "rewrite 3: " << dataWithName_;
	}

	char* dataWithName()
	{
		return dataWithName_;
	}


private:
	char data_[header_length + max_body_length];
	char dataWithName_[header_length + max_body_length];
	std::size_t body_length_;
	int name_length;
};

#endif // CHAT_MESSAGE_HPP
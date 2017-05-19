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
		return header_length + body_length_ + name_length + num;
	}

	const char* body() const
	{
		return data_ + header_length;
	}

	char* body()
	{
		return data_ + header_length;
	}

	char* dataWithName()
	{
		return dataWithName_;
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
		num = 2;
		name_length = sizeName;// применяется в функции length()
		int sizeNameBody = sizeName + sizeBody + 2;
		char header[header_length + 1] = "";
		char znaki[3] = ": ";

		std::sprintf(header, "%4d", static_cast<int>(sizeNameBody));
		std::memcpy(dataWithName_, header, sizeof(header));

		strncat(dataWithName_, arr1, sizeName);
		strncat(dataWithName_, znaki, 2);
		strncat(dataWithName_, arr2, sizeBody);
	}

	void rewriteDataForInfo(char* info, int size)
	{
		num = 0;
		body_length_ = size;
		name_length = 0;
		char header[header_length + 1] = "";
		std::sprintf(header, "%4d", static_cast<int>(size));
		std::memcpy(dataWithName_, header, sizeof(header));
		strncat(dataWithName_, info, size);
	}

	void rewriteDataForList(std::list <char*> nameList, int sizeName, std::list<char*>::iterator it, int i)
	{
		char number[3];
		char name[30];
		char point[2] = ".";
		name_length = sizeName;
		num = 2;
		body_length_ = 0;
		
		memcpy(name, *it, sizeName);
		std::sprintf(number, "%d", static_cast<int>(i));
		strncat(number, point, 1);

		char header[header_length + 1] = "";
		std::sprintf(header, "%4d", static_cast<int>(sizeName+2));
		std::memcpy(dataWithName_, header, sizeof(header));
		strncat(dataWithName_, number, sizeof(number));
		strncat(dataWithName_, name, sizeName);
	}

private:
	char data_[header_length + max_body_length];
	char dataWithName_[header_length + max_body_length];
	std::size_t body_length_;
	int name_length;
	int num;
};

#endif // CHAT_MESSAGE_HPP
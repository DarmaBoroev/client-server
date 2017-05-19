#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <boost/asio.hpp>
#include <cstring>
#include "chat_message.h"
#include <vector>

using boost::asio::ip::tcp;

  //----------------------------------------------------------------------

typedef std::deque<chat_message> chat_message_queue;

//----------------------------------------------------------------------

class chat_participant
{
public:
	virtual ~chat_participant() {}
	virtual void deliver(const chat_message& msg) = 0;
};

typedef std::shared_ptr<chat_participant> chat_participant_ptr;
std::list<char*> clientsName;
std::list<int> name_length;

//----------------------------------------------------------------------

class chat_room
{
public:
	void join(chat_participant_ptr participant)
	{
		participants_.insert(participant);
		for (auto msg : recent_msgs_)
			participant->deliver(msg);
	}

	void leave(chat_participant_ptr participant)
	{
		participants_.erase(participant);
	}

	void deliver(const chat_message& msg)
	{
		recent_msgs_.push_back(msg);
		while (recent_msgs_.size() > max_recent_msgs)
			recent_msgs_.pop_front();

		for (auto participant : participants_)
			participant->deliver(msg);
	}

private:
	std::set<chat_participant_ptr> participants_;
	enum { max_recent_msgs = 100 };
	chat_message_queue recent_msgs_;
};

//----------------------------------------------------------------------

class chat_session
	: public chat_participant,
	public std::enable_shared_from_this<chat_session>
{
public:
	chat_session(tcp::socket socket, chat_room& room)
		: socket_(std::move(socket)),
		room_(room)
	{
	}

	~chat_session()
	{
		char info[40] = "Info: Disabled ";
		strncat(info, name, sizeName);
		read_msg_.rewriteDataForInfo(info, (15+sizeName));
		room_.deliver(read_msg_);
		clientsName.remove(name);
		name_length.remove(sizeName);
	}

	void start()
	{
		room_.join(shared_from_this());
		char info[27] = "Info: Connected new client";
		read_msg_.rewriteDataForInfo(info, 26);
		room_.deliver(read_msg_);
		do_read_header();
	}

	void deliver(const chat_message& msg)
	{
		bool write_in_progress = !write_msgs_.empty();
		write_msgs_.push_back(msg);
		if (!write_in_progress)
		{
			do_write();
		}
	}

private:
	void do_read_header()
	{
		auto self(shared_from_this());
		boost::asio::async_read(socket_,
			boost::asio::buffer(read_msg_.data(), chat_message::header_length),
			[this, self](boost::system::error_code ec, std::size_t /*length*/)
		{
			if (!ec && read_msg_.decode_header())
			{
				do_read_body();
			}
			else
			{
				room_.leave(shared_from_this());
			}
		});
	}

	void do_read_body()
	{
		auto self(shared_from_this());
		boost::asio::async_read(socket_,
			boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
			[this, self](boost::system::error_code ec, std::size_t /*length*/)
		{
			if (!ec)
			{
				if (*read_msg_.body() == '#')
				{
					sizeName = read_msg_.body_length();
					iDel(read_msg_.body(), sizeName, 1);
					memcpy(name, read_msg_.body(), sizeName);
					name_length.push_back(sizeName);
					clientsName.push_back(name);
					do_read_header();
					check = false;
				}
				else if (*read_msg_.body() == '@' && *(read_msg_.body()+1)=='l')
				{
					if (clientsName.empty())
					{
						char info[20] = "Info: No one online";
						read_msg_.rewriteDataForInfo(info, 19);
						room_.deliver(read_msg_);
					}
					else
					{
						std::list<char*>::iterator it1;
						std::list<int>::iterator it2;
						it1 = clientsName.begin();
						it2 = name_length.begin();
						for (int i = 1; i <= clientsName.size(); i++)
						{
							read_msg_.rewriteDataForList(clientsName, *it2, it1++, i);
							it2++;
							room_.deliver(read_msg_);
						}
					}
					do_read_header();
				}
				else
				{
					if (check)
					{
						char info[83] = "Info: You did not enter your username!Please, enter your username.(Example: #Ivan)";
						read_msg_.rewriteDataForInfo(info, 83);
						room_.deliver(read_msg_);
						do_read_header();
					}
					else
					{
						int sizeBody = read_msg_.body_length();
						read_msg_.rewriteData(name, sizeName, read_msg_.body(), sizeBody);
						room_.deliver(read_msg_);
						do_read_header();
					}
				}
			}
			else
			{
				room_.leave(shared_from_this());
			}
		});
	}

	void iDel(char *array, int &lenAr, int nom)
	{
	
		for (int ix = nom - 1; ix < lenAr - 1; ix++)
		{
			array[ix] = array[ix + 1];
		}
		lenAr--;
	}

	void do_write()
	{
		auto self(shared_from_this());
		boost::asio::async_write(socket_,
			boost::asio::buffer(write_msgs_.front().dataWithName(),
			write_msgs_.front().length()),
			[this, self](boost::system::error_code ec, std::size_t /*length*/)
		{
			if (!ec)
			{
				write_msgs_.pop_front();
				if (!write_msgs_.empty())
				{
					do_write();
				}
			}
			else
			{
				room_.leave(shared_from_this());
			}
		});
	}

	tcp::socket socket_;
	chat_room& room_;
	chat_message read_msg_;
	chat_message_queue write_msgs_;
	char name[30];
	int sizeName = 0;
	bool check = true;
};

//----------------------------------------------------------------------

class chat_server
{
public:
	chat_server(boost::asio::io_service& io_service,
		const tcp::endpoint& endpoint)
		: acceptor_(io_service, endpoint),
		socket_(io_service)
	{
		do_accept();
	}

private:
	void do_accept()
	{
		acceptor_.async_accept(socket_,
			[this](boost::system::error_code ec)
		{
			if (!ec)
			{
				std::make_shared<chat_session>(std::move(socket_), room_)->start();
			}

			do_accept();
		});
	}

	tcp::acceptor acceptor_;
	tcp::socket socket_;
	chat_room room_;
};

//----------------------------------------------------------------------

int main()
{
	try
	{
		boost::asio::io_service io_service;

		std::list<chat_server> servers;
		tcp::endpoint endpoint(tcp::v4(), 4000);
		servers.emplace_back(io_service, endpoint);
		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
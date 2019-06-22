#include "socket.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>

#include <iostream>
#include <thread>
#include <list>
#include <algorithm>
#include <string>
#include <sstream>
#include <vector>
#include <queue>
#include <mutex>
#include <iomanip>

#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/shm.h>

#include "tools.h"
#include "session.h"

const char RESET[]       = "\033[0m"        ;
const char BLACK[]       =        "\033[30m";
const char RED[]         =        "\033[31m";
const char GREEN[]       =        "\033[32m";
const char YELLOW[]      =        "\033[33m";
const char BLUE[]        =        "\033[34m";
const char MAGENTA[]     =        "\033[35m";
const char CYAN[]        =        "\033[36m";
const char WHITE[]       =        "\033[37m";
const char BOLDBLACK[]   = "\033[1m\033[30m";
const char BOLDRED[]     = "\033[1m\033[31m";
const char BOLDGREEN[]   = "\033[1m\033[32m";
const char BOLDYELLOW[]  = "\033[1m\033[33m";
const char BOLDBLUE[]    = "\033[1m\033[34m";
const char BOLDMAGENTA[] = "\033[1m\033[35m";
const char BOLDCYAN[]    = "\033[1m\033[36m";
const char BOLDWHITE[]   = "\033[1m\033[37m";

class commandBuffer
{
private:
	static const char delimiter = ';';
	std::string sqlBuffer;
	char inQuote; // '\0' - none, '\'' - single, '\"' - double
	bool waitEscape;
	size_t position;
public:
	commandBuffer(): sqlBuffer(""), inQuote('\0'), waitEscape(false), position(0) {}
	void append(std::string raw) { sqlBuffer += raw; }
	bool hasSql()
	{
		while (position < sqlBuffer.length())
		{
			char& c = sqlBuffer[position];
			if (waitEscape) waitEscape = false;
			else if (c == delimiter && !inQuote) return true;
			else
			{
				if (c == '\'' || c == '\"')
				{
					if (!inQuote) inQuote = c;
					else if (inQuote == c) inQuote = '\0';
				}
				else if (c == '\n') c = ' ';
				else if (c == '\\') waitEscape = true;
			}
			++position;
		}
		return false;
	}
	std::string sql(bool includeDelimiter)
	{
		if (hasSql())
		{
			std::string ret = sqlBuffer.substr(0, position + int(includeDelimiter)); // 视情况加或不加分号
			sqlBuffer = sqlBuffer.substr(position + 1);
			position = 0;
			return ret;
		}
		else return "";
	}
};

const unsigned bufferSize = 1024;
const int connectionTimeout = 5; // Close connection after 60s timeout

const char endChar = '\x03';

bool bExit = false;

void cpuBreak()
{
	usleep(200000); // Sleep for 200ms
}

void sqlAbort(int signo)
{
	bExit = true;
}

std::string getTime(const char* format = "[%s] ")
{
	time_t rawtime;
	tm* timeinfo;
	char buf[64], fmt[64];
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	sprintf(fmt, "%04d/%2d/%2d %2d:%02d:%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	sprintf(buf, format, fmt);
	return buf;
}

// ==================== ^ Common ^ ==================== //
// ==================== v Server v ==================== //

std::map<std::string, OOPD::DataBase*> *yourDatabase = nullptr;

struct conn_t
{
	int conn;
	commandBuffer buffer;
	OOPD::Session session;
	conn_t(int conn): conn(conn), session(*yourDatabase) {}
};

struct yoursqlQuery
{
	conn_t& conn;
	std::string sql;
	yoursqlQuery(conn_t& conn, std::string sql): conn(conn), sql(sql) {}
};

int s;
sockaddr_in servaddr;
socklen_t len;

std::vector<conn_t> conns;
std::queue<yoursqlQuery> queries;
std::map<int, unsigned> connMap;

std::mutex queriesMutex;

void getConnection()
{
	while (!bExit)
	{
		int conn = accept(s, (sockaddr*)(&servaddr), &len);
		conns.push_back(conn);
		connMap[conn] = conns.size() - 1;
		std::clog << getTime() << CYAN << "New connection: " << GREEN << conn << RESET << std::endl;
	}
}

void getData()
{
	timeval tv;
	tv.tv_sec = 2;
	tv.tv_usec = 0;
	while (!bExit)
	{
		for (auto it = conns.begin(); it != conns.end(); ++it)
		{
			fd_set rfds;
			FD_ZERO(&rfds);
			int maxfd = 0;
			int retval = 0;
			FD_SET(it->conn, &rfds);
			maxfd = std::max(maxfd, it->conn);
			retval = select(maxfd + 1, &rfds, NULL, NULL, &tv);
			if (retval == -1)
				std::cerr << getTime() << RED << "Error occurred in select().\n" << RESET;
			else if (retval == 0)
				/* std::clog << "No message received. Waiting...\n" */;
			else
			{
				char buf[1024];
				memset(buf, 0, sizeof(buf));
				int len = recv(it->conn, buf, sizeof(buf), 0);
				if (len > 0 && buf[len - 1] == endChar) buf[--len] = '\0';
				if (len > 0) // Only response if bytes received is greater than zero
				{
					it->buffer.append(buf);

					for (std::string sql = it->buffer.sql(false); sql != ""; sql = it->buffer.sql(false))
					{
						std::clog << getTime() << GREEN << "Received" << YELLOW << std::setw(4) << it->conn << RESET << ' ' << sql << std::endl;
						queriesMutex.lock();
						queries.push(yoursqlQuery(conns[connMap[it->conn]], sql));
						queriesMutex.unlock();
					}
				}
			}
		}
		cpuBreak();
	}
}

void execute()
{
	while (!bExit)
	{
		queriesMutex.lock();
		if (!queries.empty()) {
			yoursqlQuery query = queries.front();
			queries.pop();
			queriesMutex.unlock();

			std::stringstream os("");
			std::string output("");
			try
			{
				query.conn.session.execute(query.sql, os);
				output = os.str() + endChar;
			}
			catch (const char e[])
			{
				output = RED + std::string(e) + '\n' + RESET + endChar;
			}
			catch (std::string e)
			{
				output = RED + e + '\n' + RESET + endChar;
			}

			std::clog << getTime() << GREEN << "Executed" << YELLOW << std::setw(4) << query.conn.conn << ' ' << RESET << query.sql << std::endl;

			send(query.conn.conn, output.c_str(), output.length(), 0);
		}
		else
			queriesMutex.unlock();
		cpuBreak();
	}
}

int startServer(std::map<std::string, OOPD::DataBase*>& databases, std::string serverIp, uint16_t port)
{
	std::clog << getTime() << GREEN << "Initializing..." << RESET << std::endl;

	yourDatabase = &databases;

	// New socket
	s = socket(AF_INET, SOCK_STREAM, 0);
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = inet_addr(serverIp.c_str());
	if (bind(s, (sockaddr*)(&servaddr), sizeof(servaddr)) == -1)
		throw "Cannot assign address to socket";
	if (listen(s, 20) == -1)
		throw "Cannot listen on port " + std::to_string(port);
	len = sizeof(servaddr);

	// Thread : while ==>> accept
	std::thread t(getConnection);
	t.detach();
	// Thread : input ==>> execute
	std::thread t1(execute);
	t1.detach();
	// Thread : recv ==>> queue =(t1)=>> execute
	std::thread t2(getData);
	t2.detach();

	std::clog << getTime() << "Server is running at " << YELLOW << serverIp << RESET << ":" << GREEN << std::to_string(port) << RESET << ". Press " << RED << "Ctrl+C" << RESET << " to stop." << std::endl;

	signal(SIGINT, sqlAbort);
	while (!bExit) cpuBreak();

	std::clog << getTime() << MAGENTA << "Bye" << RESET << std::endl;
	return 0;
}

// ==================== ^ Server ^ ==================== //
// ==================== v Client v ==================== //

int sock_cli;
fd_set rfds;
timeval tv;
int retval, maxfd;

commandBuffer clientBuffer;

void fetchData()
{
	while (!bExit)
	{
		// 把可读文件描述符的集合清空
		FD_ZERO(&rfds);
		// 把标准输入的文件描述符加入到集合中
		FD_SET(0, &rfds);
		maxfd = 0;
		// 把当前连接的文件描述符加入到集合中
		FD_SET(sock_cli, &rfds);
		// 找出文件描述符集合中最大的文件描述符
		maxfd = std::max(maxfd, sock_cli);
		// 设置超时时间
		tv.tv_sec = 5;
		tv.tv_usec = 0;
		// 等待服务器发来的消息
		retval = select(maxfd + 1, &rfds, NULL, NULL, &tv);
		if (retval == -1)
		{
			std::cerr << getTime() << RED << "Error occurred in select(). Exiting...\n" << RESET;
			break;
		}
		else if (retval == 0)
		{
			/* std::clog << "No data sent or received. Waiting...\n" */;
			continue;
		}
		else
		{
			if (FD_ISSET(sock_cli, &rfds))
			{
				// 服务器发来了消息
				char recvbuf[bufferSize];
				memset(recvbuf, 0, sizeof(recvbuf));
				int len = recv(sock_cli, recvbuf, bufferSize, 0);
				bool flag;
				if (len > 0 && recvbuf[len - 1] == endChar) recvbuf[--len] = '\0', flag = true;
				std::cout << recvbuf;
				if (flag) break;
			}
		}
		cpuBreak();
	}
}

void fetchInput()
{
	for (bool flag = false; !std::cin.eof() && !clientBuffer.hasSql() && !bExit; flag = true)
	{
		std::cout << (flag ? "      -> " : "yoursql> ") << std::flush;
		std::string rawTemp;
		getline(std::cin, rawTemp);
		if (!isatty(fileno(stdin)))
			std::cout << rawTemp << std::endl;
		clientBuffer.append(rawTemp + ' ');
	}
	if (std::cin.eof()) bExit = true;
}

void clientRoutine()
{
	std::string sql;
	while (!std::cin.eof() && !bExit)
	{
		fetchInput();
		if (bExit) break;
		while ((sql = trimString(clientBuffer.sql(false))) != "")
		{
			if (stringToLower(sql) == "exit")
			{
				bExit = true;
				return;
			}
			sql.push_back(';');
			for (unsigned i = 0; i < sql.length(); i += bufferSize)
				send(sock_cli, sql.c_str() + i, std::min((unsigned long)bufferSize, sql.length() - i), 0);
			fetchData();
		}
	}
}

int startClient(std::string serverIp, uint16_t port)
{
	// 定义sockfd
	sock_cli = socket(AF_INET, SOCK_STREAM, 0);
	// 定义sockaddr_in
	sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port); // 服务器端口
	servaddr.sin_addr.s_addr = inet_addr(serverIp.c_str()); // 服务器ip

	//连接服务器，成功返回0，错误返回-1
	if (connect(sock_cli, (sockaddr*)(&servaddr), sizeof(servaddr)) < 0)
		throw "Cannot connect server at " + serverIp + ':' + std::to_string(port);

	std::clog << getTime() << "Connected to server at " << YELLOW << serverIp << RESET << ':' << GREEN << port << RESET << ". Press " << RED << "Ctrl+D" << RESET << " (i.e. EOF) or enter '" << RED << "exit;" << RESET << "' to exit." << std::endl;

	signal(SIGINT, sqlAbort);
	clientRoutine();
	close(sock_cli);
	std::cout << MAGENTA << "Bye" << RESET << std::endl;
	return 0;
}

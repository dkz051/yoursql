#include <iostream>
#include <string>
#include <exception>
#include <cstdint>
#include <cstring>
#include <cstdlib>

#include "OOPDB.h"
#include "socket.h"
#include "tools.h"

#include "session.h"

enum role_t { local, client, server };

role_t mode = role_t::local;
std::string serverIp(defaultIp);
uint16_t port = 2333;

static dbSet yourDatabase;

int main(int argc, char *argv[])
{
	for (int i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--server") == 0)
			mode = role_t::server;
		else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--client") == 0)
			mode = role_t::client;
		else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--local") == 0)
			mode = role_t::local;
		else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--ip") == 0)
		{
			if (i + 1 < argc)
				serverIp = argv[++i];
			else
				throw "Server IP address not specified with -i or --ip option";
		}
		else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0)
		{
			if (i + 1 < argc)
			{
				try
				{
					port = std::stoi(argv[++i]);
				}
				catch (std::exception)
				{
					throw "Invalid port value";
				}
			}
			else
				throw "Server port not specified with -p or --port option";
		}
		else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
		{
			std::cout <<
				"Usage: main [options]\n"
				"Options:\n"
				"  -s, --server\t\tStart a new YourSQL server instance.\n"
				"  -c, --client\t\tRun YourSQL client.\n"
				"  -l, --local\t\tStart a local interactive YourSQL instance (default).\n"
				"  -i, --ip\t\tSpecify IP address to connect to. Valid only in client\n"
				"\t\t\tmode. (default: localhost)\n"
				"  -p, --port <port>\tSpecify TCP port to listen on or connect to. Valid in\n"
				"\t\t\tserver mode or client mode. (default: 2333)\n"
				"  -h, --help\t\tDisplay this help message and exit.\n";
			exit(0);
		}
	}

	try
	{
		if (mode == role_t::local) // Local database
		{
			OOPD::Session session(yourDatabase);
			session.start();
		}
		else if (mode == role_t::client) // Connect to the server
			startClient(serverIp, port);
		else if (mode == role_t::server) // Run local server
			startServer(yourDatabase, serverIp, port);
	}
	catch (std::string e)
	{
		std::cerr << e << std::endl;
	}
	return 0;
}

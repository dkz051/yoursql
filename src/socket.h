#pragma once

#include <string>
#include <cstdint>

#include "OOPDB.h"

int startServer(OOPD::OOPDB& database, std::string serverIp, uint16_t port = 2333);
int startClient(std::string serverIp, uint16_t port = 2333);

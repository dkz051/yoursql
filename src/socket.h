#pragma once

#include <string>
#include <cstdint>

#include "tools.h"

int startServer(dbSet& databases, std::string serverIp, uint16_t port = 2333);
int startClient(std::string serverIp, uint16_t port = 2333);

#pragma once

#include <string>
#include <cstdint>

#include "tools.h"
#include "DataBase.h"

int startServer(std::map<std::string, OOPD::DataBase*>& databases, std::string serverIp, uint16_t port = 2333);
int startClient(std::string serverIp, uint16_t port = 2333);

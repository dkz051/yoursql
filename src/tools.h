#ifndef TOOLS_H
#define TOOLS_H

#include <string>
#include <vector>
#include <map>

#include "DataBase.h"

typedef std::vector<std::string> tokens; // 保存被分词的 MySQL 语句，包含原 MySQL 语句中的每个 token
typedef std::vector<std::string> attrs; // 保存若干个列的名称。虽然与 tokens 本质相同，但在程序中的语义不同

typedef std::map<std::string, OOPD::DataBase*> dbSet; // 数据库的集合

extern const std::string hiddenPrimaryKey; // 没有主键时自动插入一个默认主键 (int, auto_increment)；该名称由于带了特殊符号因此不会出现在输入中

extern const char defaultIp[];

std::vector<std::string> tokenize(std::string raw); // 对语句进行分词
std::string stringToLower(std::string str); // 将字符串中所有大写字母转换为小写
std::string stringToUpper(std::string str); // 将字符串中所有大写字母转换为大写

std::string parseStringLiteral(std::string str); // 解析字符串字面值
std::string toStringLiteral(std::string str); // 将字符串转化为字面值

std::string trimString(std::string str);

std::string concatenate(std::vector<std::string> tokens, char delimeter = ' ');

#endif

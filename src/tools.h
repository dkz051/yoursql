#ifndef TOOLS_H
#define TOOLS_H

#include <string>
#include <vector>
#include <map>

typedef std::vector<std::string> tokens; // 保存被分词的 MySQL 语句，包含原 MySQL 语句中的每个 token
typedef std::vector<std::string> attrs; // 保存若干个列的名称。虽然与 tokens 本质相同，但在程序中的语义不同
typedef std::pair<int, int> interval; // 整数区间

extern const std::string hiddenPrimaryKey; // 没有主键时自动插入一个默认主键 (int, auto_increment)；该名称由于带了特殊符号因此不会出现在输入中

extern const char defaultIp[];

namespace OOPD
{
	enum sort_t { ascending, descending };
	struct order_t
	{
		std::string field;
		sort_t sort;
	};
	struct group_t
	{
		std::string field;
		std::string function;
	};
}

typedef std::vector<OOPD::group_t> groups; // 定义分组操作函数（aggregate functions）
typedef std::vector<OOPD::order_t> orders; // 定义排序方法

tokens tokenize(std::string raw); // 对语句进行分词

std::vector<interval> selectClauses(tokens tokenLower); // 切分 SELECT 语句为六个子句

int length(interval a); // 返回区间的长度clau

std::string stringToLower(std::string str); // 将字符串中所有大写字母转换为小写
std::string stringToUpper(std::string str); // 将字符串中所有大写字母转换为大写

std::string parseStringLiteral(std::string str); // 解析字符串字面值
std::string toStringLiteral(std::string str); // 将字符串转化为字面值

std::string trimString(std::string str);

std::string concatenate(std::vector<std::string> tokens, char delimiter = ' ');

#endif

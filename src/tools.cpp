#include "tools.h"

#include <string>
#include <vector>
#include <algorithm>

const std::string hiddenPrimaryKey = "^HiddenKey";

const char defaultIp[] = "127.0.0.1";

std::string trimString(std::string str)
{
	unsigned left = 0, right = str.length();
	while (left < str.length() && (str[left] == ' ' || str[left] == '\n')) ++left;
	if (left == str.length()) return "";
	while (right >= left && str[right - 1] == ' ' || str[right - 1] == '\n') --right;
	return str.substr(left, right - left);
}

// 对 MySQL 语句进行分词
// 分词后的 tokens 放进一个 std::vector<std::string> 中后返回
std::vector<std::string> tokenize(std::string raw)
{
	raw.push_back('\n');
	std::vector<std::string> res;
	std::string current;
	bool inString = false;
	char stringDelimiter; // 处理到字符串字面量中间时，该 stringDelimiter 变量保存所用的包围字符串的符号，为单引号或双引号

	auto split = [&]()
	{
		if (!current.empty())
		{
			res.push_back(std::move(current));
			current = "";
		}
	};
	for (int i = 0; i < raw.length(); ++i)
	{
		if (inString)
		{
			if (raw[i] == '\n')
				throw "String literal missing terminating character";
			current += raw[i];
			if (raw[i] == stringDelimiter)
			{
				if (raw[i + 1] == stringDelimiter)
				{
					current += stringDelimiter;
					i++;
				}
				else // 字符串字面量结束
				{
					inString = false;
				}
			}
			if (raw[i] == '\\' && raw[i + 1] != '\n')
			{
				i++;
				current += raw[i];
			}
			continue;
		}
		// 不在字符串字面量中；在空白字符处分词
		if (raw[i] == ' ' || raw[i] == '\t' || raw[i] == '\n')
		{
			split();
			continue;
		}
		// 字符串字面量起始判断
		if (raw[i] == '\'' || raw[i] == '\"')
		{
			split();
			inString = true;
			stringDelimiter = raw[i];
			current = raw[i];
			continue;
		}
		// 减号（负号，连字符）、数字、大小写字母、下划线、小数点的连续组合被认为是一整个 token（可能是数字或标识符的名称）
		// "[-_0-9A-Za-z\.]+"
		if (isdigit(raw[i]) || isalpha(raw[i]) || raw[i] == '_' ||
			raw[i] == '-' || raw[i] == '.')
		{
			current += raw[i];
			continue;
		}
		// 其它特殊字符，分词
		split();
		res.push_back({raw[i]});
	}
	return res;
}

// 解析字符串字面值
std::string parseStringLiteral(std::string str)
{
	std::string ans("");
	if ((str[0] != '\'' && str[0] != '\"') || str[0] != str.back()) // 字符串字面量必须以相同的引号开始结束
		throw "Unrecognized string literal";
	for (unsigned k = 1; k < str.size() - 1; ++k)
	{
		if (str[k] == '\\') // 转义字符
		{
			++k;
			if (str[k] == '0') ans.push_back('\0'); // 六个需要特殊处理的转义字符
			else if (str[k] == 'b') ans.push_back('\b');
			else if (str[k] == 'n') ans.push_back('\n');
			else if (str[k] == 'r') ans.push_back('\r');
			else if (str[k] == 't') ans.push_back('\t');
			else if (str[k] == 'Z') ans.push_back('\x1a');
			else ans.push_back(str[k]); // 其他情况取反斜杠后面的符号即可
		}
		else if (str[k] == str[0])
		{
			if (k + 1 == str.size() - 1 || str[k + 1] != str[0]) // 两个相同引号转义为一个（参考 MySQL 文档）
				throw "Unrecognized string literal";
			ans.push_back(str[++k]);
		}
		else
			ans.push_back(str[k]);
	}
	return ans;
}

// 将字符串转化为字面值
std::string toStringLiteral(std::string str)
{
	std::string ans("\'");
	for (unsigned k = 0; k < str.size(); ++k)
	{
		if (str[k] == '\\') ans.push_back('\\'), ans.push_back('\\');
		else if (str[k] == '\0') ans.push_back('\\'), ans.push_back('0');
		else if (str[k] == '\b') ans.push_back('\\'), ans.push_back('b');
		else if (str[k] == '\n') ans.push_back('\\'), ans.push_back('n');
		else if (str[k] == '\r') ans.push_back('\\'), ans.push_back('r');
		else if (str[k] == '\t') ans.push_back('\\'), ans.push_back('t');
		else if (str[k] == '\x1a') ans.push_back('\\'), ans.push_back('Z');
		else if (str[k] == '\'') ans.push_back('\\'), ans.push_back('\'');
		else if (str[k] == '\"') ans.push_back('\\'), ans.push_back('\"');
		else if (str[k] == '%') ans.push_back('\\'), ans.push_back('%');
		else if (str[k] == '_') ans.push_back('\\'), ans.push_back('_');
		else ans.push_back(str[k]);
	}
	ans.push_back('\'');
	return ans;
}

// 将字符串 str 中的大写字母转换为小写，并返回转换后的字符串
std::string stringToLower(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	return str;
}

// 将字符串 str 中的大写字母转换为大写，并返回转换后的字符串
std::string stringToUpper(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::toupper);
	return str;
}

// 连接多个字符串，用 delimiter 隔开
std::string concatenate(std::vector<std::string> tokens, char delimiter)
{
	std::string ans = "";
	for (auto iter = tokens.begin(); iter != tokens.end(); ++iter)
	{
		if (iter != tokens.begin()) ans += delimiter;
		ans += *iter;
	}
	return ans;
}

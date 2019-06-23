#include "data_t.h"
#include "tools.h"

// 构造空 (NULL) 对象
DataInt::DataInt(): value(0) {}
DataDouble::DataDouble(): value(0.0) {}
DataString::DataString(): value("") {}

// 根据 C++ int 或 MySQL 字面值构造 DataInt 对象
DataInt::DataInt(int value): value(value) {}
DataInt::DataInt(std::string value): value(std::stoi(value)) {}

// 根据 C++ double 或 MySQL 字面值构造 DataDouble 对象
DataDouble::DataDouble(double value): value(value) {}
DataDouble::DataDouble(std::string value): value(std::stod(value)) {}

// 根据单个 MySQL 字面值构造 DataString 对象
// TODO 注意！当前不能通过 C++ std::string 变量的值（而非 MySQL 字符串字面值）直接构造 DataString 对象
DataString::DataString(std::string str)
{
	value = parseStringLiteral(str);
}

// data_t 的复制构造函数
DataInt::DataInt(const DataInt& o): value(o.value) {}
DataDouble::DataDouble(const DataDouble& o): value(o.value) {}
DataString::DataString(const DataString& o): value(o.value) {}

bool data_t::operator>(const data_t& b) const
{
	return b < *this;
}

bool data_t::operator==(const data_t& b) const
{
	return !(*this < b) && !(b < *this);
}

bool data_t::operator<=(const data_t& b) const
{
	return !(*this > b);
}

bool data_t::operator>=(const data_t& b) const
{
	return !(*this < b);
}

bool data_t::operator!=(const data_t& b) const
{
	return (*this < b) || (b < *this);
}

// 用于简化小于号运算符编写的辅助宏，具体用法可参考下面的小于号的写法
//
// 假设比较表达式为 a<b。此处 dataType 表示 b 的类型，例如 DataInt、DataDouble。
// 如果 b 实际不为指定的类型，那么什么都不做。
//
// convertFunction 是一个将 value 值转换为实际参与比较的值的函数，
// 类似于 JavaScript 中 ToPrimitive 函数做的事情。在类似于「大小写不敏感的字符串大小比较」等场合下非常方便。
// 如果不需要转换，直接使用 value 的原始值，省略即可。
//
// 务必注意后面的 #undef compareHelper 预编译指令（防止命名污染），比较运算请一定在 #define 和 #undef 之间定义
#define compareHelper(dataType, convertFunction) \
{ \
	const dataType* dvar = dynamic_cast<const dataType*>(&b); \
	if (dvar) \
		return convertFunction(value) < convertFunction(dvar->value); \
}

// 小于号比较运算符，其中小于号左边为 DataInt 对象。
// DataInt 只能与 DataInt 和 DataDouble 相比较。
bool DataInt::operator<(const data_t &b) const
{
	compareHelper(DataInt, );
	compareHelper(DataDouble, double); // 由于 C++ 内建支持 int 和 double 的比较，这里实际上不需要转换。这种写法是一个示例。
	throw "Compare: data type mismatch"; // b 既不是 DataInt 也不是 DataDouble，就没法比较了
}

// 小于号比较运算符，其中小于号左边为 DataDouble 对象。
// DataDouble 只能与 DataInt 和 DataDouble 相比较。
bool DataDouble::operator<(const data_t &b) const
{
	compareHelper(DataInt, double);
	compareHelper(DataDouble, );
	throw "Compare: data type mismatch";
}

// 小于号比较运算符，其中小于号左边为 DataString 对象。
// DataString 只能与 DataString 相比较。
bool DataString::operator<(const data_t &b) const
{
	compareHelper(DataString, ); // 由大作业题目要求，此处字符串比较区分大小写，与 MySQL 默认情形不同
	throw "Compare: data type mismatch";
}

#undef compareHelper

// 将 DataInt 对象的内容转换为字符串并返回
std::string DataInt::get() const
{
	return std::to_string(value);
}

// 将 DataDouble 对象的内容转换为字符串并返回
std::string DataDouble::get() const
{
	return std::to_string(value);
}

// 将 DataString 对象的内容（本身就是字符串）返回
std::string DataString::get() const
{
	return value;
}

// 根据字面值构造 data_t 对象，并返回指向构造的对象的指针
// TODO 此处假定输入的字面值都是合法的（对于不合法的字面值，引发问题的部分会被忽略）
data_t* data_t::fromLiteral(std::string str)
{
	if (str[0] == '\'' || str[0] == '\"') // 引号打头，即为字符串
		return new DataString(str);
	else
	{
		if (str.length() <= 10 || (str.length() == 11 && str[0] == '-')) // int 类型的十进制字面量长度在 10（非负）或 11（负）以内
		{
			long long value = 0, flag = 1;
			for (unsigned i = 0; i < str.length(); ++i)
			{
				if (str[i] == '-' && i == 0) // 开头负号
					flag = -flag;
				else if (!isdigit(str[i])) // 非数字字符
				{
					value = 1ll << 60; // 1ll << 60 是个 magic number，它显然不能在 int 范围内装下，break 后会自动按 double 返回
					break;
				}
				else
					(value *= 10) += str[i] - '0'; // 拼接数位
			}
			value *= flag; // 处理符号
			if (value >= (int)0x80000000 && value <= (int)0x7fffffff)
				return new DataInt(str); // int
		}
		return new DataDouble(str); // double
	}
}

data_t* data_t::fromRaw(std::string raw)
{
	int dot = 0, nondigit = 0;
	double value = 0, flag = 1;
	for (unsigned i = 0; i < raw.length(); ++i)
	{
		if (raw[i] == '-' && i == 0) // 开头负号
			flag = -flag;
		else if (raw[i] == '.') // 小数点
			++dot;
		else if (!isdigit(raw[i])) // 非数字字符
			++nondigit;
		else
			(value *= 10) += raw[i] - '0'; // 拼接数位
	}
	value *= flag; // 处理符号
	if (dot == 0 && nondigit == 0 && value >= (int)0x80000000 && value <= (int)0x7fffffff)
		return new DataInt(raw); // int
	else if (nondigit == 0 && dot <= 1)
		return new DataDouble(raw); // double
	else
		return new DataString(toStringLiteral(raw)); // string
}

// 获取一个新的、具有相同值的 DataInt 对象
DataInt* DataInt::copy()
{
	return new DataInt(*this);
}

// 获取一个新的、具有相同值的 DataDouble 对象
DataDouble* DataDouble::copy()
{
	return new DataDouble(*this);
}

// 获取一个新的、具有相同值的 DataString 对象
DataString* DataString::copy()
{
	return new DataString(*this);
}

data_t::~data_t() {}

std::ostream& operator<<(std::ostream& o, const data_t& data)
{
	return o << data.get();
}

bool YourSqlData::operator<(const YourSqlData& b) const
{
	if (ptr == nullptr || b.ptr == nullptr)
		return false;
	return *ptr < *b.ptr;
}

bool YourSqlData::operator>(const YourSqlData& b) const
{
	if (ptr == nullptr || b.ptr == nullptr)
		return false;
	return *ptr > *b.ptr;
}

bool YourSqlData::operator==(const YourSqlData& b) const
{
	if (ptr == nullptr || b.ptr == nullptr)
		return false;
	return *ptr == *b.ptr;
}

bool YourSqlData::operator<=(const YourSqlData& b) const
{
	if (ptr == nullptr || b.ptr == nullptr)
		return false;
	return *ptr <= *b.ptr;
}

bool YourSqlData::operator>=(const YourSqlData& b) const
{
	if (ptr == nullptr || b.ptr == nullptr)
		return false;
	return *ptr >= *b.ptr;
}

bool YourSqlData::operator!=(const YourSqlData& b) const
{
	if (ptr == nullptr || b.ptr == nullptr)
		return false;
	return *ptr != *b.ptr;
}

YourSqlData::YourSqlData(): ptr(nullptr) {}

YourSqlData::YourSqlData(data_t* ptr): ptr(ptr) {}

YourSqlData::YourSqlData(const YourSqlData& b)
{
	if (b.ptr != nullptr) ptr = b.ptr->copy();
	else ptr = nullptr;
}

YourSqlData::YourSqlData(YourSqlData&& b): ptr(b.ptr)
{
	b.ptr = nullptr;
}

bool YourSqlData::isNull() const
{
	return ptr == nullptr;
}

YourSqlData& YourSqlData::operator=(const YourSqlData& b)
{
	delete ptr;
	if (b.ptr != nullptr) ptr = b.ptr->copy();
	else ptr = nullptr;
	return *this;
}

YourSqlData& YourSqlData::operator=(YourSqlData&& b)
{
	ptr = b.ptr;
	b.ptr = nullptr;
	return *this;
}

std::ostream& operator<<(std::ostream& o, const YourSqlData& data)
{
	if (data.ptr == nullptr) return o << "NULL";
	else return o << data.ptr->get();
}

YourSqlData::~YourSqlData()
{
	delete ptr;
	ptr = nullptr;
}

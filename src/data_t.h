#ifndef DATA_T_H
#define DATA_T_H

// 如果做完所有需求后还有时间的话就把第一阶段的类型系统移植过来（但每个数据类型都增加 isNull 字段），从而消除 magic number

#include <string>
#include <iostream>

class data_t // 封装数据库支持的基本数据类型
{
public:
	static data_t* fromLiteral(std::string str); // 根据字面值构造相应的 data_t 派生类对象
	static data_t* fromRaw(std::string raw); // 根据字符串表示形式分析出类型，并构造 data_t 派生类对象
	virtual data_t* copy() = 0; // 复制当前 data_t 的内容到新的对象
	virtual bool operator<(const data_t&) const = 0; // 大小比较运算符
	virtual bool operator>(const data_t&) const;
	virtual bool operator==(const data_t&) const;
	virtual bool operator<=(const data_t&) const;
	virtual bool operator>=(const data_t&) const;
	virtual bool operator!=(const data_t&) const;
	virtual std::string get() const = 0; // 获取保存的数据的字符串表示
	virtual ~data_t();
};

class DataInt: public data_t
{
	friend class DataDouble; // 为支持 DataInt 和 DataDouble 之间的大小比较
private:
	int value;
public:
	DataInt();
	DataInt(int);
	DataInt(std::string);
	DataInt(const DataInt&);
	DataInt* copy();
	bool operator<(const data_t&) const;
	using data_t::operator>;
	using data_t::operator==;
	virtual std::string get() const;
};

class DataDouble: public data_t
{
	friend class DataInt; // 为支持 DataInt 和 DataDouble 之间的大小比较
private:
	double value;
public:
	DataDouble();
	DataDouble(double);
	DataDouble(std::string);
	DataDouble(const DataDouble&);
	DataDouble* copy();
	bool operator<(const data_t&) const;
	using data_t::operator>;
	using data_t::operator==;
	virtual std::string get() const;
};

class DataString: public data_t
{
private:
	std::string value;
public:
	DataString();
	DataString(std::string);
	DataString(const DataString&);
	DataString* copy();
	bool operator<(const data_t&) const;
	using data_t::operator>;
	using data_t::operator==;
	virtual std::string get() const;
};

std::ostream& operator<<(std::ostream& o, const data_t& data);

class YourSqlData // 包装 data_t 的指针，可以直接扔进 STL map/set 中
{
public:
	data_t* ptr;
	YourSqlData();
	YourSqlData(data_t* ptr);
	YourSqlData(const YourSqlData& b);
	YourSqlData(YourSqlData&& b);
	bool isNull() const;
	bool operator<(const YourSqlData&) const;
	bool operator>(const YourSqlData&) const;
	bool operator==(const YourSqlData&) const;
	bool operator<=(const YourSqlData&) const;
	bool operator>=(const YourSqlData&) const;
	bool operator!=(const YourSqlData&) const;
	YourSqlData& operator=(const YourSqlData&);
	YourSqlData& operator=(YourSqlData&&);
	~YourSqlData();
};

std::ostream& operator<<(std::ostream& o, const YourSqlData& data);

#endif

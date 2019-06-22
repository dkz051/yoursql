#ifndef OPERATE_H
#define OPERATE_H
//-----------------------------------------//
#include <map>
#include <list>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>

#include "tools.h"
#include "DataBase.h"
//-----------------------------------------//
//Operate类是对数据的上层操作的集合，它不直接访问OOPDB类中的数据，而是依靠Controller类调用其方法时传入相应数据库或数据表的引用
namespace OOPD
{
	class Operate
	{
	public:
		//-----数据库层面的操作，名称以DB开头-----//
		//新建一个数据库，传入参数为DataBase容器的引用，以及数据库的名称//返回true表示成功新建，否则表示已有同名数据库
		bool DBCreate(std::map<std::string, DataBase*>& target, std::string& DBname);
		//删除指定数据库，传入参数同上，若不存在指定数据库则返回false//返回true表示删除成功，否则表示没有此数据库
		bool DBDelete(std::map<std::string, DataBase*>& target, std::string& DBname);
		//DBSelect，即更改当前选中数据库的功能，由Controller类自行实现
		//void DBSelect();
		//展示全部数据库，传入参数为DataBase容器的引用
		void DBShow(std::map<std::string, DataBase*>& target, std::ostream& o = std::cout);

		//-----数据表层面的操作，名称以Table开头-----//
		//新建一个数据表，传入参数为当前活动DataBase的引用，数据表的名称、(各列的名称及数据类型、是否设置为NOT NULL、是否设置为主键)在一个结构体中
		//返回true表示成功新建，否则表示已有同名数据库
		bool TableCreate(DataBase& target, std::string& tableName, std::vector<TableCreateAttr>& attr);
		//删除一个数据表，传入参数为当前活动DataBase的引用，以及数据表的名称，若不存在指定数据表则返回false
		bool TableDelete(DataBase& target, std::string& tableName);
		//展示全部数据表，传入参数为当前活动DataBase的引用
		void TableShow(DataBase& target, std::string& name, std::ostream& o = std::cout);
		//展示指定数据表的基本属性，传入参数为该数据表的引用
		void TableShowInfo(Table& target, std::ostream& o = std::cout);

		//-----数据表中数据的操作，名称以Data开头-----//
		//加入一行数据，传入参数为进行操作的数据表、一行中各列的值（按照固定的顺序排列）
		bool DataInsert(Table& target, std::vector<DataUpdateAttr>& attr);
		//删除符合要求的行，传入参数为进行操作的数据表、WHERE子句
		bool DataDelete(Table& target, WhereAttr& where);
		//修改符合要求的行，传入参数为进行操作的数据表、WHERE子句、需要修改的列、修改后的值
		bool DataUpdate(Table& target, WhereAttr& where, std::vector<DataUpdateAttr>& attr);
		//查询（打印出）符合要求的行，传入参数为进行操作的数据表、WHERE子句
		void DataShow(Table& target, std::vector<std::string>& colName, WhereAttr& where, bool withTitle = true, std::ostream& o = std::cout);//如果是select *就直接将Table的colName传进来
		TemporaryTable select(Table& target, WhereAttr& where);

	private:
		//-----次级操作，作为通用的代码被上述方法调用，名称以Sub开头-----//
		//SubWhere中使用的子模块，用于进行比较
		template<typename T>
		bool SubCompare(const T& left, const T& right, WhereAttrSub2& ope);
		//whereClauses子句，传入数据表以及条件，返回符合要求的行构成的数组//如果没有Where条件，就构造一个空的clauses数组传进来，函数内部会进行判断
		std::vector<Data*> SubWhere(Table& target, WhereAttr& attr);
		//用于将以string形式传入的参数还原为原始的类型
		template<typename T>
		T SubStringToNum(const std::string& valS);
		//取交集
		template<typename T>
		void SubIntersection(std::vector<T>& first, std::vector<T>& second, std::vector<T>& result);//应当确保result是一个空数组
		//取并集
		template<typename T>
		void SubUnion(std::vector<T>& first, std::vector<T>& second, std::vector<T>& result);//应当确保result是一个空数组
		//返回给定Table的全部Data
		std::vector<Data*> SubGetAllData(Table& target);
	};

	//在DataShow中调用，用于对得到的Data*数组排序，依据是key值大小
	class SubDataShowCom
	{
	private:
		int mode;//1-int, 2-double, 3-char
		int pos;//此Table中Data的key在何位置
	public:
		SubDataShowCom(int mode_in, int pos_in) : mode(mode_in), pos(pos_in) {}
		bool operator()(const Data* left, const Data* right);
	};
}

//-----------------------------------------//
#endif // OPERATE_H

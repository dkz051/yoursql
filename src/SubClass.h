#ifndef SUBCLASS_H
#define SUBCLASS_H
//-----------------------------------------//
//本文件用于集中保存一些次级的类/结构
#include <string>
#include <vector>
#include <map>
#include <utility>
//-----------------------------------------//
namespace OOPD
{
	//保存存储数据的数据类型，在多个文件中使用
	enum DataType {typeInt, typeDouble, typeChar};

	//用于保存一行数据，在多个文件中使用
	struct Data
	{
		std::vector<int> valInt;
		std::vector<double> valDouble;
		std::vector<std::string> valString;
	};

	//用于保存一个数据表中各列的含义，在Table类(Table.h)中使用
	struct DataAddressType
	{
		DataType type;//此列的数据类型
		int pos;//此列位于对应类型数组的第几位
		bool notNull;//true表示设定为notNull
	};

	//下面是在Operate类中集中使用的次级类/结构--------------------------------------------
	//在TableCreate方法中传参使用的结构
	struct TableCreateAttr
	{
		std::string colName;
		DataType type;
		bool NotNull = false;//true表示设置为NOT NULL
		bool Primary = false;//true表示设置为主键
		TableCreateAttr(const std::string name_in, DataType type_in, bool notNull_in = false, bool primary_in = false) : colName(name_in), type(type_in), NotNull(notNull_in), Primary(primary_in) {}
		TableCreateAttr(){}
	};

	//在DataUpdate方法中传参使用的结构
	struct DataUpdateAttr
	{
		std::string colName;
		std::string val;
		DataUpdateAttr(const std::string& colName_in, const std::string& val_in) : colName(colName_in), val(val_in) {}
		DataUpdateAttr() {}
	};

	//在SubWhere方法中传参使用的结构
	enum WhereEnumNodeType {logic, compare, valConst, valCol, clause, used};//表达式中结点的类型，logic, compare, valConst, valCol适用于遍历检索方式
	//clause, logic, used适用于BPTree检索方式
	enum WhereEnumCompareType {greater, equal, less, greaterEqual, lessEqual, notEqual};//比较运算符的类型
	enum WhereEnumLogicType {oprOr, oprAnd, oprNot};//逻辑运算符的类型
	struct WhereAttrSub1//第一类检索方式使用的子结构
	{
		std::string colName;
		WhereEnumCompareType compareType;
		DataType type;
		std::string val;//使用字符串的形式存储操作数
		WhereAttrSub1(const std::string& colName_in, WhereEnumCompareType compareType_in, DataType type_in, const std::string& val_in) : colName(colName_in), compareType(compareType_in), type(type_in), val(val_in) {}
	};
	struct WhereAttrSub2//第二类传参方式使用的子结构
	{
		WhereEnumNodeType type = logic;//logic表示逻辑运算符，compare表示比较运算符，valConst表示常量，valCol表示列名
		WhereEnumLogicType logicType = oprOr;//越靠右的运算符优先级越高
		WhereEnumCompareType compareType = greater;
		DataType valType = typeInt;
		std::string valOrName;//存储常量的数值(以字符串形式)或列名，通过type变量的取值来解读
		WhereAttrSub2(WhereEnumNodeType type_in, WhereEnumLogicType logicType_in, WhereEnumCompareType compareType_in, DataType valType_in, const std::string& valOrName_in) : type(type_in), logicType(logicType_in), compareType(compareType_in), valType(valType_in), valOrName(valOrName_in) {}
	};
	enum WhereEnumSearchMode {all, search, traversal};//记录检索方式
	struct WhereAttr//用于传递Where子句参数的结构
	{
		WhereEnumSearchMode mode = all;//返回全部(没有输入Where条件)，利用BPTree特性进行检索(只有主键相关条件)，进行遍历筛选(对非主键列有要求)
		//若mode == search，需要关注两个数组
		std::vector<WhereAttrSub1> searchCompare;
		struct node
		{
			WhereEnumNodeType type;//clause表示此结点为比较运算，logic表示此结点为and or not运算，used仅在Operate方法中使用，用于将结点置空
			WhereEnumLogicType logicType = oprAnd;
			int pos = 0;//仅当类型为clause时需要考虑，指示其在clauses数组中的下标
			node(WhereEnumNodeType type_in, WhereEnumLogicType logicType_in, int pos_in) : type(type_in), logicType(logicType_in), pos(pos_in) {}
		};
		std::vector<node> searchAttr;
		//若mode == traversal，需要关注一个数组
		std::vector<WhereAttrSub2> traversalAttr;
		WhereAttr(WhereEnumSearchMode mode_in, const std::vector<WhereAttrSub1>& searchCompare_in, const std::vector<node>& searchAttr_in, const std::vector<WhereAttrSub2>& traversalAttr_in) : mode(mode_in), searchCompare(searchCompare_in), searchAttr(searchAttr_in), traversalAttr(traversalAttr_in) {}
	};
}

//-----------------------------------------//
#endif // SUBCLASS_H

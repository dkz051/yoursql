#ifndef CONTROLLER_H
#define CONTROLLER_H
//-----------------------------------------//
#include <string>
#include "Operate.h"
#include "tools.h"
//-----------------------------------------//
//Controller类是数据库工作过程的总控，负责与用户进行交互
//Controller类通过指针/引用访问OOPDB类的底层数据和Operate对象
//Contrller类解析用户的输入，从中得到应该调用Operate中的哪些方法来对数据进行操作，进行数据操作的目标是什么，以及一些相关的参数
//从某种意义上讲，Controller类沟通了底层数据和对数据的上层操作(Operate类)
namespace OOPD
{
	class Controller
	{
		DataBase * activeDB;//指向当前活动数据库，通过USE方法修改
		DataBase * defaultDB;
		std::map<std::string, DataBase*> & target;//指向OOPDB类中的DataBase容器
		Operate & opt;//指向OOPDB类中的Operate对象
	public:
		Controller(std::map<std::string, DataBase*> & DBList, Operate & OPT): target(DBList), opt(OPT) {}
		~Controller() {}
		//调用此方法开始循环读取用户输入，直到遇到EOF
		void start();
		bool Readin(std::ostream& o = std::cout);
		bool execute(std::string str1, std::ostream& o = std::cout); //执行单条命令
		int CutString(std::string& str);
		std::string& AdjustString(std::string& str);
		std::string& TrimString(std::string& str);
		std::string& TrimChar(std::string& str);
		//当用户输入的第一段字符为CREATE时调用此方法，对应DataBase和Table两种创建
		bool CREATE(std::string& str3);
		//当用户输入的第一段字符为DROP时调用此方法，对应DataBase和Table两种删除
		bool DROP(std::string& str3);
		//当用户输入的第一段字符为USE时调用此方法，修改activeDB指针
		bool USE(std::string& str3);
		bool SHOW(std::string& str3, std::ostream& o = std::cout);
		bool INSERT(std::string& str3);
		bool DELETE(std::string& str3);
		bool UPDATE(std::string& str3);
		bool SELECT(std::string& str3, std::ostream& o = std::cout);
		//子方法，用于构造Operate::TableCreate方法的参数结构
		TableCreateAttr TableOp(std::string& temp);
		//子方法，用于构造Where子句的参数结构
		WhereAttr SubWhere(Table& target, std::string& wholeStr);//Data层面的Show, Delete, Updata调用此方法以获得Where参数
		//在SubWhere中调用的次级子方法，用于在一定规则下将endOfNode保持为最小值
		int& SubMin(int& endOfNode, const int& num);
	};

}
//-----------------------------------------//

#endif // CONTROLLER_H

#ifndef OOPDB_H
#define OOPDB_H
//-----------------------------------------//
#include "Controller.h"
#include <map>
#include <string>
//-----------------------------------------//
//OOPDB类是对数据库的数据/底层实现/底层接口、对数据的上层操作、总控的整体封装，上述三个模块被划分为四个类：
//Controller类负责总控，以及与用户交互（读取指令）
//Operate类负责对数据进行一些上层操作
//BPTree类是底层实现，同时有一些底层接口，将被Operate类的方法调用；DataBase类包含一个BPTree对象的数组，同时OOPDB类中有一个DataBase对象的数组
namespace OOPD
{
	class OOPDB
	{
	private:
		Operate operate;
		std::map<std::string, DataBase*> DBList;
		Controller controller;
	public:
		OOPDB() : controller(DBList, operate) {}
		~OOPDB()
		{
			for (auto it = DBList.begin(); it != DBList.end(); ++it)
				delete it->second;
		}
		//Controller类中应该用引用接收这两个参数
		void Start() {controller.start();}
		void execute(std::string sql, std::ostream& o = std::cout) { controller.execute(sql, o); }
	};
}

//-----------------------------------------//
#endif // OOPDB_H

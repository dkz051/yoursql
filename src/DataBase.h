#ifndef DATABASE_H
#define DATABASE_H
//-----------------------------------------//
#include <map>
#include <string>

#include "Table.h"
//-----------------------------------------//
//DataBase类中含有一个Table对象的容器
namespace OOPD
{
	class DataBase
	{
		friend class Controller;
		friend class Session;
		friend class Operate;
	private:
		std::map<std::string, Table*> TableList;
	public:
		~DataBase();
	};
}

//-----------------------------------------//
#endif // DATABASE_H

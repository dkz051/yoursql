#ifndef DATABASE_H
#define DATABASE_H
//-----------------------------------------//
#include "Table.h"
#include <map>
#include <string>
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
		~DataBase()
		{
			for (auto it = TableList.begin(); it != TableList.end(); ++it)
				delete it->second;
		}
	};
}

//-----------------------------------------//
#endif // DATABASE_H

#include "DataBase.h"

namespace OOPD
{
	DataBase::~DataBase()
	{
		for (auto it = TableList.begin(); it != TableList.end(); ++it)
			delete it->second;
	}
}

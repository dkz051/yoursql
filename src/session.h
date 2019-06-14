#ifndef SESSION_H
#define SESSION_H

#include <string>

#include "Operate.h"
#include "tools.h"

/**
 * 基于 Controller.h/cpp，但做了以下修改。
 *
 * (1) 直接包含 Operate 对象；（没有必要让上层创建这种辅助类的对象）
 * (2) 替换命令解释器；（D 组第一阶段的解释器太烂没法用）
 * (3) 对大量标识符重新命名。（实际上没有必要，这样做主要是容易区分第一阶段和第二阶段的接口）
 */

namespace OOPD
{
	class Session
	{
		std::string selected; // 保存当前活动数据库的名称。注意不能使用指针（可能会被其他 Session 删除，变成野指针；也不能用静态变量，因为要保证各客户端的会话独立）
		dbSet& db;
		Operate opt;

		DataBase* getDatabase(const std::string& dbName = "");
		Table* getTable(const std::string& tableName, const std::string& dbName = "");

		bool createDatabase(std::string dbName);
		bool createTable(std::string tableName, const tokens& traits);
		bool dropDatabase(std::string dbName);
		bool dropTable(std::string tableName);
		bool use(std::string dbName);
		int insert(std::string tableName, const attrs& fields, const tokens& values);
		int insertRaw(std::string tableName, const attrs& fields, tokens values);
		int remove(const std::string& tableName, const tokens& whereClause = tokens()); // delete
		int update(const std::string& tableName, const tokens& setClause, const tokens& whereClause = tokens());
		//子方法，用于构造Where子句的参数结构
		WhereAttr SubWhere(Table& target, std::string& wholeStr);//Data层面的Show, Delete, Updata调用此方法以获得Where参数
		//在SubWhere中调用的次级子方法，用于在一定规则下将endOfNode保持为最小值
		int& SubMin(int& endOfNode, const int& num);
	public:
		Session(dbSet& dbList): selected(""), db(dbList) {}
		~Session() {}
		void start();
		std::string execute(std::string sql, std::ostream& o = std::cout);
	};
}
//-----------------------------------------//

#endif // SESSION_H

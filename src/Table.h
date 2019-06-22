#ifndef TABLE_H
#define TABLE_H
//-----------------------------------------//
#include <string>
#include <map>
#include <vector>
#include <utility>

#include "tools.h"
#include "BPTree.h"
//Table类是底层数据结构中较上层的部分，一个Table类维护多个索引树，以提高检索效率
//因此Table类重新给出一些进行检索、修改、删除、新建的接口
//并且Table类额外存储一些辅助数据维护的信息

namespace OOPD
{
	class Table
	{
		friend class Operate;
		friend class Controller;
		friend class Session;
		friend class TemporaryTable;
	private:
		int rowID; //行ID，每插入一行+1
		int TreeRank; //索引树的阶数
		int TypeNum; //数据类型数量
		std::string PrimaryCol; //主索引的列名
		std::map<std::string, DataAddressType> DataAddress;//通过列名查找此列数据的数据类型以及在Data对象中的存储位置，pair中的DataType告知应去哪个数组中查找，int则是其在该数组中的下标

		std::vector<std::string> columnNames; // 存储列名称的 vector

		std::map<std::string, BPTree<int>*> IntTreeList; //通过列名管理多个索引树
		std::map<std::string, BPTree<double>*> DoubleTreeList;
		std::map<std::string, BPTree<std::string>*> CharTreeList;
	public:
		Table(std::vector<TableCreateAttr> & KeyInfo, std::vector<TableCreateAttr> & ColInfo, const int tree_rank, const int type_num);
		~Table();//需要把所有树释放
		BPTree<int>* GetTreeInt(const TableCreateAttr & KeyInfo);
		BPTree<double>* GetTreeDouble(const TableCreateAttr & KeyInfo);
		BPTree<std::string>* GetTreeChar(const TableCreateAttr & KeyInfo);
		//一些底层控制接口
	};

	// 临时表对象，用于 SELECT 语句返回中间结果后、输出前；不用于存储数据
	// 如此操作的原因是 D 组的结构设计的过于复杂
	class TemporaryTable final
	{
	private:
		std::vector<Data*> rows;
		std::string primary;
		std::vector<std::string> columnNames;
		std::map<std::string, DataAddressType> DataAddress; // 同 Table 中的含义
	public:
		size_t size();
		void print(const attrs& fields, bool withTitle = true, std::ostream& o = std::cout);
		void orderBy(const orders& order);
		TemporaryTable(const Table& table);
		TemporaryTable(const Table& table, std::vector<Data*> data);
	};
}

//-----------------------------------------//
#endif // TABLE_H

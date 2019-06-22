#include "Table.h"

#include "Operate.h"

namespace OOPD
{
	Table::Table(std::vector<TableCreateAttr> & KeyInfo, std::vector<TableCreateAttr> & ColInfo, const int tree_rank, const int type_num) : TreeRank(tree_rank), TypeNum(type_num)
	{
		int *pos = new int[TypeNum];
		for (int i = 0; i < TypeNum; ++i)
			pos[i] = 0;
		for (auto it = ColInfo.begin(); it != ColInfo.end(); ++it)
		{
			DataAddress.insert(std::pair<std::string, DataAddressType>(it->colName, {it->type, pos[it->type]++, it->NotNull}));
			if (it->Primary) PrimaryCol = it->colName;
			columnNames.push_back(it->colName);
		}
		for (auto it = KeyInfo.begin(); it != KeyInfo.end(); ++it)
		{
			switch (it->type)
			{
				case typeInt:
					IntTreeList.insert(std::pair<std::string, BPTree<int>*>(it->colName, new BPTree<int>(TreeRank)));
					break;
				case typeDouble:
					DoubleTreeList.insert(std::pair<std::string, BPTree<double>*>(it->colName, new BPTree<double>(TreeRank)));
					break;
				case typeChar:
					CharTreeList.insert(std::pair<std::string, BPTree<std::string>*>(it->colName, new BPTree<std::string>(TreeRank)));
					break;
			}
		}
		delete [] pos;
	}
	//初始化表以及表中的索引树

	Table::~Table()
	{
		std::vector<Data*> data;
		if (!IntTreeList.empty()) data = IntTreeList.begin()->second->GetData();
		else if (!DoubleTreeList.empty()) data = DoubleTreeList.begin()->second->GetData();
		else if (!CharTreeList.empty()) data = CharTreeList.begin()->second->GetData();
		else return;
		for (auto it = data.begin(); it != data.end(); ++it)
			delete *it;
		for (auto it = IntTreeList.begin(); it != IntTreeList.end(); ++it)
			delete it->second;
		for (auto it = DoubleTreeList.begin(); it != DoubleTreeList.end(); ++it)
			delete it->second;
		for (auto it = CharTreeList.begin(); it != CharTreeList.end(); ++it)
			delete it->second;
		IntTreeList.clear();
		DoubleTreeList.clear();
		CharTreeList.clear();
		DataAddress.clear();
	}
	//析构表

	// 创建空的临时表
	TemporaryTable::TemporaryTable(const Table& table): primary(table.PrimaryCol), columnNames(table.columnNames), DataAddress(table.DataAddress) {}

	// 创建有数据的临时表
	TemporaryTable::TemporaryTable(const Table& table, std::vector<Data*> data): primary(table.PrimaryCol), columnNames(table.columnNames), DataAddress(table.DataAddress), rows(data) {}

	// 返回表中记录条数
	size_t TemporaryTable::size()
	{
		return rows.size();
	}

	// 输出表的内容，不包含标题
	void TemporaryTable::print(const attrs& fields, bool withTitle, std::ostream& o)
	{
		if (rows.size() == 0)
			return;//则不输出任何信息
		if (withTitle)
		{
			for (auto it = fields.begin(); it != fields.end(); ++it) //输出列名
				o << *it << "\t";
			o << std::endl;
		}
		auto end = rows.end();
		// const DataAddressType& info = DataAddress[primary];//对此数组进行排序，依据主键的大小
		// switch (info.type)
		// {
		// 	case typeInt: std::sort(rows.begin(), end, SubDataShowCom(1, info.pos)); break;
		// 	case typeDouble: std::sort(rows.begin(), end, SubDataShowCom(2, info.pos)); break;
		// 	case typeChar: std::sort(rows.begin(), end, SubDataShowCom(3, info.pos)); break;
		// }
		for (auto it = rows.begin(); it != end; ++it)//对于每一行
		{
			auto colEnd = fields.end();
			for (auto colIt = fields.begin(); colIt != colEnd; ++colIt)//对于该行的每一列
			{
				const DataAddressType& info = DataAddress[*colIt];

				if (colIt != fields.begin()) o << '\t';

				switch (info.type)
				{
					case typeInt:
						if ((*it)->valInt[info.pos] == 0x3f3f3f) {o << "NULL"; break;}
						else {o << (*it)->valInt[info.pos]; break;}
					case typeDouble:
						if (int((*it)->valDouble[info.pos]) == 0x3f3f3f) {o << "NULL"; break;}
						else {o << std::fixed << std::setprecision(4) << (*it)->valDouble[info.pos]; break;}
					case typeChar: o << (*it)->valString[info.pos]; break;
				}
			}
			o << std::endl;
		}
	}

	// 对表中全部数据排序
	void TemporaryTable::orderBy(const orders& order)
	{
		auto sortOperator = [&](Data* a, Data* b) -> bool
		{
			for (auto iter = order.begin(); iter != order.end(); ++iter)
			{
				int id = DataAddress[iter->field].pos;
				bool val = iter->sort == OOPD::sort_t::ascending ? false : true;
				switch (DataAddress[iter->field].type)
				{
					case typeInt:
					{
						if (a->valInt[id] != b->valInt[id])
							return val ^ (a->valInt[id] < b->valInt[id]);
						break;
					}
					case typeDouble:
					{
						if (a->valDouble[id] != b->valDouble[id])
							return val ^ (a->valDouble[id] < b->valDouble[id]);
						break;
					}
					case typeChar:
					{
						if (a->valString[id] != b->valString[id])
							return val ^ (a->valString[id] < b->valString[id]);
						break;
					}
				}
			}
			return false;
		};
		std::sort(rows.begin(), rows.end(), sortOperator);
	}

	BPTree<int>* Table::GetTreeInt(const TableCreateAttr & KeyInfo)
	{
		auto it = IntTreeList.find(KeyInfo.colName);
		if (it == IntTreeList.end()) return nullptr;
		else return it->second;
	}

	BPTree<double>* Table::GetTreeDouble(const TableCreateAttr & KeyInfo)
	{
		auto it = DoubleTreeList.find(KeyInfo.colName);
		if (it == DoubleTreeList.end()) return nullptr;
		else return it->second;
	}

	BPTree<std::string>* Table::GetTreeChar(const TableCreateAttr & KeyInfo)
	{
		auto it = CharTreeList.find(KeyInfo.colName);
		if (it == CharTreeList.end()) return nullptr;
		else return it->second;
	}
	//获取表中某一棵索引树
}

#include "Table.h"

#include <cmath>
#include <algorithm>

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

	// 创建空的临时表（没有表结构）
	TemporaryTable::TemporaryTable() {}

	// 创建空的临时表（与参数提供的表的结构相同，但没有数据）
	TemporaryTable::TemporaryTable(const Table& table): primary(table.PrimaryCol), columnNames(table.columnNames), DataAddress(table.DataAddress) {}

	// 创建有数据的临时表
	TemporaryTable::TemporaryTable(const Table& table, std::vector<Data*> data): primary(table.PrimaryCol), columnNames(table.columnNames), DataAddress(table.DataAddress)
	{
		for (auto iter = data.begin(); iter != data.end(); ++iter)
			rows.push_back(**iter);
	}

	// 返回表中记录条数
	size_t TemporaryTable::size()
	{
		return rows.size();
	}

	// 输出表的内容
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
						if (it->valInt[info.pos] == 0x3f3f3f) {o << "NULL"; break;}
						else {o << it->valInt[info.pos]; break;}
					case typeDouble:
						if (int(it->valDouble[info.pos]) == 0x3f3f3f) {o << "NULL"; break;}
						else {o << std::fixed << std::setprecision(4) << it->valDouble[info.pos]; break;}
					case typeChar: o << it->valString[info.pos]; break;
				}
			}
			o << std::endl;
		}
	}

	// 对表中全部数据排序
	void TemporaryTable::orderBy(const orders& order)
	{
		std::sort(rows.begin(), rows.end(),
			[&](Data a, Data b) -> bool
			{
				for (auto iter = order.begin(); iter != order.end(); ++iter)
				{
					int id = DataAddress[iter->field].pos;
					bool val = iter->sort == OOPD::sort_t::ascending ? false : true;
					switch (DataAddress[iter->field].type)
					{
						case typeInt:
						{
							if (a.valInt[id] != b.valInt[id])
								return val ^ (a.valInt[id] < b.valInt[id]);
							break;
						}
						case typeDouble:
						{
							if (a.valDouble[id] != b.valDouble[id])
								return val ^ (a.valDouble[id] < b.valDouble[id]);
							break;
						}
						case typeChar:
						{
							if (a.valString[id] != b.valString[id])
								return val ^ (a.valString[id] < b.valString[id]);
							break;
						}
					}
				}
				return false;
			}
		);
	}

	// 对记录集分组并执行相应函数
	TemporaryTable TemporaryTable::aggregate(const attrs& fields, const groups& group)
	{
		// 先按分组所用的列排序
		orders order;
		for (auto iter = fields.begin(); iter != fields.end(); ++iter)
			order.push_back((order_t){*iter, OOPD::sort_t::ascending});
		orderBy(order);

		// 判断两条记录是否属于同一组（指定的列的值全相等）
		auto equals = [&](Data a, Data b) -> bool
		{
			for (auto iter = fields.begin(); iter != fields.end(); ++iter)
			{
				int id = DataAddress[*iter].pos;
				switch (DataAddress[*iter].type)
				{
					case typeInt:
					{
						if (a.valInt[id] != b.valInt[id])
							return false;
						break;
					}
					case typeDouble:
					{
						if (a.valDouble[id] != b.valDouble[id])
							return false;
						break;
					}
					case typeChar:
					{
						if (a.valString[id] != b.valString[id])
							return false;
						break;
					}
				}
			}
			return true;
		};

		const int TypeNum = 3;

		int *pos = new int[TypeNum];
		for (int i = 0; i < TypeNum; ++i)
			pos[i] = 0;

		// 创建临时表
		TemporaryTable result;
		result.primary = primary; // 临时表的主键与原表相同
		for (auto iter = fields.begin(); iter != fields.end(); ++iter)
		{
			result.columnNames.push_back(*iter);
			result.DataAddress.insert(std::pair<std::string, DataAddressType>(*iter, {DataAddress[*iter].type, pos[DataAddress[*iter].type]++, DataAddress[*iter].notNull}));
		}

		// 在后面附加上数字函数字段
		for (auto iter = group.begin(); iter != group.end(); ++iter)
		{
			std::string function = stringToUpper(iter->function);
			std::string colName = function + '(' + iter->field + ')';
			OOPD::DataType colType = DataAddress[iter->field].type;
			if (function == "COUNT") colType = typeInt;
			else if (function == "AVG" || function == "STDDEV") colType = typeDouble;
			result.columnNames.push_back(colName);
			result.DataAddress.insert(std::pair<std::string, DataAddressType>(colName, {colType, pos[colType]++, false}));
		}

		// 最后是特殊字段 COUNT(*)
		result.columnNames.push_back("COUNT(*)");
		result.DataAddress.insert(std::pair<std::string, DataAddressType>("COUNT(*)", {OOPD::typeInt, pos[typeInt]++, false}));

		// 对数据分组并计算数字函数
		auto aggregate = [&](std::vector<Data>::iterator begin, std::vector<Data>::iterator end, const attrs& fields, const groups& group, std::map<std::string, DataAddressType>& DA) -> Data
		{
			Data ans;
			ans.valInt.resize(pos[typeInt]);
			ans.valDouble.resize(pos[typeDouble]);
			ans.valString.resize(pos[typeChar]);

			for (auto iter = fields.begin(); iter != fields.end(); ++iter)
			{
				OOPD::DataType type = DA[*iter].type;
				int id1 = DA[*iter].pos;
				int id2 = DataAddress[*iter].pos;
				if (type == typeInt)
					ans.valInt[id1] = begin->valInt[id2];
				else if (type == typeDouble)
					ans.valDouble[id1] = begin->valDouble[id2];
				else if (type == typeChar)
					ans.valString[id1] = begin->valString[id2];
			}

			for (auto iter = group.begin(); iter != group.end(); ++iter)
			{
				std::string function = stringToUpper(iter->function);
				std::string name = function + '(' + iter->field + ')';
				OOPD::DataType type = DA[name].type;
				int id = DA[name].pos;

				double sum = 0.0, sum2 = 0.0, dblE;
				int count = 0, intE;
				std::string strE;

				int sId = DA[iter->field].pos;
				OOPD::DataType sType = DA[iter->field].type;

				bool flag = false;
				for (auto jter = begin; jter != end; ++jter)
				{
					if (sType == typeInt)
					{
						int vInt = jter->valInt[sId];
						if (vInt == 0x3f3f3f) // is null
							continue;
						sum += vInt;
						sum2 += vInt * vInt;
						if (!flag)
							intE = vInt;
						else
						{
							if (function == "MAX") intE = std::max(intE, vInt);
							else intE = std::min(intE, vInt);
						}
					}
					else if (sType == typeDouble)
					{
						double vDbl = jter->valDouble[sId];
						if ((int)vDbl == 0x3f3f3f) // is null
							continue;
						sum += vDbl;
						sum2 += vDbl * vDbl;
						if (!flag)
							dblE = vDbl;
						else
						{
							if (function == "MAX") dblE = std::max(dblE, vDbl);
							else dblE = std::min(dblE, vDbl);
						}
					}
					else if (sType == typeChar)
					{
						std::string vChar = jter->valString[sId];
						if (vChar == "NULL") // is null
							continue;
						if (!flag)
							strE = vChar;
						else
						{
							if (function == "MAX") strE = std::max(strE, vChar);
							else strE = std::min(strE, vChar);
						}
					}
					++count;
				}

				if (function == "MAX" || function == "MIN")
				{
					if (type == typeInt) ans.valInt[id] = count ? intE : 0x3f3f3f;
					else if (type == typeDouble) ans.valDouble[id] = count ? dblE : 0x3f3f3f;
					else if (type == typeChar) ans.valString[id] = count ? strE : "NULL";
				}
				else if (function == "AVG" || function == "SUM")
				{
					if (type != typeDouble) throw "Calculate: Type Error";
					ans.valDouble[id] = (function == "SUM" ? sum : (count ? sum / count : 0x3f3f3f));
				}
				else if (function == "COUNT")
				{
					if (type != typeInt) throw "Calculate: Type Error";
					ans.valInt[id] = count;
				}
				else if (function == "STDDEV")
				{
					if (type != typeDouble) throw "Calculate: Type Error";
					ans.valDouble[id] = count ? sqrt(sum2 / count - pow(sum / count, 2.0)) : 0x3f3f3f;
				}
			}
			ans.valInt[DA["COUNT(*)"].pos] = end - begin;
			return ans;
		};

		for (unsigned i = 0, j = 0; i <= rows.size(); ++i)
		{
			if (i == rows.size() || (i > 0 && !equals(rows[i - 1], rows[i])))
			{
				result.rows.push_back(aggregate(rows.begin() + j, rows.begin() + i, fields, group, result.DataAddress));
				j = i;
			}
		}

		delete[] pos;
		return result;
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

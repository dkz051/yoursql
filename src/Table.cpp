#include "Table.h"

#include <cmath>
#include <algorithm>

#include "Operate.h"

namespace OOPD
{
	Table::Table(std::vector<TableCreateAttr> & KeyInfo, std::vector<TableCreateAttr> & ColInfo, const int tree_rank, const int type_num) : TreeRank(tree_rank), TypeNum(type_num)
	{
	//	int *pos = new int[TypeNum];
	//	for (int i = 0; i < TypeNum; ++i)
	//		pos[i] = 0;
		int columns = 0;
		for (auto it = ColInfo.begin(); it != ColInfo.end(); ++it)
		{
	//		DataAddress.insert(std::pair<std::string, DataAddressType>(it->colName, {it->type, pos[it->type]++, it->NotNull}));
			DataAddress[it->colName] = (DataAddressType){it->type, columns++, it->NotNull};
			if (it->Primary) PrimaryCol = it->colName;
			columnNames.push_back(it->colName);
		}
		for (auto it = KeyInfo.begin(); it != KeyInfo.end(); ++it)
		{
		/*	switch (it->type)
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
			}*/
			TreeList[it->colName] = new BPTree<YourSqlData>(TreeRank);
		}
		//delete [] pos;
	}
	//初始化表以及表中的索引树

	Table::~Table()
	{
		std::vector<Data*> data;
	//	if (!IntTreeList.empty()) data = IntTreeList.begin()->second->GetData();
	//	else if (!DoubleTreeList.empty()) data = DoubleTreeList.begin()->second->GetData();
	//	else if (!CharTreeList.empty()) data = CharTreeList.begin()->second->GetData();
		if (!TreeList.empty()) data = TreeList.begin()->second->GetData();
		else return;
		for (auto it = data.begin(); it != data.end(); ++it)
			delete *it;
	//	for (auto it = IntTreeList.begin(); it != IntTreeList.end(); ++it)
	//		delete it->second;
	//	for (auto it = DoubleTreeList.begin(); it != DoubleTreeList.end(); ++it)
	//		delete it->second;
	//	for (auto it = CharTreeList.begin(); it != CharTreeList.end(); ++it)
	//		delete it->second;
		for (auto it = TreeList.begin(); it != TreeList.end(); ++it)
			delete it->second;
	//	IntTreeList.clear();
	//	DoubleTreeList.clear();
	//	CharTreeList.clear();
		TreeList.clear();
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
			return; // 没有数据则不输出任何信息
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

			/*	switch (info.type)
				{
					case typeInt:
						if (it->valInt[info.pos] == 0x3f3f3f) {o << "NULL"; break;}
						else {o << it->valInt[info.pos]; break;}
					case typeDouble:
						if (int(it->valDouble[info.pos]) == 0x3f3f3f) {o << "NULL"; break;}
						else {o << std::fixed << std::setprecision(4) << it->valDouble[info.pos]; break;}
					case typeChar: o << it->valString[info.pos]; break;
				}*/
				o << it->values[info.pos];
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
				/*	switch (DataAddress[iter->field].type)
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
					}*/

					if (iter->sort == sort_t::ascending)
					{
						if (a.values[id].isNull() && b.values[id].isNull()) continue;
						if (a.values[id].isNull()) return true;
						if (b.values[id].isNull()) return false;
						if (a.values[id] != b.values[id]) return a.values[id] < b.values[id];
					}
					else
					{
						if (a.values[id].isNull() && b.values[id].isNull()) continue;
						if (b.values[id].isNull()) return true;
						if (a.values[id].isNull()) return false;
						if (a.values[id] != b.values[id]) return a.values[id] > b.values[id];
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
			/*	switch (DataAddress[*iter].type)
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
				}*/
				if (a.values[id] != b.values[id])
					return false;
			}
			return true;
		};

	//	const int TypeNum = 3;

	//	int *pos = new int[TypeNum];
	//	for (int i = 0; i < TypeNum; ++i)
	//		pos[i] = 0;

		int columns = 0;

		// 创建临时表
		TemporaryTable result;
		result.primary = primary; // 临时表的主键与原表相同
		for (auto iter = fields.begin(); iter != fields.end(); ++iter)
		{
			result.columnNames.push_back(*iter);
		//	result.DataAddress.insert(std::pair<std::string, DataAddressType>(*iter, {DataAddress[*iter].type, pos[DataAddress[*iter].type]++, DataAddress[*iter].notNull}));
			result.DataAddress.insert(std::pair<std::string, DataAddressType>(*iter, {DataAddress[*iter].type, columns++, DataAddress[*iter].notNull}));
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
			//result.DataAddress.insert(std::pair<std::string, DataAddressType>(colName, {colType, pos[colType]++, false}));
			result.DataAddress.insert(std::pair<std::string, DataAddressType>(colName, {colType, columns++, false}));
		}

		// 最后是特殊字段 COUNT(*)
		result.columnNames.push_back("COUNT(*)");
		//result.DataAddress.insert(std::pair<std::string, DataAddressType>("COUNT(*)", {OOPD::typeInt, pos[typeInt]++, false}));
		result.DataAddress.insert(std::pair<std::string, DataAddressType>("COUNT(*)", {OOPD::typeInt, columns++, false}));

		if (!result.DataAddress.count(result.primary))
			result.primary = "";

		// 对数据分组并计算数字函数
		auto aggregate = [&](std::vector<Data>::iterator begin, std::vector<Data>::iterator end, const attrs& fields, const groups& group, std::map<std::string, DataAddressType>& DA) -> Data
		{
			Data ans;
			//ans.valInt.resize(pos[typeInt]);
			//ans.valDouble.resize(pos[typeDouble]);
			//ans.valString.resize(pos[typeChar]);
			ans.values.resize(columns);

			// 用来分组的列，直接从表中拿数据，内容一定一样
			for (auto iter = fields.begin(); iter != fields.end(); ++iter)
			{
				OOPD::DataType type = DA[*iter].type;
				int id1 = DA[*iter].pos; // 获取在最终返回的临时表中列的序号
				int id2 = DataAddress[*iter].pos; // 获取在原始数据表中列的序号
			/*	if (type == typeInt)
					ans.valInt[id1] = begin->valInt[id2];
				else if (type == typeDouble)
					ans.valDouble[id1] = begin->valDouble[id2];
				else if (type == typeChar)
					ans.valString[id1] = begin->valString[id2];
					*/
				ans.values[id1] = begin->values[id2];
			}

			// 对于每个数字函数调用，计算其值
			for (auto iter = group.begin(); iter != group.end(); ++iter)
			{
				std::string function = stringToUpper(iter->function); // 函数名称
				std::string name = function + '(' + iter->field + ')'; // 列名
				OOPD::DataType type = DA[name].type; // 要返回的类型
				int id = DA[name].pos; // 列序号

				double sum = 0.0, sum2 = 0.0;//, dblE; // sum - 求和，sum2 - 平方和，dblE - double 型极值
				int count = 0;//, intE; // count - 计数，intE - int 型极值
				//std::string strE; // strE - string 型极值
				const YourSqlData* extremum = nullptr;

				int sId = DataAddress[iter->field].pos; // 被计算的列的编号
				OOPD::DataType sType = DA[iter->field].type; // 及类型

				bool flag = false;
				for (auto jter = begin; jter != end; ++jter)
				{
					const YourSqlData& value = jter->values[sId];
					if (value.isNull()) continue;
					if (sType != typeChar)
					{
						double numeric = std::stod(value.ptr->get());
						sum += numeric;
						sum2 += pow(numeric, 2.0);
					}
					if (!flag)
						extremum = &value, flag = true;
					else
					{
						if (function == "MAX" && value > *extremum) extremum = &value;
						else if (function == "MIN" && value < *extremum) extremum = &value;
					}

				/*	if (sType == typeInt)
					{
						int vInt = jter->valInt[sId];
						if (vInt == 0x3f3f3f) // is null
							continue;
						sum += vInt;
						sum2 += vInt * vInt;
						if (!flag)
						{
							intE = vInt;
							flag = true;
						}
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
						{
							dblE = vDbl;
							flag = true;
						}
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
						{
							strE = vChar;
							flag = true;
						}
						else
						{
							if (function == "MAX") strE = std::max(strE, vChar);
							else strE = std::min(strE, vChar);
						}
					}*/
					++count;
				}
				if (function == "MAX" || function == "MIN")
					/*if (type == typeInt) ans.valInt[id] = count ? intE : 0x3f3f3f;
					else if (type == typeDouble) ans.valDouble[id] = count ? dblE : 0x3f3f3f;
					else if (type == typeChar) ans.valString[id] = count ? strE : "NULL";*/
					ans.values[id] = *extremum;
				else if (function == "AVG")
				{
					if (type != typeDouble) throw "Aggregate: Type Error";
					//ans.valDouble[id] = (count ? sum / count : 0x3f3f3f);
					ans.values[id] = (count ? YourSqlData(new DataDouble(sum / count)) : YourSqlData(new DataDouble()));
				}
				else if (function == "SUM")
				{
					//if (type == typeInt) ans.valInt[id] = count ? sum : 0x3f3f3f;
					//else if (type == typeDouble) ans.valDouble[id] = count ? sum : 0x3f3f3f;
					//else throw "Aggregate: Type Error";
					if (type == typeChar) throw "Aggregate: Type Error";
					ans.values[id] = (count ? YourSqlData(new DataDouble(sum)) : YourSqlData(new DataDouble()));
				}
				else if (function == "COUNT")
				{
					if (type != typeInt) throw "Aggregate: Type Error";
					//ans.valInt[id] = count;
					ans.values[id] = YourSqlData(new DataInt(count));
				}
				else if (function == "STDDEV")
				{
					if (type != typeDouble) throw "Aggregate: Type Error";
					//ans.valDouble[id] = count ? sqrt(sum2 / count - pow(sum / count, 2.0)) : 0x3f3f3f;
					ans.values[id] = count ? YourSqlData(new DataDouble(sqrt(sum2 / count - pow(sum / count, 2.0)))) : YourSqlData(new DataDouble());
				}
			}
			//ans.valInt[DA["COUNT(*)"].pos] = end - begin;
			ans.values[DA["COUNT(*)"].pos] = YourSqlData(new DataInt(end - begin));
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

//		delete[] pos;
		return result;
	}

/*	BPTree<int>* Table::GetTreeInt(const TableCreateAttr & KeyInfo)
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
*/
	BPTree<YourSqlData>* Table::GetTree(const TableCreateAttr & KeyInfo)
	{
		auto it = TreeList.find(KeyInfo.colName);
		if (it == TreeList.end()) return nullptr;
		else return it->second;
	} //获取表中某一棵索引树
}

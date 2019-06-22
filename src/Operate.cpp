#include "Operate.h"

#include "tools.h"

namespace OOPD
{
	//新建一个数据库，传入参数为DataBase容器的引用，以及数据库的名称//返回true表示成功新建，否则表示已有同A
	bool Operate::DBCreate(std::map<std::string, DataBase*>& target, std::string& DBname)
	{
		auto result = target.insert(std::make_pair(DBname, new DataBase()));
		return result.second;
	}

	//删除指定数据库，传入参数同上，若不存在指定数据库则返回false//返回true表示删除成功，否则表示没有此数据库
	bool Operate::DBDelete(std::map<std::string, DataBase*>& target, std::string& DBname)
	{
		auto it = target.find(DBname);
		if (it != target.end())
		{
			delete it->second;
			target.erase(it);
			return true;
		}
		return false;
	}

	//展示全部数据库，传入参数为DataBase容器的引用
	void Operate::DBShow(std::map<std::string, DataBase*>& target, std::ostream& o)
	{
		if (target.empty()) return;
		o << "Database" << std::endl;
		auto end = target.end();
		for (auto it = target.begin(); it != end; ++it)
		{
			o << it->first << std::endl;
		}
		return;
	}

	//---------------------------------------------------------------------------------------------------------------------------------------//

	//新建一个数据表，传入参数为当前活动DataBase的引用，数据表的名称、(各列的名称及数据类型、是否设置为NOT NULL、是否设置为主键)在一个结构体中
	//返回true表示成功新建，否则表示已有同名数据库
	bool Operate::TableCreate(DataBase& target, std::string& tableName, std::vector<TableCreateAttr>& attr)
	{
		//提取参数中的主键信息
		auto end = attr.end();
		std::vector<TableCreateAttr> keyInfo;//存储索引列信息（目前仅有主键列为索引列）
		std::vector<TableCreateAttr> colInfo;//保存所有列信息
		for (auto it = attr.begin(); it != end; ++it)
		{
			if (it->Primary)//如果是主键
				keyInfo.push_back(*it);
			colInfo.push_back(std::move(*it));
		}
		auto result = target.TableList.insert(std::make_pair(tableName, new Table(keyInfo, colInfo, 4, 3)));//这里的4表示树的阶数，3表示数据类型的数量，可以修改
		return result.second;
	}

	//删除一个数据表，传入参数为当前活动DataBase的引用，以及数据表的名称，若不存在指定数据表则返回false
	bool Operate::TableDelete(DataBase& target, std::string& tableName)
	{
		auto it = target.TableList.find(tableName);
		if (it != target.TableList.end())
		{
			delete  it->second;
			target.TableList.erase(it);
			return true;
		}
		return false;
	}

	//展示全部数据表，传入参数为当前活动DataBase的引用
	void Operate::TableShow(DataBase& target, std::string& name, std::ostream& o)
	{
		if (target.TableList.empty()) return;
		o << "Tables_in_" << name << std::endl;
		auto end = target.TableList.end();
		for (auto it = target.TableList.begin(); it != end; ++it)
			o << it->first << std::endl;
		return;
	}

	//展示指定数据表的基本属性，传入参数为该数据表的引用
	void Operate::TableShowInfo(Table& target, std::ostream& o)
	{
		o << "Field	Type	Null	Key	Default	Extra\n";
		//auto end = target.DataAddress.end();
		//for (auto it = target.DataAddress.begin(); it != end; ++it)
		for (auto iter = target.columnNames.begin(); iter != target.columnNames.end(); ++iter)
		{
			auto it = target.DataAddress.find(*iter);
			if (it->first == hiddenPrimaryKey) continue;
			//名称
			o << it->first << '\t';
			//数据类型
			switch (it->second.type)
			{
				case typeInt: o << "int(11)\t"; break;
				case typeDouble: o << "double\t"; break;
				case typeChar: o << "char(1)\t"; break;
			}
			//NOT NULL - 设定为NOT NULL应输出NO，反之输出YES
			if (it->second.notNull)
				o << "NO\t";
			else
				o << "YES\t";
			if (it->first == target.PrimaryCol) o << "PRI\t";
			else o << "\t";
			o << "NULL\t";
			o << std::endl;
		}
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//根据样例略微调整
	}

	//---------------------------------------------------------------------------------------------------------------------------------------//

	//加入一行数据，传入参数为进行操作的数据表、一行中各列的值（按照固定的顺序排列）
	bool Operate::DataInsert(Table& target, std::vector<DataUpdateAttr>& attr)
	{
		Data* newData = new Data;//首先开辟足够的空间
		auto AddEnd = target.DataAddress.end();
		for (auto AddIt = target.DataAddress.begin(); AddIt != AddEnd; ++AddIt)
		{
			switch (AddIt->second.type)
			{
				case typeInt: newData->valInt.push_back(0x3f3f3f); break;
				case typeDouble: newData->valDouble.push_back(0x3f3f3f); break;
				case typeChar: newData->valString.push_back("NULL"); break;
			}
		}
		//然后初始化值
		auto end = attr.end();
		for (auto it = attr.begin(); it != end; ++it)//对于每一列
		{
			const DataAddressType& info = target.DataAddress[it->colName];
			switch (info.type)
			{
				case typeInt: newData->valInt[info.pos] = SubStringToNum<int>(it->val); break;
				case typeDouble: newData->valDouble[info.pos] = SubStringToNum<double>(it->val); break;
				case typeChar: newData->valString[info.pos] = std::move(it->val); break;
			}
		}
		//调用底层接口
		bool result;
		DataAddressType& keyInfo = target.DataAddress[target.PrimaryCol];
		switch (keyInfo.type)
		{
			case typeInt: result = target.GetTreeInt(TableCreateAttr(target.PrimaryCol, typeInt))->Insert(newData->valInt[keyInfo.pos], newData); break;
			case typeDouble: result = target.GetTreeDouble(TableCreateAttr(target.PrimaryCol, typeDouble))->Insert(newData->valDouble[keyInfo.pos], newData); break;
			case typeChar: result = target.GetTreeChar(TableCreateAttr(target.PrimaryCol, typeChar))->Insert(newData->valString[keyInfo.pos], newData); break;
		}
		return result;
	}

	//删除符合要求的行，传入参数为进行操作的数据表、WHERE子句
	bool Operate::DataDelete(Table& target, WhereAttr& where)
	{
		auto range = SubWhere(target, where);
		const DataAddressType& info = target.DataAddress[target.PrimaryCol];//获得key值的信息
		auto end = range.end();
#ifdef DEBUG
		int recordID = 0;
#endif
		for (auto it = range.begin(); it != end; ++it)//对于每一行Data
		{
#ifdef DEBUG
			std::cerr << "Deleting record #" << ++recordID << "/" << range.size() << '\n';
#endif
			bool result;
			switch (info.type)//调用主键树的删除接口，传入本行Data的主键值
			{
				case typeInt: result = target.GetTreeInt(TableCreateAttr(target.PrimaryCol, typeInt))->Delete((*it)->valInt[info.pos]); break;
				case typeDouble: result = target.GetTreeDouble(TableCreateAttr(target.PrimaryCol, typeDouble))->Delete((*it)->valDouble[info.pos]); break;
				case typeChar: result = target.GetTreeChar(TableCreateAttr(target.PrimaryCol, typeChar))->Delete((*it)->valString[info.pos]); break;
			}
			if (result == false)//如果删除失败
				return false;
			delete *it;//delete本行Data
		}
		return true;
	}

	//修改符合要求的行，传入参数为进行操作的数据表、WHERE子句、需要修改的列、修改后的值
	bool Operate::DataUpdate(Table& target, WhereAttr& where, std::vector<DataUpdateAttr>& attr)
	{
		auto range = SubWhere(target, where);//range的每一个元素都是一个Data指针
		auto end = range.end();
		for (auto it = range.begin(); it != end; ++it)//对每个Data进行操作
		{
			auto attrEnd = attr.end();
			for (auto attrIt = attr.begin(); attrIt != attrEnd; ++attrIt)//对每一个需要修改的列
			{
				const DataAddressType& info = target.DataAddress[attrIt->colName];//获取该列的基本信息
				bool result = true;
				if (attrIt->colName == target.PrimaryCol)//如果修改的是主键，需要调用底层接口来调整索引
				{
					switch (info.type)
					{
						case typeInt:
						{
							int tmpOld = (*it)->valInt[info.pos];//记录旧数据
							int tmpNew = SubStringToNum<int>(attrIt->val);//从传入参数中解析出新数据
							(*it)->valInt[info.pos] = tmpNew;
							result = target.IntTreeList[attrIt->colName]->Update(tmpOld, tmpNew);
							break;
						}
						case typeDouble:
						{
							double tmpOld = (*it)->valDouble[info.pos];//记录旧数据
							double tmpNew = SubStringToNum<double>(attrIt->val);//从传入参数中解析出新数据
							(*it)->valDouble[info.pos] = tmpNew;
							result = target.DoubleTreeList[attrIt->colName]->Update(tmpOld, tmpNew);
							break;
						}
						case typeChar:
						{
							std::string tmpOld = std::move((*it)->valString[info.pos]);//记录旧数据
							std::string tmpNew = std::move(attrIt->val);//从传入参数中解析出新数据
							(*it)->valString[info.pos] = tmpNew;
							result = target.CharTreeList[attrIt->colName]->Update(tmpOld, tmpNew);
							break;
						}
					}
				}
				else//如果修改的不是主键，就只需要修改值
				{
					switch (info.type)
					{
						case typeInt: (*it)->valInt[info.pos] = SubStringToNum<int>(attrIt->val); break;
						case typeDouble: (*it)->valDouble[info.pos] = SubStringToNum<double>(attrIt->val); break;
						case typeChar: (*it)->valString[info.pos] = attrIt->val; break;
					}
				}
				if (result == false)
					return false;
			}

		}
		return true;
	}

	//查询（打印出）符合要求的行，传入参数为进行操作的数据表、WHERE子句
	void Operate::DataShow(Table& target, std::vector<std::string>& colName, WhereAttr& where, bool withTitle, std::ostream& o)
	{
		TemporaryTable result = select(target, where);
		if (result.size() == 0) // 如果没有元素，即没有符合要求的行
			return;//则不输出任何信息
		if (withTitle)
		{
			for (auto it = colName.begin(); it != colName.end(); ++it) //输出列名
				o << *it << "\t";
			o << std::endl;
		}
		result.print(colName, o);
		return;
	}

	// 根据条件筛选数据
	TemporaryTable Operate::select(Table& target, WhereAttr& where)
	{
		return TemporaryTable(target, SubWhere(target, where));
	}

	//---------------------------------------------------------------------------------------------------------------------------------------//

	//SubWhere中使用的子模块，用于进行比较
	template<typename T>
	bool Operate::SubCompare(const T& left, const T& right, WhereAttrSub2& ope)
	{
		switch (ope.compareType)
		{
			case greater: return left > right;
			case equal: return left == right;
			case less: return left < right;
			case greaterEqual: return left >= right;
			case lessEqual: return left <= right;
			case notEqual: return left != right;
		}
	}
	//whereClauses子句，传入数据表以及条件，返回符合要求的行构成的数组
	std::vector<Data*> Operate::SubWhere(Table& target, WhereAttr& attr)
	{
		if (attr.mode == all)
			return SubGetAllData(target);//调用返回全部data的方法

		else if (attr.mode == traversal)//使用遍历的方式筛选Data
		{
			std::vector<Data*> allData = SubGetAllData(target);//调用返回全部data的接口
			std::vector<Data*> returnData;//建立空数组存储符合要求的Data
			auto end = allData.end();
			for (auto it = allData.begin(); it != end; ++it)//对于每一个Data，根据传入的条件判断其是否符合要求
			{
				std::vector<WhereAttrSub2> localAttr = attr.traversalAttr;//由于下面的操作涉及对参数数组的修改，需要创建局部变量
				auto attrEnd = localAttr.end();
				//首先处理全部的比较运算符
				std::list<bool> comResult;//记录比较运算符的计算结果
				for (auto attrIt = localAttr.begin(); attrIt != attrEnd; ++attrIt)//遍历传入的表达式
					if (attrIt->type == compare)//只取出其中的比较运算符
					{
						auto leftIt = attrIt;//获得比较运算符两侧的操作数
						--leftIt;
						auto rightIt = attrIt;
						++rightIt;
						DataType type = leftIt->valType;//获得操作数的数据类型
						switch (type)//根据数据类型分类
						{
							case typeInt://数据类型为int
							{
								int leftVal;
								int rightVal;
								if (leftIt->type == valConst)//如果是常量
									leftVal = SubStringToNum<int>(leftIt->valOrName);
								else//如果是列名
									leftVal = (*it)->valInt[target.DataAddress[leftIt->valOrName].pos];
								if (rightIt->type == valConst)//如果是常量
									rightVal = SubStringToNum<int>(rightIt->valOrName);
								else//如果是列名
									rightVal = (*it)->valInt[target.DataAddress[rightIt->valOrName].pos];
								//判定是否为null
								if (leftVal == 0x3f3f3f || rightVal == 0x3f3f3f)
									comResult.push_back(false);
								else
									comResult.push_back(SubCompare(leftVal, rightVal, *attrIt));
								break;
							}
							case typeDouble://数据类型为double
							{
								double leftVal;
								double rightVal;
								if (leftIt->type == valConst)//如果是常量
									leftVal = SubStringToNum<double>(leftIt->valOrName);
								else//如果是列名
									leftVal = (*it)->valDouble[target.DataAddress[leftIt->valOrName].pos];
								if (rightIt->type == valConst)//如果是常量
									rightVal = SubStringToNum<double>(rightIt->valOrName);
								else//如果是列名
									rightVal = (*it)->valDouble[target.DataAddress[rightIt->valOrName].pos];
								//判定是否为null
								if (leftVal == 0x3f3f3f || rightVal == 0x3f3f3f)
									comResult.push_back(false);
								else
									comResult.push_back(SubCompare(leftVal, rightVal, *attrIt));
								break;
							}
							case typeChar://数据类型为char
							{
								const std::string* leftVal;
								const std::string* rightVal;
								if (leftIt->type == valConst)
									leftVal = &(leftIt->valOrName);
								else
									leftVal = &((*it)->valString[target.DataAddress[leftIt->valOrName].pos]);
								if (rightIt->type == valConst)
									rightVal = &(rightIt->valOrName);
								else
									rightVal = &((*it)->valString[target.DataAddress[rightIt->valOrName].pos]);
								//判定是否为null
								if (*leftVal == "NULL" || *rightVal == "NULL")
									comResult.push_back(false);
								else
									comResult.push_back(SubCompare(*leftVal, *rightVal, *attrIt));
								break;
							}
						}//完成一个比较运算符
					}
				//完成全部比较运算符，下面开始处理逻辑运算符and or not
				while (true)//每次循环找到当前优先级最高的逻辑运算符
				{
					auto cursor = localAttr.begin();//指向运算优先级最高的逻辑运算符
					auto rightVal = comResult.begin();//指向运算优先级最高的逻辑运算符右侧的比较运算的结果
					bool flag = true;//指示cursor是否被设置过
					auto comIt = comResult.begin();
					for (auto attrIt = localAttr.begin(); attrIt != attrEnd; ++attrIt)//遍历整个表达式
					{
						if (attrIt->type == logic)//只取出逻辑运算符
							if (flag || attrIt->logicType > cursor->logicType)//如果当前光标仍为空或者此运算符优先级更高
							{
								cursor = attrIt;//使cursor指向优先级更高的逻辑运算符
								flag = false;//表示cursor被设置过了
								rightVal = comIt;//根据两个数组同时遍历的逻辑，此时comIt必然指向逻辑运算符右侧的比较运算的结果
							}
						if (attrIt->type == compare)//同时也在遍历之前的comResult数组，每当遇到比较运算符结点，就向后移动一位
							++comIt;
					}
					if (flag)//若全部逻辑运算符均运算完毕，则结束循环
						break;
					cursor->type = used;//当前的逻辑运算符结点在完成本次运算后应当被删除，方式是将其结点类型设置used
					switch (cursor->logicType)
					{
						case oprNot: *rightVal = !(*rightVal); break;//直接对此元素取非
						case oprAnd:
						{
							auto leftVal = rightVal;
							--leftVal;//comIt2指向左操作数
							*leftVal = (*leftVal) && (*rightVal);//左操作数修改为计算结果
							comResult.erase(rightVal);//删除右操作数
							cursor += 2;//同时将右侧比较运算符在表达式中对应的结点清空
							cursor->type = used;
							break;
						}
						case oprOr:
						{
							auto leftVal = rightVal;
							--leftVal;//comIt2指向左操作数
							*leftVal = (*leftVal) || (*rightVal);//左操作数修改为计算结果
							comResult.erase(rightVal);//删除右操作数
							cursor += 2;//同时将右侧比较运算符在表达式中对应的结点清空
							cursor->type = used;
							break;
						}
					}
				}
				//处理完全部逻辑运算符，目前comResult数组中应当只有一个元素，即最终结果
				if (*comResult.begin())//如果最终结果是true
					returnData.push_back(*it);//将此Data的指针放入结果数组
			}
			return returnData;
		}

		else//利用BPTree特点快速检索
		{
			auto end = attr.searchAttr.end();
			//处理全部NOT运算符
			for (auto it = attr.searchAttr.begin(); it != end; ++it)//遍历整个表达式
				if (it->type == logic && it->logicType == oprNot)//取出not
				{
					it->type = used;//清空该结点
					++it;
					if (it->type == clause)//not后方的结点应当都是比较运算
						switch (attr.searchCompare[it->pos].compareType)
						{
							case greater: attr.searchCompare[it->pos].compareType = less; break;
							case equal: attr.searchCompare[it->pos].compareType = notEqual; break;
							case less: attr.searchCompare[it->pos].compareType = greater; break;
							case greaterEqual: attr.searchCompare[it->pos].compareType = lessEqual; break;
							case lessEqual: attr.searchCompare[it->pos].compareType = greaterEqual; break;
							case notEqual: attr.searchCompare[it->pos].compareType = equal; break;
						}
				}
			//处理全部比较运算，能够合并的首先合并
			std::list<std::vector<Data*> > result;//新建空数组保存结果，内层是各比较运算得到的Data指针数组
			for (auto it = attr.searchAttr.begin(); it != end; ++it)//遍历整个表达式
				if (it->type == clause)//取出比较运算
				{
					auto it2 = it;
					++it2;
					if (it2 != end && it2->type == logic && it2->logicType == oprAnd)//如果此比较运算的后方是and
					{
						++it2;//再取出and后方的比较运算
						if (it2 != end && it2->type == clause && attr.searchCompare[it->pos].colName == attr.searchCompare[it2->pos].colName)//如果二者是对同一列
						{
							it2->type = used;//清空中间的逻辑运算符结点和后面的比较运算结点，仅保留前面的比较运算结点
							auto it3 = it;
							++it3;
							it3->type = used;
							WhereAttrSub1* com1 = &(attr.searchCompare[it->pos]);//构造新变量避免反复查找
							WhereAttrSub1* com2 = &(attr.searchCompare[it2->pos]);
							bool com1LessCom2;//确保com1中的常量更小
							switch (com1->type)
							{
								case typeInt: com1LessCom2 = SubStringToNum<int>(com1->val) < SubStringToNum<int>(com2->val); break;
								case typeDouble: com1LessCom2 = SubStringToNum<double>(com1->val) < SubStringToNum<double>(com2->val); break;
								case typeChar: com1LessCom2 = com1->val < com2->val; break;
							}
							if (!com1LessCom2)
							{
								auto tmp = com1;
								com1 = com2;
								com2 = tmp;
							}
							std::vector<Data*> compareResult;//创建空数组保存结果
							//下面分情况处理
							//中间有共同区间的情况
							if ((com1->compareType == notEqual || com1->compareType == greater || com1->compareType == greaterEqual) && (com2->compareType == notEqual || com2->compareType == less || com2->compareType == lessEqual))
							{
								std::vector<Data*> midResult;//保存中间的结果
								bool leftClose = com1->compareType == greaterEqual ? true : false;//若为大于等于则左侧闭，否则左侧均为开
								bool rightClose = com2->compareType == lessEqual ? true : false;
								switch (com1->type)
								{
									case typeInt: midResult = target.IntTreeList[com1->colName]->BatchSearch(SubStringToNum<int>(com1->val), SubStringToNum<int>(com2->val), leftClose, rightClose); break;
									case typeDouble: midResult = target.DoubleTreeList[com1->colName]->BatchSearch(SubStringToNum<double>(com1->val), SubStringToNum<double>(com2->val), leftClose, rightClose); break;
									case typeChar: midResult = target.CharTreeList[com1->colName]->BatchSearch(com1->val, com2->val, leftClose, rightClose); break;
								}
								if (com1->compareType == notEqual)//如果左侧还有区间
								{
									std::vector<Data*> tmpResult;
									switch (com1->type)
									{
										case typeInt: tmpResult = target.IntTreeList[com1->colName]->BatchSearch(SubStringToNum<int>(com1->val), false, false); break;
										case typeDouble: tmpResult = target.DoubleTreeList[com1->colName]->BatchSearch(SubStringToNum<double>(com1->val), false, false); break;
										case typeChar: tmpResult = target.CharTreeList[com1->colName]->BatchSearch(com1->val, false, false); break;
									}
									std::vector<Data*> afterUnion;
									SubUnion(midResult, tmpResult, afterUnion);
									midResult = std::move(afterUnion);
								}
								if (com2->compareType == notEqual)//如果右侧还有区间
								{
									std::vector<Data*> tmpResult;
									switch (com2->type)
									{
										case typeInt: tmpResult = target.IntTreeList[com2->colName]->BatchSearch(SubStringToNum<int>(com2->val), false, true); break;
										case typeDouble: tmpResult = target.DoubleTreeList[com2->colName]->BatchSearch(SubStringToNum<double>(com2->val), false, true); break;
										case typeChar: tmpResult = target.CharTreeList[com2->colName]->BatchSearch(com2->val, false, true); break;
									}
									std::vector<Data*> afterUnion;
									SubUnion(midResult, tmpResult, afterUnion);
									midResult = std::move(afterUnion);
								}
								compareResult = std::move(midResult);
							}
							//如果中间没有共同区间，同时左侧有共同区间，即左结点不能向右，同时右结点有向左
							else if((com1->compareType == less || com1->compareType == lessEqual || com1->compareType == equal) && (com2->compareType == less || com2->compareType == lessEqual || com2->compareType == notEqual))
							{
								if (com1->compareType == equal)//首先处理等于的情况
								{
									switch (com1->type)
									{
										case typeInt: compareResult = target.IntTreeList[com1->colName]->BatchSearch(SubStringToNum<int>(com1->val), SubStringToNum<int>(com1->val), true, true); break;
										case typeDouble: compareResult = target.DoubleTreeList[com1->colName]->BatchSearch(SubStringToNum<double>(com1->val), SubStringToNum<double>(com1->val), true, true); break;
										case typeChar: compareResult = target.CharTreeList[com1->colName]->BatchSearch(com1->val, com1->val, true, true); break;
									}
								}
								else//不是等于的情况
								{
									bool leftClose = com1->compareType == lessEqual ? true : false;//若为小于等于，则为闭区间
									switch (com1->type)
									{
										case typeInt: compareResult = target.IntTreeList[com1->colName]->BatchSearch(SubStringToNum<int>(com1->val), leftClose, false); break;
										case typeDouble: compareResult = target.DoubleTreeList[com1->colName]->BatchSearch(SubStringToNum<double>(com1->val), leftClose, false); break;
										case typeChar: compareResult = target.CharTreeList[com1->colName]->BatchSearch(com1->val, leftClose, false); break;
									}
								}
							}
							//如果中间和左侧都没有共同区间，但是右侧有共同区间，由于上面已经排除了左结点不向右的情况，所以只考虑右结点不能向左的情况
							else if (com2->compareType == greater || com2->compareType ==  greaterEqual || com2->compareType == equal)
							{
								if (com2->compareType == equal)//首先处理等于
								{
									switch (com2->type)
									{
										case typeInt: compareResult = target.IntTreeList[com2->colName]->BatchSearch(SubStringToNum<int>(com2->val), SubStringToNum<int>(com2->val), true, true); break;
										case typeDouble: compareResult = target.DoubleTreeList[com2->colName]->BatchSearch(SubStringToNum<double>(com2->val), SubStringToNum<double>(com2->val), true, true); break;
										case typeChar: compareResult = target.CharTreeList[com2->colName]->BatchSearch(com2->val, com2->val, true, true); break;
									}
								}
								else
								{
									bool rightClose = com2->compareType == greaterEqual ? true : false;
									switch (com2->type)
									{
										case typeInt: compareResult = target.IntTreeList[com2->colName]->BatchSearch(SubStringToNum<int>(com2->val), rightClose, true); break;
										case typeDouble: compareResult = target.DoubleTreeList[com2->colName]->BatchSearch(SubStringToNum<double>(com2->val), rightClose, true); break;
										case typeChar: compareResult = target.CharTreeList[com2->colName]->BatchSearch(com2->val, rightClose, true); break;
									}
								}
							}
							//没有共同区间
							else
								bool a = true;//什么都不做，让compareResult为空
							result.push_back(std::move(compareResult));
							continue;
						}
					}
					//如果不满足上面的条件，就按单独的比较运算处理
					std::vector<Data*> compareResult;
					WhereAttrSub1* com = &(attr.searchCompare[it->pos]);
					switch (com->compareType)
					{
						case greater:
						{
							switch (com->type)
							{
								case typeInt: compareResult = target.IntTreeList[com->colName]->BatchSearch(SubStringToNum<int>(com->val), false, true); break;
								case typeDouble: compareResult = target.DoubleTreeList[com->colName]->BatchSearch(SubStringToNum<double>(com->val), false, true); break;
								case typeChar: compareResult = target.CharTreeList[com->colName]->BatchSearch(com->val, false, true); break;
							}
							break;
						}
						case less:
						{
							switch (com->type)
							{
								case typeInt: compareResult = target.IntTreeList[com->colName]->BatchSearch(SubStringToNum<int>(com->val), false, false); break;
								case typeDouble: compareResult = target.DoubleTreeList[com->colName]->BatchSearch(SubStringToNum<double>(com->val), false, false); break;
								case typeChar: compareResult = target.CharTreeList[com->colName]->BatchSearch(com->val, false, false); break;
							}
							break;
						}
						case greaterEqual:
						{
							switch (com->type)
							{
								case typeInt: compareResult = target.IntTreeList[com->colName]->BatchSearch(SubStringToNum<int>(com->val), true, true); break;
								case typeDouble: compareResult = target.DoubleTreeList[com->colName]->BatchSearch(SubStringToNum<double>(com->val), true, true); break;
								case typeChar: compareResult = target.CharTreeList[com->colName]->BatchSearch(com->val, true, true); break;
							}
							break;
						}
						case lessEqual:
						{
							switch (com->type)
							{
								case typeInt: compareResult = target.IntTreeList[com->colName]->BatchSearch(SubStringToNum<int>(com->val), true, false); break;
								case typeDouble: compareResult = target.DoubleTreeList[com->colName]->BatchSearch(SubStringToNum<double>(com->val), true, false); break;
								case typeChar: compareResult = target.CharTreeList[com->colName]->BatchSearch(com->val, true, false); break;
							}
							break;
						}
						case equal:
						{
							switch (com->type)
							{
								case typeInt: compareResult = target.IntTreeList[com->colName]->BatchSearch(SubStringToNum<int>(com->val), SubStringToNum<int>(com->val), true, true); break;
								case typeDouble: compareResult = target.DoubleTreeList[com->colName]->BatchSearch(SubStringToNum<double>(com->val), SubStringToNum<double>(com->val), true, true); break;
								case typeChar: compareResult = target.CharTreeList[com->colName]->BatchSearch(com->val, com->val, true, true); break;
							}
							break;
						}
						case notEqual:
						{
							std::vector<Data*> tmpResult1;
							std::vector<Data*> tmpResult2;
							switch (com->type)//先分别得到两边
							{
								case typeInt: tmpResult1 = target.IntTreeList[com->colName]->BatchSearch(SubStringToNum<int>(com->val), false, true); tmpResult2 = target.IntTreeList[com->colName]->BatchSearch(SubStringToNum<int>(com->val), false, false); break;
								case typeDouble: tmpResult1 = target.DoubleTreeList[com->colName]->BatchSearch(SubStringToNum<double>(com->val), false, true); tmpResult2 = target.DoubleTreeList[com->colName]->BatchSearch(SubStringToNum<double>(com->val), false, false); break;
								case typeChar: tmpResult1 = target.CharTreeList[com->colName]->BatchSearch(com->val, false, true); tmpResult2 = target.CharTreeList[com->colName]->BatchSearch(com->val, false, false); break;
							}
							SubUnion(tmpResult1, tmpResult2, compareResult);//再取并集
							break;
						}
					}
					result.push_back(std::move(compareResult));
				}
			//根据逻辑运算符对上面得到的各集合取交并
			while (true)
			{
				auto cursor = attr.searchAttr.begin();//应指向运算优先级最高的逻辑运算符
				auto rightVal = result.begin();//指向上面找到的逻辑运算符右侧的比较运算结果
				bool flag = true;//指示cursor是否被设置过
				auto resultIt = result.begin();//用于遍历比较运算的结果
				for (auto it = attr.searchAttr.begin(); it != end; ++it)//遍历整个表达式
				{
					if (it->type == logic)//若是逻辑运算符
						if (flag || cursor->logicType < it->logicType)
						{
							cursor = it;//令cursor指向优先级更高的逻辑运算符
							flag = false;//指示cursor被设置过
							rightVal = resultIt;//根据同时遍历的逻辑，此时resultIt必然指向被找到的优先级更高的逻辑运算符右侧的比较运算的结果
						}
					if (it->type == clause)//同时遍历比较运算的结果
						++resultIt;
				}
				if (flag)//终止条件
					break;
				cursor->type = used;//在表达式中清空该逻辑运算符的结点
				auto leftVal = rightVal;
				--leftVal;//指向左侧比较运算的结果
				if (cursor->logicType == oprAnd)//and运算符
				{
					std::vector<Data*> tmpResult;
					SubIntersection(*leftVal, *rightVal, tmpResult);//取交集
					*leftVal = std::move(tmpResult);//左侧的结果修改为交集结果
					result.erase(rightVal);//删除右侧结果
					++cursor;//同时在表达式中清空代表右操作数的结点
					cursor->type = used;
				}
				else
				{
					std::vector<Data*> tmpResult;
					SubUnion(*leftVal, *rightVal, tmpResult);//取并集
					*leftVal = std::move(tmpResult);//左侧的结果修改为并集结果
					result.erase(rightVal);//删除右侧结果
					++cursor;//同时在表达式中清空代表右操作数的结点
					cursor->type = used;
				}
			}
			//当前表达式中全部逻辑运算符均运算完毕，result中只有一个结点，即结果
			auto finalEnd = result.begin()->end();
			//清理主键为NULL的数据
			switch (target.DataAddress[target.PrimaryCol].type)
			{
				case typeInt:
				{
					for (auto finalIt = result.begin()->begin(); finalIt != finalEnd; ++finalIt)
						if ((*finalIt)->valInt[target.DataAddress[target.PrimaryCol].pos] == 0x3f3f3f)
							result.begin()->erase(finalIt);
					break;
				}
				case typeDouble:
				{
					for (auto finalIt = result.begin()->begin(); finalIt != finalEnd; ++finalIt)
						if ((*finalIt)->valDouble[target.DataAddress[target.PrimaryCol].pos] == 0x3f3f3f)
							result.begin()->erase(finalIt);
					break;
				}
				case typeChar:
				{
					for (auto finalIt = result.begin()->begin(); finalIt != finalEnd; ++finalIt)
						if ((*finalIt)->valString[target.DataAddress[target.PrimaryCol].pos] == "NULL")
							result.begin()->erase(finalIt);
					break;
				}
			}
			return *(result.begin());
		}
	}

	//用于将以string形式传入的参数还原为原始的类型
	template<typename T>
	T Operate::SubStringToNum(const std::string& valS)
	{
		std::istringstream ss{valS};
		T valNum;
		ss >> valNum;
		return valNum;
	}

	//取交集
	template<typename T>
	void Operate::SubIntersection(std::vector<T>& first, std::vector<T>& second, std::vector<T>& result)//应当确保result是一个空数组
	{
		//首先进行排序
		std::sort(first.begin(), first.end());
		std::sort(second.begin(), second.end());
		auto firstIt = first.begin();
		auto firstEnd = first.end();
		auto secondIt = second.begin();
		auto secondEnd = second.end();
		while (firstIt != firstEnd && secondIt != secondEnd)
		{
			if (*firstIt < *secondIt)
				++firstIt;
			else if (*secondIt < *firstIt)
				++secondIt;
			else
			{
				result.push_back(*firstIt);
				++firstIt;
				++secondIt;
			}
		}
		return;
	}

	//取并集
	template<typename T>
	void Operate::SubUnion(std::vector<T>& first, std::vector<T>& second, std::vector<T>& result)//应当确保result是一个空数组
	{
		//首先进行排序
		std::sort(first.begin(), first.end());
		std::sort(second.begin(), second.end());
		auto firstIt = first.begin();
		auto firstEnd = first.end();
		auto secondIt = second.begin();
		auto secondEnd = second.end();
		while (true)
		{
			if (firstIt == firstEnd)
			{
				while (secondIt != secondEnd)
				{
					result.push_back(*secondIt);
					++secondIt;
				}
				return;
			}
			if (secondIt == secondEnd)
			{
				while (firstIt != firstEnd)
				{
					result.push_back(*firstIt);
					++firstIt;
				}
				return;
			}

			if (*firstIt < *secondIt)
			{
				result.push_back(*firstIt);
				++firstIt;
			}
			else if (*firstIt > *secondIt)
			{
				result.push_back(*secondIt);
				++secondIt;
			}
			else
			{
				result.push_back(*firstIt);
				++firstIt;
				++secondIt;
			}
		}
	}

	//返回给定Table的全部Data
	std::vector<Data*> Operate::SubGetAllData(Table& target)
	{
		switch (target.DataAddress[target.PrimaryCol].type)
		{
			case typeInt: return target.IntTreeList[target.PrimaryCol]->GetData(); break;
			case typeDouble: return target.DoubleTreeList[target.PrimaryCol]->GetData(); break;
			case typeChar: return target.CharTreeList[target.PrimaryCol]->GetData(); break;
		}
	}

	//在DataShow中调用，用于对得到的Data*数组排序，依据是key值大小
	bool SubDataShowCom::operator()(const Data* left, const Data* right)
	{
		switch (mode)//根据数据类型分别处理
		{
			case 1: return left->valInt[pos] < right->valInt[pos]; break;
			case 2: return left->valDouble[pos] < right->valDouble[pos]; break;
			case 3: return left->valString[pos] < right->valString[pos]; break;
		}
	}
}

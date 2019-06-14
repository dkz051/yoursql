#include "session.h"

#include <fstream>

#include "tools.h"

namespace OOPD
{
	// 根据名称获取数据库的指针。如果省略数据库名则返回当前活动数据库。如果该数据库不存在则抛出异常
	DataBase* Session::getDatabase(const std::string& dbName)
	{
		std::string dbn = dbName == "" ? selected : dbName;
		if (db.count(dbn)) return db[dbn];
		else throw "Database does not exist";
	}

	// 根据名称获取表的指针。如果省略数据库名则在当前活动数据库中查找。如果该表不存在则抛出异常
	Table* Session::getTable(const std::string& tableName, const std::string& dbName)
	{
		DataBase* db = getDatabase(dbName);
		if (db->TableList.count(tableName)) return db->TableList[tableName];
		else throw "Table does not exist";
	}

	void Session::start()
	{
		std::string command;
		while (getline(std::cin, command, ';'))
			execute(command, std::cout);
		return;
	}

	std::string Session::execute(std::string sql, std::ostream& o)
	{
		auto str = tokenize(sql), strLower = str; // 调用 tokenize 函数分词
		std::transform(strLower.begin(), strLower.end(), strLower.begin(), ::stringToLower); // 全部小写

		if (str.size() < 1) return "Not a statement"; // 不是完整语句

		if (strLower[0] == "create")
		{
			if (strLower[1] == "database")
				createDatabase(str[2]);
			else if (strLower[1] == "table")
			{
				auto& tableName = str[2];
				auto traits = tokens(str.begin() + 4, str.end() - 1);
				createTable(str[2], traits);
			}
		}
		if (strLower[0] == "drop")
		{
			if (strLower[1] == "database")
				dropDatabase(str[2]);
			else if (strLower[1] == "table")
				dropTable(str[2]);
		}
		if (strLower[0] == "use" && strLower[1] != "tables")
			use(str[1]);
		if (strLower[0] == "show")
		{
			if (strLower[1] == "databases")
				opt.DBShow(db, o);
			else if (strLower[1] == "tables")
				opt.TableShow(*getDatabase(), selected, o);
			else if (strLower[1] == "columns")
				opt.TableShowInfo(*getDatabase()->TableList[str[3]], o);
		}
		if (strLower[0] == "insert" && strLower[1] == "into")
		{
			auto t = std::find(strLower.begin(), strLower.end(), "values") - strLower.begin();
			insert(
				str[2],
				tokens(str.begin() + 4, str.begin() + t - 1),
				tokens(str.begin() + t + 2, str.end() - 1)
			);
		}
		if (strLower[0] == "load")
		{
			auto p = std::find(strLower.begin(), strLower.end(), "infile") - strLower.begin();
			std::string inputFileName = parseStringLiteral(str[p + 1]);
			auto t = std::find(strLower.begin(), strLower.end(), ")") - strLower.begin();
			auto& tableName = str[p + 4];
			auto attrName = tokens(str.begin() + p + 6, str.begin() + t);
			std::ifstream i(inputFileName);
			if (!i.is_open())
				throw "Cannot open input file.";
			std::string line;
			while (getline(i, line))
			{
				attrs values = tokenize(line);
				tokens attrValue;
				for (auto iter = values.begin(); iter != values.end(); ++iter)
				{
					if (iter != values.begin()) attrValue.push_back(",");
					attrValue.push_back(*iter);
				}
				insertRaw(tableName, attrName, attrValue);
			}
		}
		if (strLower[0] == "delete" && strLower[1] == "from")
		{
			auto t = find(strLower.begin(), strLower.end(), "where") - strLower.begin();
			if (t == (int)strLower.size())
				remove(str[2]);
			else
				remove(str[2], tokens(str.begin() + t + 1, str.end()));
		}
		if (strLower[0] == "update")
		{
			auto t = std::find(strLower.begin(), strLower.end(), "where") - strLower.begin();
			auto setClause = tokens(str.begin() + 3, str.begin() + t);
			if (t == (int)strLower.size())
				update(str[1], setClause);
			else
				update(str[1], setClause, tokens(str.begin() + t + 1, str.end()));
		}
		if (strLower[0] == "select")
		{
			auto f = std::find(strLower.begin(), strLower.end(), "into") - strLower.begin();
			std::ostream* os = nullptr;
			if (f == (int)strLower.size())
				os = &o;
			else
			{
				std::string outputFileName = parseStringLiteral(str[f + 2]);
				std::ofstream* of = new std::ofstream(outputFileName, std::ios::out);
				if (!of->is_open())
					throw "Cannot open output file. Does the output file already exist?";
				os = of;
			}
			auto p = std::find(strLower.begin(), strLower.end(), "from") - strLower.begin();
			auto& tableName = str[p + 1];

			Table& table = *getTable(tableName);

			auto attrName = tokens(str.begin() + 1, str.begin() + p);
			if (str[1] == "*")
				attrName = table.columnNames;
			for (auto iter = attrName.begin(); iter != attrName.end(); ++iter)
				if (*iter == hiddenPrimaryKey)
				{
					attrName.erase(iter);
					break;
				}

			auto t = std::find(strLower.begin(), strLower.end(), "where") - strLower.begin();
			std::string whereStr = "";
			if (t == (int)strLower.size())
			{
				auto where = SubWhere(table, whereStr);
				opt.DataShow(table, attrName, where, *os);
			}
			else
			{
				whereStr = concatenate(tokens(str.begin() + t + 1, str.end()));
				auto where = SubWhere(table, whereStr);
				opt.DataShow(table, attrName, where, *os);
			}
			if (os != &o) delete os;
		}
		return "Temporarily nothing returned";
	}

	bool Session::createDatabase(std::string dbName)
	{
		return opt.DBCreate(db, dbName);
	}

	bool Session::createTable(std::string tableName, const tokens& traits)
	{
		std::vector<TableCreateAttr> fields;
		std::string primaryKey;
		for (auto t = traits.begin(); t < traits.end(); )
		{
			auto p = find(t, traits.end(), ",");
			if (p - t == 5)
				primaryKey = *(t + 3);
			else
			{
				TableCreateAttr field;
				auto typeLower = stringToLower(*(t + 1));

				field.colName = *t;
				field.NotNull = ((p - t) == 4);
				field.Primary = false;

				if (typeLower == "int") field.type = typeInt;
				else if (typeLower == "double") field.type = typeDouble;
				else if (typeLower == "char") field.type = typeChar;
				else throw "Unrecognized data type";

				fields.push_back(field);
			}
			t = p + 1;
		}
		if (!primaryKey.empty())
		{
			for (auto iter = fields.begin(); iter != fields.end(); ++iter)
				if (iter->colName == primaryKey)
					iter->Primary = true;
		}
		else
		{
			TableCreateAttr field;
			field.colName = hiddenPrimaryKey;
			field.Primary = true;
			field.NotNull = true;
			field.type = typeInt;
			fields.push_back(field);
		}
		return opt.TableCreate(*getDatabase(), tableName, fields);
	}

	bool Session::dropDatabase(std::string dbName)
	{
		if (dbName == selected) selected = "";
		return opt.DBDelete(db, dbName);
	}

	bool Session::dropTable(std::string tableName)
	{
		return opt.TableDelete(*getDatabase(), tableName);
	}

	bool Session::use(std::string dbName)
	{
		if (db.count(dbName))
		{
			selected = dbName;
			return true;
		}
		return false;
	}

	int Session::insert(std::string tableName, const attrs& fields, const tokens& values)
	{
		if (fields.size() != values.size()) throw "Insert data broken";
		Table* table = getTable(tableName);
		std::vector<DataUpdateAttr> attr;
		for (auto iter = fields.begin(), jter = values.begin(); iter != fields.end(); ++iter, ++jter)
			if (*iter != ",")
			{
				std::string value = *jter;
				if (table->DataAddress[*iter].type == typeChar) value = parseStringLiteral(value);
				attr.push_back(DataUpdateAttr(*iter, value));
			}
		if (table->PrimaryCol == hiddenPrimaryKey)
			attr.push_back(DataUpdateAttr(hiddenPrimaryKey, std::to_string(table->rowID)));
		++table->rowID;
		return (int)opt.DataInsert(*table, attr);
	}

	int Session::insertRaw(std::string tableName, const attrs& fields, tokens values)
	{
		if (fields.size() != values.size()) throw "Insert data broken";

		Table* table = getTable(tableName);

		auto iter = fields.begin();
		auto jter = values.begin();
		for (; iter != fields.end(); ++iter, ++jter)
			if (table->DataAddress[*iter].type == typeChar)
				*jter = toStringLiteral(*jter);
		return insert(tableName, fields, values);
	}

	int Session::remove(const std::string& tableName, const tokens& whereClause)
	{
		Table& targetTable = *getTable(tableName);
		if (whereClause.empty())
		{
			std::string whereStr = "";
			auto temp = SubWhere(targetTable, whereStr);
			return opt.DataDelete(targetTable, temp);
		}
		std::string whereStr(concatenate(whereClause));
		auto temp = SubWhere(targetTable, whereStr);
		return opt.DataDelete(targetTable, temp);//调用SubWhere获得Where参数，并调用DataDelete方法
	}

	//bool Session::UPDATE(std::string& str3)
	int Session::update(const std::string& tableName, const tokens& setClause, const tokens& whereClause)
	{
		Table& targetTable = *getTable(tableName);

		std::vector<DataUpdateAttr> updateInfo;
		const std::string& field = setClause[0];
		std::string value = setClause[2];
		if (targetTable.DataAddress[field].type == typeChar)
			value = parseStringLiteral(value);

		updateInfo.push_back(DataUpdateAttr(setClause[0], value));

		if (whereClause.empty())
		{
			std::string whereStr = "";
			auto where = SubWhere(targetTable, whereStr);
			return opt.DataUpdate(targetTable, where, updateInfo);
		}
		else
		{
			std::string whereStr = concatenate(whereClause);
			auto where = SubWhere(targetTable, whereStr);
			return opt.DataUpdate(targetTable, where, updateInfo);
		}
	}

	WhereAttr Session::SubWhere(Table& target, std::string& wholeStr)
	{
		//首先判定是否传入空字符串（这是与调用函数的约定，若输入中未给出WHERE子句，则直接传入空字符串）
		if (wholeStr.size() == 0)
			return WhereAttr(all, std::vector<WhereAttrSub1>(), std::vector<WhereAttr::node>(), std::vector<WhereAttrSub2>());

		//构造一个WhereAttr对象，采用traversal模式，但只用于更加细分地存储当前表达式
		WhereAttr traversalWhere(traversal, std::vector<WhereAttrSub1>(), std::vector<WhereAttr::node>(), std::vector<WhereAttrSub2>());
		auto end = wholeStr.end();
		int pos = 0;//记录当前下标位置
		for (auto it = wholeStr.begin(); it != end; ++it)//遍历整个表达式
		{
			int endOfNode = -1;//记录当前结点的结束位置，这里是假设当前结点不是运算符，因此若其是运算符，则endOfNode == pos
			//在下面的检测过程中，会不断调试endOfNode的值，若未检测到对应字符(串)，则endOfNode会保持原值
			//若检测到对应字符(串)，且其位置比endOfNode记录的位置更靠前，则endOfNode调整为该位置
			//若发现某次检测后endOfNode == pos，则说明当前结点就是被检测的字符，因此将其信息压入参数结构并跳至下一个结点的判定
			//如果全部检测过后仍未进入下一个结点的判定，就说明当前结点不是运算符，于是根据endOfNode中记录的位置信息得到当前结点的长度，并对该结点继续进行分析
			if (*it == ' ')//若当前指向空格，则向后一位，重新开始判定过程
			{
				pos++;
				continue;
			}
			//首先检测比较运算符
			if (SubMin(endOfNode, wholeStr.find(">=", pos)) == pos && pos != -1)//若当前结点为>=
			{
				traversalWhere.traversalAttr.push_back(WhereAttrSub2(compare, oprOr, greaterEqual, typeInt, std::string()));//压入结点
				pos += 2;//移动pos跳过>=，进入下一个结点的判定
				++it;//同样移动it，但是for循环自带一次位移，因此只++
				continue;
			}
			if (SubMin(endOfNode, wholeStr.find("<=", pos)) == pos && pos != -1)//若当前结点为<=
			{
				traversalWhere.traversalAttr.push_back(WhereAttrSub2(compare, oprOr, lessEqual, typeInt, std::string()));//压入结点
				pos += 2;
				++it;
				continue;
			}
			if (SubMin(endOfNode, wholeStr.find_first_of('=', pos)) == pos && pos != -1)//若当前结点为=
			{
				traversalWhere.traversalAttr.push_back(WhereAttrSub2(compare, oprOr, equal, typeInt, std::string()));//压入结点
				pos++;
				continue;
			}
			if (SubMin(endOfNode, wholeStr.find("!=", pos)) == pos && pos != -1)//若当前结点为!=
			{
				traversalWhere.traversalAttr.push_back(WhereAttrSub2(compare, oprOr, notEqual, typeInt, std::string()));//压入结点
				pos += 2;
				++it;
				continue;
			}
			if (SubMin(endOfNode, wholeStr.find_first_of('>', pos)) == pos && pos != -1)//之前已经排除>=，若当前结点为>
			{
				traversalWhere.traversalAttr.push_back(WhereAttrSub2(compare, oprOr, greater, typeInt, std::string()));//压入结点
				pos++;
				continue;
			}
			if (SubMin(endOfNode, wholeStr.find_first_of('<', pos)) == pos && pos != -1)//之前已经排除<=，若当前结点为<
			{
				traversalWhere.traversalAttr.push_back(WhereAttrSub2(compare, oprOr, less, typeInt, std::string()));//压入结点
				pos++;
				continue;
			}
			//若执行到此位置，则说明当前结点不是比较运算符，于是进入逻辑运算符的判定
			//由于可能涉及大小写判定问题，所以先构造一个新的字符串，将全部字母改为大写
			std::string wholeStrUpper(stringToUpper(wholeStr));
			int andPos1 = wholeStrUpper.find("AND ", pos);
			int andPos2 = wholeStrUpper.find(" AND ", pos);
			if (andPos1 == pos && pos != -1)//若当前结点为AND
			{
				traversalWhere.traversalAttr.push_back(WhereAttrSub2(logic, oprAnd, greater, typeInt, std::string()));//压入结点
				pos += 4;
				it += 3;
				continue;
			}
			SubMin(endOfNode, andPos2);
			int orPos1 = wholeStrUpper.find("OR ", pos);
			int orPos2 = wholeStrUpper.find(" OR ", pos);
			if (orPos1 == pos && pos != -1)//若当前结点为OR
			{
				traversalWhere.traversalAttr.push_back(WhereAttrSub2(logic, oprOr, greater, typeInt, std::string()));//压入结点
				pos += 3;
				it += 2;
				continue;
			}
			SubMin(endOfNode, orPos2);
			int notPos1 = wholeStrUpper.find("NOT ", pos);
			int notPos2 = wholeStrUpper.find(" NOT ", pos);
			//假如有一个列名或变量名中含有NOT，可以确定：notPos1 != pos，因此如果两者相等就说明当前结点为NOT
			//否则当前结点不为NOT，即NOT前方一定有空格，就可以用第二个notPos2移动endOfNode位置
			if (notPos1 == pos && pos != -1)//若当前结点为NOT
			{
				traversalWhere.traversalAttr.push_back(WhereAttrSub2(logic, oprNot, greater, typeInt, std::string()));//压入结点
				pos += 4;
				it += 3;
				continue;
			}
			SubMin(endOfNode, notPos2);
			//若执行到当前位置，则说明当前结点不是运算符，需要判定其是列名还是常量
			//首先提取当前结点的信息
			int strLen = (endOfNode == -1 ? -1 : endOfNode - pos);//创建一个新的字符串，若endOfNode == -1，则应直接取到全长表达式的结尾，否则取出
			//endOfNode - pos长度
			std::string thisNode(wholeStr.substr(pos, strLen));
			while (*thisNode.rbegin() == ' ')//去除可能的尾部空格
				thisNode.erase(--thisNode.end());
			if (thisNode[0] == '"')//若以引号开始，则是字符串常量
			{
				traversalWhere.traversalAttr.push_back(WhereAttrSub2(valConst, oprOr, greater, typeChar, std::string(thisNode.substr(1, thisNode.size() - 2))));//长度-1去除后部引号
				pos += strLen;
				it += (strLen - 1);
				if (strLen == -1) break;
				continue;
			}
			if ((thisNode[0] >= '0' && thisNode[0] <= '9') || thisNode[0] == '-' || thisNode[0] == '.')//若以数字或负号或小数点开始，则是数字常量，需要再判定是int还是double
			{
				double valDouble = std::stod(thisNode);
				int valInt = int(valDouble);
				if (valInt == valDouble)//若没有切除，则认为是int
					traversalWhere.traversalAttr.push_back(WhereAttrSub2(valConst, oprOr, greater, typeInt, thisNode));
				else//认为是double
					traversalWhere.traversalAttr.push_back(WhereAttrSub2(valConst, oprOr, greater, typeDouble, thisNode));
				pos += strLen;
				it += (strLen - 1);
				if (strLen == -1) break;
				continue;
			}
			//若执行到此处，则说明当前结点只能是列名
			DataType colType = target.DataAddress[thisNode].type;;//判定此列的数据类型
			traversalWhere.traversalAttr.push_back(WhereAttrSub2(valCol, oprOr, greater, colType, thisNode));
			pos += strLen;
			it += (strLen - 1);
			if (strLen == -1) break; //漏写一句话，浪费一下午
		}
		//traversalWhere的创建已经完成，下面需要判定是否符合使用search模式的条件，即全部列名只涉及到主键且比较运算只涉及列与常量的比较
		//遍历traversalWhere中的traversalAttr数组以进行判定
		auto attrEnd = traversalWhere.traversalAttr.end();
		for (auto attrIt = traversalWhere.traversalAttr.begin(); attrIt != attrEnd; ++attrIt)
		{
			if (attrIt->type == valCol)//如果是列名
				if (attrIt->valOrName != target.PrimaryCol)//检测列名与主键是否一致
					return traversalWhere;//若不一致则结束检测
			if (attrIt->type == compare)//如果是比较运算符
			{
				auto leftVal = attrIt;
				--leftVal;
				auto rightVal = attrIt;
				++rightVal;
				if (leftVal->type + rightVal->type != valConst + valCol)//检测两侧的操作数是否一个是常量一个是列名
					return traversalWhere;//若不一致则结束检测
			}
		}
		//若执行到此处，则说明可以使用search模式，需要重新构造参数结构
		WhereAttr searchWhere(search, std::vector<WhereAttrSub1>(), std::vector<WhereAttr::node>(), std::vector<WhereAttrSub2>());
		int comparePos = 0;//用于记录表示比较运算的结点已经压入到哪个下标处
		for (auto attrIt = traversalWhere.traversalAttr.begin(); attrIt != attrEnd; ++attrIt)//遍历traversalWhere
		{
			if (attrIt->type == logic)//如果是逻辑运算符，则直接压入
				searchWhere.searchAttr.push_back(WhereAttr::node(logic, attrIt->logicType, 0));
			if (attrIt->type == compare)//如果是比较运算符，需要提取整个比较运算的信息并压入
			{
				auto constVal = attrIt;//指向比较运算中的常量
				++constVal;
				auto colVal = attrIt;//指向比较运算中的列名
				--colVal;//默认列名位于运算符左侧
				if (colVal->type == valConst)//若列名位于运算符右侧
				{
					constVal += 2;
					colVal -= 2;
					//处理比较运算符到合理的方向
					switch (attrIt->compareType)
					{
						case greater: attrIt->compareType = less; break;
						case less: attrIt->compareType = greater; break;
						case greaterEqual: attrIt->compareType = lessEqual; break;
						case lessEqual: attrIt->compareType = greaterEqual; break;
						default: break;
					}
				}
				//压入信息，首先将比较运算的信息压入searchCompare数组，然后将指向此处的信息压入searchAttr数组
				searchWhere.searchCompare.push_back(WhereAttrSub1(colVal->valOrName, attrIt->compareType, target.DataAddress[colVal->valOrName].type, constVal->valOrName));
				searchWhere.searchAttr.push_back(WhereAttr::node(clause, oprAnd, comparePos));
				++comparePos;//下标加一
			}
		}
		return searchWhere;
	}

	int& Session::SubMin(int& endOfNode, const int& num)
	{
		if (endOfNode == -1 || (endOfNode > num && num != -1))
			endOfNode = num;
		return endOfNode;
	}
}

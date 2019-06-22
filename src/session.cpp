#include "session.h"

#include <fstream>

#include "tools.h"

namespace OOPD
{
	// �������ƻ�ȡ���ݿ��ָ�롣���ʡ�����ݿ����򷵻ص�ǰ����ݿ⡣��������ݿⲻ�������׳��쳣
	DataBase* Session::getDatabase(const std::string& dbName)
	{
		std::string dbn = dbName == "" ? selected : dbName;
		if (db.count(dbn)) return db[dbn];
		else throw "Database does not exist";
	}

	// �������ƻ�ȡ���ָ�롣���ʡ�����ݿ������ڵ�ǰ����ݿ��в��ҡ�����ñ��������׳��쳣
	Table* Session::getTable(const std::string& tableName, const std::string& dbName)
	{
		DataBase* db = getDatabase(dbName);
		if (db->TableList.count(tableName)) return db->TableList[tableName];
		else throw "Table does not exist";
	}

	void Session::start()
	{
		std::string sql;
		fstream file;
		file.open("data.sql", ios::in);
		while (getline(file, sql, ';'))
		{
			sql = trimString(sql);
			if (sql == "") break;
			execute(sql, std::cout);
		}
		file.close();
		
		file.open("data.sql", ios::out | ios::app);
		while (getline(std::cin, sql, ';'))
		{
			sql = trimString(sql);
			if (sql == "") break;
			execute(sql, std::cout);
			file << sql << std::endl;
		}
		file.close();
		return;
	}

	void Session::execute(std::string sql, std::ostream& o)
	{
		auto str = tokenize(sql), strLower = str; // ���� tokenize �����ִ�
		std::transform(strLower.begin(), strLower.end(), strLower.begin(), ::stringToLower); // ȫ��Сд

		if (str.size() < 2) throw "Not a complete statement"; // �����������
		else if (strLower[0] == "create")
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
		else if (strLower[0] == "drop" && strLower[1] == "database")
			dropDatabase(str[2]);
		else if (strLower[0] == "drop" && strLower[1] == "table")
			dropTable(str[2]);
		else if (strLower[0] == "use" && strLower[1] != "tables")
			use(str[1]);
		else if (strLower[0] == "show" && strLower[1] == "databases")
			opt.DBShow(db, o);
		else if (strLower[0] == "show" && strLower[1] == "tables")
			opt.TableShow(*getDatabase(), selected, o);
		else if (strLower[0] == "show" && strLower[1] == "columns")
			opt.TableShowInfo(*getDatabase()->TableList[str[3]], o);
		else if (strLower[0] == "insert" && strLower[1] == "into")
		{
			auto t = std::find(strLower.begin(), strLower.end(), "values") - strLower.begin();
			insert(
				str[2],
				tokens(str.begin() + 4, str.begin() + t - 1),
				tokens(str.begin() + t + 2, str.end() - 1)
			);
		}
		else if (strLower[0] == "load")
		{
			auto p = std::find(strLower.begin(), strLower.end(), "infile") - strLower.begin();
			if (p == strLower.size()) throw "Input file not specified";

			std::string inputFileName = parseStringLiteral(str[p + 1]);
			auto t = std::find(strLower.begin(), strLower.end(), ")") - strLower.begin();
			auto& tableName = str[p + 4];
			auto attrName = tokens(str.begin() + p + 6, str.begin() + t);
			std::ifstream i(inputFileName);
			if (!i.is_open())
				throw "Cannot open input file";
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
		else if (strLower[0] == "delete" && strLower[1] == "from")
		{
			auto t = find(strLower.begin(), strLower.end(), "where") - strLower.begin();
			if (t == (int)strLower.size())
				remove(str[2]);
			else
				remove(str[2], tokens(str.begin() + t + 1, str.end()));
		}
		else if (strLower[0] == "update")
		{
			auto t = std::find(strLower.begin(), strLower.end(), "where") - strLower.begin();
			auto setClause = tokens(str.begin() + 3, str.begin() + t);
			if (t == (int)strLower.size())
				update(str[1], setClause);
			else
				update(str[1], setClause, tokens(str.begin() + t + 1, str.end()));
		}
		else if (strLower[0] == "select")
		{
			std::vector<interval> clauses = selectClauses(strLower);

			enum clause_t { SELECT = 0, INTO, FROM, WHERE, GROUP, ORDER };

			// ��ȡ����ļ� start
			bool import = false;

			std::ostream* os = nullptr;
			if (length(clauses[INTO]) == 0)
				os = &o;
			else
			{
				import = true;
				std::string outputFileName = parseStringLiteral(str[clauses[INTO].first + 2]);
				std::ofstream* of = new std::ofstream(outputFileName, std::ios::out);
				if (!of->is_open())
					throw "Cannot open output file. Does the output file already exist?";
				os = of;
			}
			// ��ȡ����ļ� end

			// ��ȡ���������� start
			auto& tableName = str[clauses[FROM].first + 1];

			Table& table = *getTable(tableName);

			auto attrName = tokens(str.begin() + 1, str.begin() + clauses[FROM].first);
			if (str[1] == "*")
				attrName = table.columnNames;
			for (auto iter = attrName.begin(); iter != attrName.end(); ++iter)
				if (*iter == hiddenPrimaryKey)
				{
					attrName.erase(iter);
					break;
				}
			// ��ȡ���������� end

			// ��ȡ WHERE �Ӿ� start
			std::string whereStr = "";
			if (length(clauses[WHERE]) != 0)
				whereStr = concatenate(tokens(str.begin() + clauses[WHERE].first + 1, str.begin() + clauses[WHERE].second));

			OOPD::WhereAttr where = SubWhere(table, whereStr);
			// ��ȡ WHERE �Ӿ� end

			// ��ȡ ORDER BY �Ӿ� start
			auto ob = std::find(strLower.begin(), strLower.end(), "order") - strLower.begin();
			orders orderClause;
			if (length(clauses[ORDER]) != 0)
			{
				for (unsigned i = clauses[ORDER].first + 2; i < clauses[ORDER].second; i += 2)
				{
					OOPD::sort_t order = OOPD::sort_t::ascending;
					if (i + 1 < clauses[ORDER].second && strLower[i + 1] == "desc")
						order = OOPD::sort_t::descending;
					orderClause.push_back((order_t){str[i], order});
					if (i + 1 < clauses[ORDER].second && strLower[i + 1] != ",")
						++i;
				}
			}
			else
				orderClause.push_back((order_t){table.PrimaryCol, OOPD::sort_t::ascending});
			// ��ȡ ORDER BY �Ӿ� end

			// ִ�в�ѯ����
			//opt.DataShow(table, attrName, where, !import, *os);
			opt.select(table, where, attrName, os == &o, groups(), orderClause, *os);
			// ��������ļ���ɾ���ļ���
			if (os != &o) delete os;
		}
		else throw "Unknown SQL command";
	}

	void Session::createDatabase(std::string dbName)
	{
		if (!opt.DBCreate(db, dbName))
			throw "Database '" + dbName + "' already exists";
	}

	void Session::createTable(std::string tableName, const tokens& traits)
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
		if (!opt.TableCreate(*getDatabase(), tableName, fields))
			throw "Table '" + tableName + "' already exists";
	}

	void Session::dropDatabase(std::string dbName)
	{
		if (dbName == selected) selected = "";
		if (!opt.DBDelete(db, dbName))
			throw "Database '" + dbName + "' does not exist";
	}

	void Session::dropTable(std::string tableName)
	{
		if (!opt.TableDelete(*getDatabase(), tableName))
			throw "Table '" + tableName + "' does not exist";
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
		return opt.DataDelete(targetTable, temp);//����SubWhere���Where������������DataDelete����
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
		//�����ж��Ƿ�����ַ�������������ú�����Լ������������δ����WHERE�Ӿ䣬��ֱ�Ӵ�����ַ�����
		if (wholeStr.size() == 0)
			return WhereAttr(all, std::vector<WhereAttrSub1>(), std::vector<WhereAttr::node>(), std::vector<WhereAttrSub2>());

		//����һ��WhereAttr���󣬲���traversalģʽ����ֻ���ڸ���ϸ�ֵش洢��ǰ���ʽ
		WhereAttr traversalWhere(traversal, std::vector<WhereAttrSub1>(), std::vector<WhereAttr::node>(), std::vector<WhereAttrSub2>());
		auto end = wholeStr.end();
		int pos = 0;//��¼��ǰ�±�λ��
		for (auto it = wholeStr.begin(); it != end; ++it)//�����������ʽ
		{
			int endOfNode = -1;//��¼��ǰ���Ľ���λ�ã������Ǽ��赱ǰ��㲻���������������������������endOfNode == pos
			//������ļ������У��᲻�ϵ���endOfNode��ֵ����δ��⵽��Ӧ�ַ�(��)����endOfNode�ᱣ��ԭֵ
			//����⵽��Ӧ�ַ�(��)������λ�ñ�endOfNode��¼��λ�ø���ǰ����endOfNode����Ϊ��λ��
			//������ĳ�μ���endOfNode == pos����˵����ǰ�����Ǳ������ַ�����˽�����Ϣѹ������ṹ��������һ�������ж�
			//���ȫ����������δ������һ�������ж�����˵����ǰ��㲻������������Ǹ���endOfNode�м�¼��λ����Ϣ�õ���ǰ���ĳ��ȣ����Ըý��������з���
			if (*it == ' ')//����ǰָ��ո������һλ�����¿�ʼ�ж�����
			{
				pos++;
				continue;
			}
			//���ȼ��Ƚ������
			if (SubMin(endOfNode, wholeStr.find(">=", pos)) == pos && pos != -1)//����ǰ���Ϊ>=
			{
				traversalWhere.traversalAttr.push_back(WhereAttrSub2(compare, oprOr, greaterEqual, typeInt, std::string()));//ѹ����
				pos += 2;//�ƶ�pos����>=��������һ�������ж�
				++it;//ͬ���ƶ�it������forѭ���Դ�һ��λ�ƣ����ֻ++
				continue;
			}
			if (SubMin(endOfNode, wholeStr.find("<=", pos)) == pos && pos != -1)//����ǰ���Ϊ<=
			{
				traversalWhere.traversalAttr.push_back(WhereAttrSub2(compare, oprOr, lessEqual, typeInt, std::string()));//ѹ����
				pos += 2;
				++it;
				continue;
			}
			if (SubMin(endOfNode, wholeStr.find_first_of('=', pos)) == pos && pos != -1)//����ǰ���Ϊ=
			{
				traversalWhere.traversalAttr.push_back(WhereAttrSub2(compare, oprOr, equal, typeInt, std::string()));//ѹ����
				pos++;
				continue;
			}
			if (SubMin(endOfNode, wholeStr.find("!=", pos)) == pos && pos != -1)//����ǰ���Ϊ!=
			{
				traversalWhere.traversalAttr.push_back(WhereAttrSub2(compare, oprOr, notEqual, typeInt, std::string()));//ѹ����
				pos += 2;
				++it;
				continue;
			}
			if (SubMin(endOfNode, wholeStr.find_first_of('>', pos)) == pos && pos != -1)//֮ǰ�Ѿ��ų�>=������ǰ���Ϊ>
			{
				traversalWhere.traversalAttr.push_back(WhereAttrSub2(compare, oprOr, greater, typeInt, std::string()));//ѹ����
				pos++;
				continue;
			}
			if (SubMin(endOfNode, wholeStr.find_first_of('<', pos)) == pos && pos != -1)//֮ǰ�Ѿ��ų�<=������ǰ���Ϊ<
			{
				traversalWhere.traversalAttr.push_back(WhereAttrSub2(compare, oprOr, less, typeInt, std::string()));//ѹ����
				pos++;
				continue;
			}
			//��ִ�е���λ�ã���˵����ǰ��㲻�ǱȽ�����������ǽ����߼���������ж�
			//���ڿ����漰��Сд�ж����⣬�����ȹ���һ���µ��ַ�������ȫ����ĸ��Ϊ��д
			std::string wholeStrUpper(stringToUpper(wholeStr));
			int andPos1 = wholeStrUpper.find("AND ", pos);
			int andPos2 = wholeStrUpper.find(" AND ", pos);
			if (andPos1 == pos && pos != -1)//����ǰ���ΪAND
			{
				traversalWhere.traversalAttr.push_back(WhereAttrSub2(logic, oprAnd, greater, typeInt, std::string()));//ѹ����
				pos += 4;
				it += 3;
				continue;
			}
			SubMin(endOfNode, andPos2);
			int orPos1 = wholeStrUpper.find("OR ", pos);
			int orPos2 = wholeStrUpper.find(" OR ", pos);
			if (orPos1 == pos && pos != -1)//����ǰ���ΪOR
			{
				traversalWhere.traversalAttr.push_back(WhereAttrSub2(logic, oprOr, greater, typeInt, std::string()));//ѹ����
				pos += 3;
				it += 2;
				continue;
			}
			SubMin(endOfNode, orPos2);
			int notPos1 = wholeStrUpper.find("NOT ", pos);
			int notPos2 = wholeStrUpper.find(" NOT ", pos);
			//������һ��������������к���NOT������ȷ����notPos1 != pos��������������Ⱦ�˵����ǰ���ΪNOT
			//����ǰ��㲻ΪNOT����NOTǰ��һ���пո񣬾Ϳ����õڶ���notPos2�ƶ�endOfNodeλ��
			if (notPos1 == pos && pos != -1)//����ǰ���ΪNOT
			{
				traversalWhere.traversalAttr.push_back(WhereAttrSub2(logic, oprNot, greater, typeInt, std::string()));//ѹ����
				pos += 4;
				it += 3;
				continue;
			}
			SubMin(endOfNode, notPos2);
			//��ִ�е���ǰλ�ã���˵����ǰ��㲻�����������Ҫ�ж������������ǳ���
			//������ȡ��ǰ������Ϣ
			int strLen = (endOfNode == -1 ? -1 : endOfNode - pos);//����һ���µ��ַ�������endOfNode == -1����Ӧֱ��ȡ��ȫ�����ʽ�Ľ�β������ȡ��
			//endOfNode - pos����
			std::string thisNode(wholeStr.substr(pos, strLen));
			while (*thisNode.rbegin() == ' ')//ȥ�����ܵ�β���ո�
				thisNode.erase(--thisNode.end());
			if (thisNode[0] == '"')//�������ſ�ʼ�������ַ�������
			{
				traversalWhere.traversalAttr.push_back(WhereAttrSub2(valConst, oprOr, greater, typeChar, std::string(thisNode.substr(1, thisNode.size() - 2))));//����-1ȥ��������
				pos += strLen;
				it += (strLen - 1);
				if (strLen == -1) break;
				continue;
			}
			if ((thisNode[0] >= '0' && thisNode[0] <= '9') || thisNode[0] == '-' || thisNode[0] == '.')//�������ֻ򸺺Ż�С���㿪ʼ���������ֳ�������Ҫ���ж���int����double
			{
				double valDouble = std::stod(thisNode);
				int valInt = int(valDouble);
				if (valInt == valDouble)//��û���г�������Ϊ��int
					traversalWhere.traversalAttr.push_back(WhereAttrSub2(valConst, oprOr, greater, typeInt, thisNode));
				else//��Ϊ��double
					traversalWhere.traversalAttr.push_back(WhereAttrSub2(valConst, oprOr, greater, typeDouble, thisNode));
				pos += strLen;
				it += (strLen - 1);
				if (strLen == -1) break;
				continue;
			}
			//��ִ�е��˴�����˵����ǰ���ֻ��������
			DataType colType = target.DataAddress[thisNode].type;;//�ж����е���������
			traversalWhere.traversalAttr.push_back(WhereAttrSub2(valCol, oprOr, greater, colType, thisNode));
			pos += strLen;
			it += (strLen - 1);
			if (strLen == -1) break; //©дһ�仰���˷�һ����
		}
		//traversalWhere�Ĵ����Ѿ���ɣ�������Ҫ�ж��Ƿ����ʹ��searchģʽ����������ȫ������ֻ�漰�������ұȽ�����ֻ�漰���볣���ıȽ�
		//����traversalWhere�е�traversalAttr�����Խ����ж�
		auto attrEnd = traversalWhere.traversalAttr.end();
		for (auto attrIt = traversalWhere.traversalAttr.begin(); attrIt != attrEnd; ++attrIt)
		{
			if (attrIt->type == valCol)//���������
				if (attrIt->valOrName != target.PrimaryCol)//��������������Ƿ�һ��
					return traversalWhere;//����һ����������
			if (attrIt->type == compare)//����ǱȽ������
			{
				auto leftVal = attrIt;
				--leftVal;
				auto rightVal = attrIt;
				++rightVal;
				if (leftVal->type + rightVal->type != valConst + valCol)//�������Ĳ������Ƿ�һ���ǳ���һ��������
					return traversalWhere;//����һ����������
			}
		}
		//��ִ�е��˴�����˵������ʹ��searchģʽ����Ҫ���¹�������ṹ
		WhereAttr searchWhere(search, std::vector<WhereAttrSub1>(), std::vector<WhereAttr::node>(), std::vector<WhereAttrSub2>());
		int comparePos = 0;//���ڼ�¼��ʾ�Ƚ�����Ľ���Ѿ�ѹ�뵽�ĸ��±괦
		for (auto attrIt = traversalWhere.traversalAttr.begin(); attrIt != attrEnd; ++attrIt)//����traversalWhere
		{
			if (attrIt->type == logic)//������߼����������ֱ��ѹ��
				searchWhere.searchAttr.push_back(WhereAttr::node(logic, attrIt->logicType, 0));
			if (attrIt->type == compare)//����ǱȽ����������Ҫ��ȡ�����Ƚ��������Ϣ��ѹ��
			{
				auto constVal = attrIt;//ָ��Ƚ������еĳ���
				++constVal;
				auto colVal = attrIt;//ָ��Ƚ������е�����
				--colVal;//Ĭ������λ����������
				if (colVal->type == valConst)//������λ��������Ҳ�
				{
					constVal += 2;
					colVal -= 2;
					//����Ƚ������������ķ���
					switch (attrIt->compareType)
					{
						case greater: attrIt->compareType = less; break;
						case less: attrIt->compareType = greater; break;
						case greaterEqual: attrIt->compareType = lessEqual; break;
						case lessEqual: attrIt->compareType = greaterEqual; break;
						default: break;
					}
				}
				//ѹ����Ϣ�����Ƚ��Ƚ��������Ϣѹ��searchCompare���飬Ȼ��ָ��˴�����Ϣѹ��searchAttr����
				searchWhere.searchCompare.push_back(WhereAttrSub1(colVal->valOrName, attrIt->compareType, target.DataAddress[colVal->valOrName].type, constVal->valOrName));
				searchWhere.searchAttr.push_back(WhereAttr::node(clause, oprAnd, comparePos));
				++comparePos;//�±��һ
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
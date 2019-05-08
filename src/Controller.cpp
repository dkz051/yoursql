#include "Controller.h"

namespace OOPD
{
    int Controller::CutString(std::string& str)
    {
        for (int i = 0;i < str.size(); ++i)
            if (str[i] == ' ' || str[i] == '(') return i;
        return str.size();
    }

    std::string & Controller::AdjustString(std::string &str)
    {
        for (int i = 0; i < str.size(); ++i)
            if (str[i] >= 'a' && str[i] <= 'z') str[i] -= 32;
        return str;
    }

    std::string & Controller::TrimString(std::string &str)
    {
        while (str[0] == ' ' || str[0] == '\n')
            str.erase(str.begin());
        return str;
    }

    std::string & Controller::TrimChar(std::string &str)
    {
        if (str[0] == '\'' || str[0] == '"') str.erase(str.begin());
        if (str.back() == '\'' || str.back() == '"') str.pop_back();
        return str;
    }

    TableCreateAttr Controller::TableOp(std::string& temp)
    {
        int pos=0;
        TableCreateAttr action;
        std::vector<std::string>saver;//储存分离的字符串便于分析
        char CutSignal=' ';
        while (temp[0] == ' ') temp.erase(temp.begin());
        pos =temp.find(CutSignal);
        while (pos != std::string::npos)//以逗号为间隔提取每一个参数组
        {
            saver.push_back(temp.substr(0,pos));
            temp=temp.substr(pos+1,temp.size());
            pos =temp.find(CutSignal);
        }
        saver.push_back(temp);
        action.colName=saver[0];
        AdjustString(saver[1]);
        if(saver[1]=="INT") action.type=typeInt;
        else if(saver[1]=="DOUBLE") action.type=typeDouble;
        else if(saver[1]=="CHAR") action.type=typeChar;
        if(saver.size()==4) action.NotNull=true;
        return action;
    }

    void Controller::start()
    {
        while(Readin());
        return;
    }

    bool Controller::Readin()
    {
        std::string str1, str2, str3;
        if (!getline(std::cin,str1,';')) return false;
        TrimString(str1); //过滤换行符
        if (!(str1[0] >= 'A' && str1[0] <= 'Z' || str1[0] >= 'a' && str1[0] <= 'z')) return true;
        if (CutString(str1) != std::string::npos)
        {
            str2 = str1.substr(0, CutString(str1));
            str3 = str1.substr(CutString(str1) + 1, str1.size());
        }
        else str2 = str1;
        AdjustString(str2);
        if (str2 == "CREATE") CREATE(str3);
        else if (str2 == "DROP") DROP(str3);
        else if (str2 == "USE") USE(str3);
        else if(str2 == "SHOW") SHOW(str3);
        else if (str2 == "INSERT") INSERT(str3);
        else if (str2 == "UPDATE") UPDATE(str3);
        else if (str2 == "DELETE") DELETE(str3);
        else if (str2 == "SELECT") SELECT(str3);
        return true;
    }

    bool Controller::CREATE(std::string& str3)
    {
        std::string str4, str5;
        if (CutString(str3) != std::string::npos)
        {
            str4 = str3.substr(0, CutString(str3));
            str5 = str3.substr(CutString(str3) + 1, str3.size());
        }
        else str4 = str3;
        AdjustString(str4);
        if(str4=="DATABASE")
        {
            std::string name=str5;
            if(!opt.DBCreate(target,name)) return false;
            if (target.size() == 2)
                activeDB = target.find(name)->second;
        }
        else if(str4=="TABLE")
        {
            std::string name(str5.begin(), str5.begin() + CutString(str5));
            std::string term(str5.begin() + CutString(str5) + 1, str5.end() - 1); //过滤右括号
            std::vector<TableCreateAttr> attr;
            int pos = 0;
            char CutSignal = ',';
            term += CutSignal;
            pos =  term.find(CutSignal);
            while (pos != std::string::npos)//以逗号为间隔提取每一个参数组
            {
                std::string temp = term.substr(0,pos), testofkey;
                if (temp.size() >= 7) testofkey = temp.substr(0, 7); //判断是否是设置主键的操作
                if (testofkey == "PRIMARY")
                {
                    std::string key_word;
                    int key_pos =  temp.find('(') + 1;
                    while (temp[key_pos] != ')')
                        key_word.push_back(temp[key_pos++]);
                    for (auto it = attr.begin(); it != attr.end(); ++it)
                        if (it->colName == key_word) it->Primary = true;
                }
                else attr.push_back(TableOp(temp));
                term = term.substr(pos + 1, term.size());
                TrimString(term);
                pos = term.find(CutSignal);
            }
            return opt.TableCreate(*activeDB, name, attr); //std::cout<<"FAILED TO CREATE TABLE "<<std::endl;
        }
        return true;
    }

    bool Controller::DROP(std::string& str3)
    {
        std::string str4, str5;
        if (CutString(str3) != std::string::npos)
        {
            str4 = str3.substr(0, CutString(str3));
            str5 = str3.substr(CutString(str3) + 1, str3.size());
        }
        else str4 = str3;
        if(str4=="DATABASE")
        {
            if (target.find(str5)->second == activeDB) activeDB = nullptr;
            return opt.DBDelete(target,str5);
        }
        else if(str4=="TABLE") return opt.TableDelete(*activeDB, str5);
        return false;
    }

    bool Controller::INSERT(std::string& str3)
    {
        std::vector<DataUpdateAttr>attr;
        //提取表名
        int pos_first = str3.find(' ');
        int pos_L1 = str3.find('(');
        int pos_R1 = str3.find(')');
        std::string tablename(str3.begin() + pos_first, str3.begin() + pos_L1);
        TrimString(tablename);
        std::string val_string(str3.begin()+ pos_R1 + 1,str3.end());
        int pos_L2 = val_string.find('(');
        int pos_R2 = val_string.find(')');
        std::string colname = str3.substr(pos_L1 + 1, pos_R1 - pos_L1 - 1);
        val_string = val_string.substr(pos_L2 + 1, pos_R2 - pos_L2 - 1);
        char CutSignal=',';
        colname += CutSignal;
        val_string += CutSignal;
        int pos1 = colname.find(CutSignal);
        int pos2 = val_string.find(CutSignal);
        while (pos1!= std::string::npos && pos2!=std::string::npos)//以逗号为间隔提取每一个参数组
        {
            std::string temp1, temp2;
            temp1 = colname.substr(0, pos1);
            temp2 = val_string.substr(0, pos2);
            colname = colname.substr(pos1 + 1, colname.size());
            val_string = val_string.substr(pos2 + 1, val_string.size());
            TrimString(colname);
            TrimString(val_string);
            TrimString(temp1);
            TrimString(temp2);
            TrimChar(temp2);
            pos1 = colname.find(CutSignal);
            pos2 = val_string.find(CutSignal);
            attr.push_back(DataUpdateAttr(temp1, temp2));
        }
        return opt.DataInsert(*activeDB->TableList.find(tablename)->second, attr);
    }

    bool Controller::SHOW(std::string& temp)
    {
        std::vector<std::string>saver; //储存分离的字符串便于分析
        char CutSignal=' ';
        int pos =temp.find(CutSignal);
        while (pos != std::string::npos)
        {
            saver.push_back(temp.substr(0,pos));
            temp=temp.substr(pos+1,temp.size());
            pos =temp.find(CutSignal);
        }
        saver.push_back(temp);
        AdjustString(saver[0]);
        if(saver[0] =="DATABASES") opt.DBShow(target);
        else if(saver[0] =="TABLES")
        {
            std::string name;
            for (auto it = target.begin(); it != target.end(); ++it)
                if (it->second == activeDB)
                {
                    name = it->first;
                    break;
                }
            opt.TableShow(*activeDB, name);
        }
        else if(saver[0] =="COLUMNS")
            opt.TableShowInfo(*activeDB->TableList.find(saver.back())->second);
        return true;
    }

    bool Controller::USE(std::string& str3)//center为定义的controller对象
    {
        auto it = target.find(str3);
        if (it != target.end())
        {
            activeDB = it->second;
            return true;
        }
        return false;
    }

    bool Controller::DELETE(std::string& str3)
    {
        //去除空格
        TrimString(str3);
        //提取数据表名称
        int nameBegin = str3.find_first_of(' ') + 1;//nameBegin指向FROM后的第一个非空格字符，该字符应属于数据表名称的一部分
        while (str3[nameBegin] == ' ')
            nameBegin++;
        int nameEnd = str3.find_first_of(' ', nameBegin);//nameEnd指向数据表名称后的空格，即数据表名称与where之间的空格
        std::string tableName(str3.substr(nameBegin, nameEnd - nameBegin));//提取数据表名称
        Table& targetTable = *(activeDB->TableList[tableName]);//得到目标数据表的引用
        //提取Where条件
        while (str3[nameEnd] == ' ')
            nameEnd++;
        int whereBegin = str3.find_first_of(' ', nameEnd) + 1;//使whereBegin指向Where表达式(WHERE字段后方的部分)的开头
        if (whereBegin == 0)
        {
            std::string whereStr = "";
            auto temp = SubWhere(targetTable, whereStr);
            return opt.DataDelete(targetTable, temp);
        }
        while (str3[whereBegin] == ' ')
            whereBegin++;
        std::string whereStr(str3.substr(whereBegin));
        auto temp = SubWhere(targetTable, whereStr);
        return opt.DataDelete(targetTable, temp);//调用SubWhere获得Where参数，并调用DataDelete方法
    }

    bool Controller::SELECT(std::string& str3)
    {
        bool judge = false; //判断是否有WhereClause
        TrimString(str3); //去除空格
        //提取数据表信息
        std::vector<std::string> saver; //储存分离的字符串便于分析
        std::vector<std::string> colname; //储存需要显示的列名
        char CutSignal=' ';
        int pos =str3.find(CutSignal);
        while (pos != std::string::npos)
        {
            saver.push_back(str3.substr(0,pos));
            str3 = str3.substr(pos+1,str3.size());
            pos = str3.find(CutSignal);
            std::string temp(saver.back());
            if (AdjustString(temp) == "WHERE")
            {
                judge = true;
                break;
            }
        }
        saver.push_back(TrimString(str3));
        for (auto it = saver.begin(); it != saver.end(); ++it)
        {
            std::string temp(*it);
            if (AdjustString(temp) != "FROM") continue;
            pos = it -saver.begin(); //记录FROM关键字的位置
            break;
        }
        std::string tableName = saver[pos + 1];//获得数据表名称
        Table& targetTable = *(activeDB->TableList[tableName]);//得到目标数据表的引用
        //提取选择信息
        for (auto it = saver.begin(); it != saver.end(); ++it)
        {
            if (it - saver.begin() == pos) break;
            if (*it == "*")
            {
                for (auto iter = targetTable.DataAddress.begin(); iter != targetTable.DataAddress.end(); ++iter)
                    colname.push_back(iter->first);
                break;
            }
            colname.push_back(*it);
        }
        //调用方法
        if (!judge) str3 = "";
        auto temp = SubWhere(targetTable, str3);
        opt.DataShow(targetTable, colname, temp);
    }

    bool Controller::UPDATE(std::string& str3)
    {
        //去除空格
        TrimString(str3);
        //提取数据表信息
        int nameEnd = str3.find_first_of(' ');//nameEnd指向数据表名称后的空格
        std::string tableName(str3.substr(0, nameEnd));//获得数据表名称
        Table& targetTable = *(activeDB->TableList[tableName]);//得到目标数据表的引用
        //提取更新信息
        while (str3[nameEnd] == ' ')//nameEnd指向SET的首字母
            nameEnd++;
        int colBegin = str3.find_first_of(' ', nameEnd);//colBegin指向列名的首字符
        while (str3[colBegin] == ' ')
            colBegin++;
        int equalPos = str3.find_first_of('=', colBegin);//equalPos指向列名与常量之间的等号
        int valBegin = equalPos + 1;//valBegin指向常量的首字符
        while (str3[valBegin] == ' ')
            valBegin++;
        int valEnd = str3.find_first_of(' ', valBegin);//valEnd指向常量后的空格
        //构造传参结构
        std::vector<DataUpdateAttr> updateInfo;//构造空数组
        int colEnd = equalPos - 1;//colEnd指向列名的末字符后一位
        while (str3[colEnd] == ' ')
            colEnd--;
        colEnd++;
        std::string temp = str3.substr(valBegin, valEnd - valBegin);
        updateInfo.push_back(DataUpdateAttr(str3.substr(colBegin, colEnd - colBegin), TrimChar(temp)));//压入信息
        //提取Where条件
        while (str3[valEnd] == ' ')
            valEnd++;
        int whereBegin = str3.find_first_of(' ', valEnd) + 1;//使whereBegin指向Where表达式(WHERE字段后方的部分)的开头
        if (whereBegin == 0)
        {
            std::string whereStr = "";
            auto temp = SubWhere(targetTable, whereStr);
        }
        while (str3[whereBegin] == ' ')
            whereBegin++;
        std::string whereStr(str3.substr(whereBegin));
        //调用方法
        auto tmp = SubWhere(targetTable, whereStr);
        return opt.DataUpdate(targetTable, tmp, updateInfo);
    }

    WhereAttr Controller::SubWhere(Table& target, std::string& wholeStr)
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
            std::string wholeStrUpper(wholeStr);
            AdjustString(wholeStrUpper);
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

    int& Controller::SubMin(int& endOfNode, const int& num)
    {
        if (endOfNode == -1 || (endOfNode > num && num != -1))
            endOfNode = num;
        return endOfNode;
    }
}

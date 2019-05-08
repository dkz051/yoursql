#include "Table.h"

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

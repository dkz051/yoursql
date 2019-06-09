#ifndef BPTREE_H
#define BPTREE_H

#include "BPTreeNode.h"
#include <iostream>

namespace OOPD
{
	template <typename T>
	class BPTree
	{
	private:
		int Rank; // B+树的阶数
		Node<T>* RootPointer; // B+树的根指针
	public:
		BPTree(int rank);
		~BPTree();
		int GetRank() {return Rank;} // 获得B+树的阶数
		Node<T>* GetRoot() {return RootPointer;} // 获得B+树的根指针
		void Print(Node<T>* pos, int depth); //打印整个B+树
		bool Insert(T & key, Data* data); //添加索引为key的数据，若索引已经存在则返回false
		bool Delete(T & key); //删除索引为key的数据，若索引不存在则返回false
		bool Update(T & key, T & new_key); //以new_key更新索引为key的数据，若失败则返回false
		Node<T>* GetFirstNode(); //返回B+树索引值最小的叶结点
		Node<T>* GetTargetNode(T & key); //查找索引为key的数据所在叶结点
		void DeleteNode(Node<T>* pos); //删除pos及pos的所有子节点
		Data* Search(T & key); //查找索引为key的数据，若没有找到则返回空指针（一般用于key唯一的情况）
		std::vector<Data*> BatchSearch(T key_lower, T key_upper, bool lower_close, bool upper_close); //批量查找key的值处于某一范围内的数据，若没有找到则返回空vector
		std::vector<Data*> BatchSearch(T key, bool close, bool greater_than); //批量查找大于或者小于key的数据，若没有找到则返回空vector
		std::vector<Data*> GetData(); //返回B+树上所有数据
	};

	template<typename T>
	BPTree<T>::BPTree(int rank) : Rank(rank) //构造一棵新B+树
	{
		Node<T>* root = new Node<T>(nullptr, LEAF);
		RootPointer = root;
	}

	template<typename T> //析构
	BPTree<T>::~BPTree()
	{
		DeleteNode(RootPointer);
	}

	template<typename T>
	void BPTree<T>::Print(Node<T>* pos, int depth)
	{
		if (RootPointer == nullptr)
		{
			std::cout << "The Tree Is Empty !" << std::endl;
			return;
		}
		if (pos->GetType() != LEAF)
			for (auto it = pos->ChildPointer.begin(); it != pos->ChildPointer.end(); ++it)
				Print(*it, depth + 1);
		std::cout << "Depth  " << depth;
		if (pos->ParentPointer != nullptr) std::cout << "   Parent  " << pos->ParentPointer->KeyValue[0] << std::endl;
		else std::cout << "  NULL" << std::endl;
		for (auto it = pos->KeyValue.begin(); it != pos->KeyValue.end(); ++it)
		{
			std::cout << "    KeyNum   " << it - pos->KeyValue.begin() << "   Key   " << *it;
			if (!pos->DataPointer.empty()) std::cout << "   Data   " << pos->DataPointer[it - pos->KeyValue.begin()]->VAL << std::endl;
			else std::cout << std::endl;
		}
		return;
	}

	template<typename T>
	bool BPTree<T>::Insert(T & key, Data* data)
	{
		bool flag = false;
		Node<T>* NowNode = RootPointer;
		while (NowNode->GetType() != LEAF) //找到待插入数据的叶结点
		{
			flag = false;
			for (auto it = NowNode->KeyValue.begin(); it != NowNode->KeyValue.end(); ++it)
			{
				if (key >= *it) continue;
				NowNode = NowNode->ChildPointer[it - NowNode->KeyValue.begin()];
				flag = true;
				break;
			}
			if (!flag) NowNode = NowNode->ChildPointer.back();
		}
		flag = false;
		for (auto it = NowNode->KeyValue.begin(); it != NowNode->KeyValue.end(); ++it) //找到叶结点上待插入数据的位置并插入数据
		{
			if (key == *it) return false;
			if (key > *it) continue;
			NowNode->DataPointer.insert(it - NowNode->KeyValue.begin() + NowNode->DataPointer.begin(), data);
			NowNode->KeyValue.insert(it, key);
			flag = true;
			break;
		}
		if (!flag)
		{
			NowNode->DataPointer.push_back(data);
			NowNode->KeyValue.push_back(key);
		}
		while (NowNode->KeyValue.size() >= Rank) //进行结点的分裂（如果有必要），并更新根指针
		{
			NowNode = NowNode->Split(Rank / 2);
			RootPointer = NowNode;
		}
		while (RootPointer->ParentPointer != nullptr)
			RootPointer = RootPointer->ParentPointer;
		return true;
	}

	template<typename T>
	bool BPTree<T>::Delete(T & key)
	{
		bool flag = false;
		Node<T>* NowNode = RootPointer;
		while (NowNode->GetType() != LEAF) //找到待删除数据的叶结点
		{
			flag = false;
			for (auto it = NowNode->KeyValue.begin(); it != NowNode->KeyValue.end(); ++it)
			{
				if (key >= *it) continue;
				NowNode = NowNode->ChildPointer[it - NowNode->KeyValue.begin()];
				flag = true;
				break;
			}
			if (!flag) NowNode = NowNode->ChildPointer.back();
		}
		flag = false;
		for (auto it = NowNode->KeyValue.begin(); it != NowNode->KeyValue.end(); ++it) //找到叶结点上待删除数据的位置并删除数据
		{
			if (key != *it) continue;
			flag = true;
			NowNode->DataPointer.erase(it - NowNode->KeyValue.begin() + NowNode->DataPointer.begin());
			NowNode->KeyValue.erase(it);
			break;
		}
		if (!flag) return false;
		if (NowNode->ParentPointer != nullptr && NowNode->GetType() == LEAF) //更新叶结点父结点的key值
			for (auto it = NowNode->ParentPointer->ChildPointer.begin(); it != NowNode->ParentPointer->ChildPointer.end(); ++it)
			{
				if (*it != NowNode) continue;
				if (it == NowNode->ParentPointer->ChildPointer.begin()) break;
				if (!NowNode->KeyValue.empty())
				   NowNode->ParentPointer->KeyValue[it - NowNode->ParentPointer->ChildPointer.begin() - 1] = (*it)->KeyValue[0];
				else if (it + 1 != NowNode->ParentPointer->ChildPointer.end())
				   NowNode->ParentPointer->KeyValue[it - NowNode->ParentPointer->ChildPointer.begin() - 1] = (*(it + 1))->KeyValue[0];
				break;
			}
		while (NowNode->GetType() != OOPD::LEAF && // 我还是克制一下不要说脏话好了，但是在这上浪费我们一下午时间真的好吗
			NowNode->KeyValue.size() < Rank / 2) //进行结点的旋转、合并（如果有需要），并更新根指针
		{
			if (NowNode == RootPointer)
			{
				if (!NowNode->KeyValue.empty()) break;
				if (NowNode->ChildPointer.empty())
				{
					RootPointer = nullptr;
					delete NowNode;
					return true;
				}
				NowNode->ChildPointer[0]->ParentPointer = nullptr;
				RootPointer = NowNode->ChildPointer[0];
				delete NowNode;
				break;
			}
			Node<T>* left_target = nullptr;
			Node<T>* right_target = nullptr;
			Node<T>* parent = NowNode->ParentPointer;
			for (auto it = parent->ChildPointer.begin(); it != parent->ChildPointer.end(); ++it)
			{
				if (*it != NowNode) continue;
				if (it != parent->ChildPointer.begin()) left_target = *(it - 1);
				if ((it + 1) != parent->ChildPointer.end()) right_target = *(it + 1);
				break;
			}
			if (left_target != nullptr && left_target->KeyValue.size() >= Rank / 2 + 1)
			{
				NowNode->Spin(left_target, RIGHT);
				break;
			}
			else if (right_target != nullptr && right_target->KeyValue.size() >= Rank / 2 + 1)
			{
				NowNode->Spin(right_target, LEFT);
				break;
			}
			if (left_target != nullptr) NowNode = NowNode->Merge(left_target, RIGHT);
		//	else NowNode = NowNode->Merge(right_target, LEFT);
			else if (right_target != nullptr) NowNode = NowNode->Merge(right_target, LEFT);
		}
		NowNode = RootPointer;
		if (NowNode->KeyValue.empty()
			&& NowNode->GetType() != OOPD::LEAF) // 万一删光了？
		{
			NowNode->ChildPointer[0]->ParentPointer = nullptr;
			RootPointer = NowNode->ChildPointer[0];
			delete NowNode;
		}
		return true;
	}

	template<typename T>
	bool BPTree<T>::Update(T & key, T & new_key)
	{
		bool flag = false;
		Node<T>* NowNode = RootPointer;
		Data* target;
		if (Search(new_key) != nullptr) return false;
		while (NowNode->GetType() != LEAF) //寻找待更新数据指针所在叶结点
		{
			flag = false;
			for (auto it = NowNode->KeyValue.begin(); it != NowNode->KeyValue.end(); ++it)
			{
				if (key >= *it) continue;
				NowNode = NowNode->ChildPointer[it - NowNode->KeyValue.begin()];
				flag = true;
				break;
			}
			if (!flag) NowNode = NowNode->ChildPointer.back();
		}
		flag = false;
		for (auto it = NowNode->KeyValue.begin(); it != NowNode->KeyValue.end(); ++it)
		{
			if (key != *it) continue;
			flag = true;
			target = NowNode->DataPointer[it - NowNode->KeyValue.begin()]; //暂存并删除原数据指针
			NowNode->DataPointer.erase(it - NowNode->KeyValue.begin() + NowNode->DataPointer.begin());
			NowNode->KeyValue.erase(it);
			break;
		}
		if (!flag) return false;
		if (NowNode->ParentPointer != nullptr && NowNode->GetType() == LEAF) //更新父结点Key值
			for (auto it = NowNode->ParentPointer->ChildPointer.begin(); it != NowNode->ParentPointer->ChildPointer.end(); ++it)
			{
				if (*it != NowNode) continue;
				if (it == NowNode->ParentPointer->ChildPointer.begin()) break;
				if (!NowNode->KeyValue.empty())
				   NowNode->ParentPointer->KeyValue[it - NowNode->ParentPointer->ChildPointer.begin() - 1] = (*it)->KeyValue[0];
				else if (it + 1 != NowNode->ParentPointer->ChildPointer.end())
				   NowNode->ParentPointer->KeyValue[it - NowNode->ParentPointer->ChildPointer.begin() - 1] = (*(it + 1))->KeyValue[0];
				break;
			}
		while (NowNode->KeyValue.size() < Rank / 2) //进行必要的旋转、合并操作
		{
			if (NowNode == RootPointer)
			{
				if (!NowNode->KeyValue.empty()) break;
				if (NowNode->ChildPointer.empty())
				{
					RootPointer = nullptr;
					delete NowNode;
					return true;
				}
				NowNode->ChildPointer[0]->ParentPointer = nullptr;
				RootPointer = NowNode->ChildPointer[0];
				delete NowNode;
				break;
			}
			Node<T>* left_target = nullptr;
			Node<T>* right_target = nullptr;
			Node<T>* parent = NowNode->ParentPointer;
			for (auto it = parent->ChildPointer.begin(); it != parent->ChildPointer.end(); ++it)
			{
				if (*it != NowNode) continue;
				if (it != parent->ChildPointer.begin()) left_target = *(it - 1);
				if ((it + 1) != parent->ChildPointer.end()) right_target = *(it + 1);
				break;
			}
			if (left_target != nullptr && left_target->KeyValue.size() >= Rank / 2 + 1)
			{
				NowNode->Spin(left_target, RIGHT);
				break;
			}
			else if (right_target != nullptr && right_target->KeyValue.size() >= Rank / 2 + 1)
			{
				NowNode->Spin(right_target, LEFT);
				break;
			}
			if (left_target != nullptr) NowNode = NowNode->Merge(left_target, RIGHT);
			else NowNode = NowNode->Merge(right_target, LEFT);
		}
		NowNode = RootPointer; //重置NowNode，准备插入新结点
		while (NowNode->GetType() != LEAF)
		{
			flag = false;
			for (auto it = NowNode->KeyValue.begin(); it != NowNode->KeyValue.end(); ++it)
			{
				if (new_key >= *it) continue;
				NowNode = NowNode->ChildPointer[it - NowNode->KeyValue.begin()];
				flag = true;
				break;
			}
			if (!flag) NowNode = NowNode->ChildPointer.back();
		}
		flag = false;
		for (auto it = NowNode->KeyValue.begin(); it != NowNode->KeyValue.end(); ++it) //将新插入结点的数据指针赋值为原数据指针
		{
			if (new_key == *it) return false;
			if (new_key > *it) continue;
			NowNode->DataPointer.insert(it - NowNode->KeyValue.begin() + NowNode->DataPointer.begin(), target);
			NowNode->KeyValue.insert(it, new_key);
			flag = true;
			break;
		}
		if (!flag)
		{
			NowNode->DataPointer.push_back(target);
			NowNode->KeyValue.push_back(new_key);
		}
		while (NowNode->KeyValue.size() >= Rank) //进行必要的分裂操作
		{
			NowNode = NowNode->Split(Rank / 2);
			RootPointer = NowNode;
		}
		while (RootPointer->ParentPointer != nullptr)
			RootPointer = RootPointer->ParentPointer;
		return true;
	}

	template<typename T>
	Node<T>* BPTree<T>::GetFirstNode()
	{
		Node<T>* NowNode = RootPointer;
		if (NowNode == nullptr) return NowNode;
		while (NowNode->GetType() != LEAF)
			NowNode = NowNode->ChildPointer[0];
		return NowNode;
	}

	template<typename T>
	Node<T>* BPTree<T>::GetTargetNode(T & key)
	{
		Node<T>* NowNode = RootPointer;
		while (NowNode->GetType() != LEAF)
		{
			bool flag = false;
			for (auto it = NowNode->KeyValue.begin(); it != NowNode->KeyValue.end(); ++it)
			{
				if (*it <= key) continue;
				NowNode = NowNode->ChildPointer[it - NowNode->KeyValue.begin()];
				flag = true;
				break;
			}
			if (!flag) NowNode = NowNode->ChildPointer.back();
		}
		return NowNode;
	}

	template<typename T>
	void BPTree<T>::DeleteNode(Node<T>* pos)
	{
		if (pos == nullptr) return;
		for (auto it = pos->ChildPointer.begin(); it != pos->ChildPointer.end(); ++it)
			DeleteNode(*it);
		delete pos;
		return;
	}

	template<typename T>
	Data* BPTree<T>::Search(T & key)
	{
		Data* target = nullptr;
		Node<T>* NowNode = GetTargetNode(key);
		for (auto it = NowNode->KeyValue.begin(); it != NowNode->KeyValue.end(); ++it)
		{
			if (key != *it) continue;
			target = NowNode->DataPointer[it - NowNode->KeyValue.begin()];
			break;
		}
		return target;
	}

	template <typename T>
	std::vector<Data*> BPTree<T>::BatchSearch(T key_lower, T key_upper, bool lower_close, bool upper_close)
	{
		std::vector<Data*> data;
		Node<T>* NowNode = GetTargetNode(key_lower);
		for (auto it = NowNode->KeyValue.begin(); it != NowNode->KeyValue.end(); ++it)//搜寻符合条件的数据并返回其指针
		{
			if (lower_close)
				if (*it < key_lower) continue;
			if (!lower_close)
				if (*it <= key_lower) continue;
			if (upper_close)
				if (*it > key_upper) return data;
			if (!upper_close)
				if (*it >= key_upper) return data;
			data.push_back(NowNode->DataPointer[it - NowNode->KeyValue.begin()]);
		};
		if (data.empty()) return data;
		NowNode = NowNode->NextNode;
		while (NowNode != nullptr) //继续向前搜寻符合条件的数据并返回其指针
		{
			for (auto it = NowNode->KeyValue.begin(); it != NowNode->KeyValue.end(); ++it)
			{
				if (upper_close)
					if (*it > key_upper) return data;
				if (!upper_close)
					if (*it >= key_upper) return data;
				data.push_back(NowNode->DataPointer[it - NowNode->KeyValue.begin()]);
			}
			NowNode = NowNode->NextNode;
		}
		return data;
	}

	template <typename T>
	std::vector<Data*> BPTree<T>::BatchSearch(T key, bool close, bool greater_than)
	{
		std::vector<Data*> data;
		Node<T>* NowNode;
		if (greater_than) //判断大小
		{
			NowNode = GetTargetNode(key);
			for (auto it = NowNode->KeyValue.begin(); it != NowNode->KeyValue.end(); ++it) //搜寻符合条件的数据并返回其指针
			{
				if (close)
					if (*it < key) continue;
				if (!close)
					if (*it <= key) continue;
				data.push_back(NowNode->DataPointer[it - NowNode->KeyValue.begin()]);
			};
			if (data.empty()) return data;
			NowNode = NowNode->NextNode;
			while (NowNode != nullptr) //继续向前搜寻符合条件的数据并返回其指针
			{
				for (auto it = NowNode->KeyValue.begin(); it != NowNode->KeyValue.end(); ++it)
					data.push_back(NowNode->DataPointer[it - NowNode->KeyValue.begin()]);
				NowNode = NowNode->NextNode;
			}
		}
		else
		{
			NowNode = GetFirstNode();
			while (NowNode != nullptr) //从第一个数据开始搜寻符合条件的数据并返回其指针
			{
				for (auto it = NowNode->KeyValue.begin(); it != NowNode->KeyValue.end(); ++it)
				{
					if (close)
						if (*it > key) return data;
					if (!close)
						if (*it >= key) return data;
					data.push_back(NowNode->DataPointer[it - NowNode->KeyValue.begin()]);
				}
				NowNode = NowNode->NextNode;
			}
		}
		return data;
	}

	template <typename T>
	std::vector<Data*> BPTree<T>::GetData()
	{
		std::vector<Data*> data;
		Node<T>* NowNode = GetFirstNode();
		while (NowNode != nullptr) //遍历叶结点数据指针
		{
			for (auto it = NowNode->KeyValue.begin(); it != NowNode->KeyValue.end(); ++it)
				data.push_back(NowNode->DataPointer[it - NowNode->KeyValue.begin()]);
			NowNode = NowNode->NextNode;
		}
		return data;
	}
}

#endif // BPTREE_H

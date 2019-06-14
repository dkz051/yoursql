#ifndef BPTREENODE_H
#define BPTREENODE_H

#include <vector>
#include <string>
#include "SubClass.h"

namespace OOPD
{
	enum NodeType{LEAF, INTERIOR};
	enum Direction{LEFT, RIGHT};
	enum Function{COPY, MOVE};

	template<typename T>
	class Node
	{
		template<typename T1> friend class BPTree; // 将BP树声明为Node的友元类，确保其能对Node中的保护成员进行操作
	public:
		Node(Node* parent, NodeType type = LEAF) : Type(type), ParentPointer(parent), NextNode(nullptr), PreNode(nullptr){} //初始化结点
		NodeType GetType() {return Type;} //获取结点类型（内部结点、叶结点）
		void SetType(NodeType type) {Type = type;} //设置结点类型（内部结点、叶结点）
		int InsertKey(T & index, Function fun = COPY); //在同一个结点内插入键值
		Node<T>* Split(int split_pos); //进行分裂操作
		Node<T>* Merge(Node<T>* target, Direction dir); //进行合并操作
		void Spin(Node<T>* target, Direction dir); //进行旋转操作
	protected:
		NodeType Type;
		Node<T>* ParentPointer;
		Node<T>* PreNode;
		Node<T>* NextNode;
		std::vector<Node<T>*> ChildPointer;
		std::vector<Data*> DataPointer;
		std::vector<T> KeyValue;
	};

	template<typename T>
	int Node<T>::InsertKey(T & index, Function fun)
	{
		for (auto it = KeyValue.begin(); it != KeyValue.end(); ++it)
		{
			if ((*it) < index) continue;
			int pos = it - KeyValue.begin() + 1;
			if (fun == COPY) KeyValue.insert(it, index);
			else KeyValue.insert(it, std::move(index));
			return pos;
		}
		int pos = KeyValue.size() + 1;
		if (fun == COPY) KeyValue.push_back(index);
		else KeyValue.push_back(std::move(index));
		return pos;
	}

	template<typename T>
	Node<T>* Node<T>::Split(int split_pos)
	{
		Node<T>* parent = this->ParentPointer;
		Node<T>* new_brother = new Node<T>(parent, this->GetType());
		bool flag = this->GetType() == INTERIOR;
		if (parent == nullptr) //判断是不是需要新加根结点
		{
			 parent = new Node<T>(nullptr, INTERIOR);
			 parent->ChildPointer.push_back(this);
		}
		if (flag) parent->ChildPointer.insert(parent->ChildPointer.begin() + parent->InsertKey(this->KeyValue[split_pos], MOVE), new_brother); //更新父结点的子结点指针
		else
		{
			parent->ChildPointer.insert(parent->ChildPointer.begin() + parent->InsertKey(this->KeyValue[split_pos]), new_brother);
			new_brother->KeyValue.push_back(std::move(this->KeyValue[split_pos])); //在新结点上更新内部结点的键值
			new_brother->DataPointer.push_back(std::move(this->DataPointer[split_pos]));
		}
		for (auto it = this->KeyValue.begin() + split_pos + 1; it != this->KeyValue.end(); ++it) //在新结点上更新结点的子结点指针或数据指针
		{
			new_brother->KeyValue.push_back(std::move(*it));
			if (flag)
			{
				new_brother->ChildPointer.push_back(std::move(*(it - this->KeyValue.begin() + this->ChildPointer.begin())));
				new_brother->ChildPointer.back()->ParentPointer = new_brother;
			}
			else new_brother->DataPointer.push_back(std::move(this->DataPointer[it - this->KeyValue.begin()]));
		}
		if (flag) //删除原内部结点中的一部分子结点指针
		{
			new_brother->ChildPointer.push_back(std::move(this->ChildPointer.back()));
			new_brother->ChildPointer.back()->ParentPointer = new_brother;
			this->KeyValue.erase(this->KeyValue.begin() + split_pos, this->KeyValue.end());
			this->ChildPointer.erase(this->ChildPointer.begin() + split_pos + 1, this->ChildPointer.end());
		}
		else //删除原叶结点中的一部数据指针
		{
			this->KeyValue.erase(this->KeyValue.begin() + split_pos, this->KeyValue.end());
			this->DataPointer.erase(this->DataPointer.begin() + split_pos, this->DataPointer.end());
			if (this->NextNode != nullptr)
			{
				this->NextNode->PreNode = new_brother;
				new_brother->NextNode = this->NextNode;
			}
			new_brother->PreNode = this;
			this->NextNode = new_brother;
		}
		this->ParentPointer = parent; //更新原结点和新结点的父指针
		new_brother->ParentPointer = parent;
		return parent; //返回二者共同的父指针
	}

	template<typename T>
	void Node<T>::Spin(Node<T>* target, Direction dir)
	{
		Node<T>* LeftNode;
		Node<T>* parent = this->ParentPointer;
		bool flag = this->GetType() == INTERIOR;
		if (dir == LEFT) LeftNode = this; //判断旋转方向
		else LeftNode = target;
		for (auto it = parent->KeyValue.begin(); it != parent->KeyValue.end(); ++it)
		{
			if (parent->ChildPointer[it - parent->KeyValue.begin()] != this && parent->ChildPointer[it - parent->KeyValue.begin()] != target) continue;
			if (dir == LEFT)
			{
				if (flag) //对内部结点进行子结点指针转移及键值转移
				{
					this->KeyValue.push_back(std::move(*it));
					*it = std::move(target->KeyValue[0]);
					target->KeyValue.erase(target->KeyValue.begin());
					target->ChildPointer[0]->ParentPointer = this;
					this->ChildPointer.push_back(std::move(target->ChildPointer[0]));
					target->ChildPointer.erase(target->ChildPointer.begin());
				}
				else //对叶结点进行数据指针转移及键值转移
				{
					*it = target->KeyValue[1];
					this->KeyValue.push_back(std::move(target->KeyValue[0]));
					this->DataPointer.push_back(std::move(target->DataPointer[0]));
					target->KeyValue.erase(target->KeyValue.begin());
					target->DataPointer.erase(target->DataPointer.begin());
				}
			}
			else
			{
				if (flag) //对内部结点进行子结点指针转移及键值转移
				{
					this->KeyValue.insert(this->KeyValue.begin(), std::move(*it));
					*it = std::move(target->KeyValue.back());
					target->KeyValue.pop_back();
					target->ChildPointer.back()->ParentPointer = this;
					this->ChildPointer.insert(this->ChildPointer.begin(), std::move(target->ChildPointer.back()));
					target->ChildPointer.pop_back();
				}
				else //对叶结点进行数据指针转移及键值转移
				{
					*it = target->KeyValue.back();
					this->KeyValue.insert(this->KeyValue.begin(), std::move(target->KeyValue.back()));
					this->DataPointer.insert(this->DataPointer.begin(), std::move(target->DataPointer.back()));
					target->KeyValue.pop_back();
					target->DataPointer.pop_back();

				}
			}
			break;
		}
	}

	template<typename T>
	Node<T>* Node<T>::Merge(Node<T>* target, Direction dir)
	{
		Node<T>* LeftNode;
		Node<T>* RightNode;
		Node<T>* parent = this->ParentPointer;
		bool flag = this->GetType() == INTERIOR;
		bool flagX = false;
		if (dir == LEFT) //判断合并方向
		{
			LeftNode = this;
			RightNode = target;
		}
		else
		{
			LeftNode = target;
			RightNode = this;
		}
		if (!flag) //修改叶结点的前后结点指针
		{
			if (RightNode->NextNode != nullptr)
			{
				LeftNode->NextNode = RightNode->NextNode;
				RightNode->NextNode->PreNode = LeftNode;
			}
			else LeftNode->NextNode = nullptr;
		}
		for (auto it = parent->ChildPointer.begin(); it != parent->ChildPointer.end(); ++it) //删除多余结点
		{
			if (*it != RightNode) continue;
			if (flag) LeftNode->KeyValue.push_back(std::move(parent->KeyValue[it - parent->ChildPointer.begin() - 1]));
			parent->KeyValue.erase(it - parent->ChildPointer.begin() + parent->KeyValue.begin() - 1);
			parent->ChildPointer.erase(it);
			flagX = true;
			break;
		}
		if (!flagX && flag) //对内部结点进行子结点指针转移以及键值转移
		{
			LeftNode->KeyValue.push_back(std::move(parent->KeyValue.back()));
			parent->ChildPointer.pop_back();
			parent->KeyValue.pop_back();
		}
		for (auto it = RightNode->KeyValue.begin(); it != RightNode->KeyValue.end(); ++it)
			LeftNode->KeyValue.push_back(std::move((*it)));
		if (flag) //对内部结点进行子结点指针转移
			for (auto it = RightNode->ChildPointer.begin(); it != RightNode->ChildPointer.end(); ++it)
			{
				LeftNode->ChildPointer.push_back(std::move((*it)));
				LeftNode->ChildPointer.back()->ParentPointer = LeftNode;
			}
		else //对叶结点进行数据指针转移
			for (auto it = RightNode->DataPointer.begin(); it != RightNode->DataPointer.end(); ++it)
				LeftNode->DataPointer.push_back(std::move((*it)));
		delete RightNode; //删除被合并的结点
		return LeftNode;
	}
}

#endif // BPTREENODE_H

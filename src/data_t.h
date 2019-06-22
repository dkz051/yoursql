#ifndef DATA_T_H
#define DATA_T_H

// 如果做完所有需求后还有时间的话就把第一阶段的类型系统移植过来（但每个数据类型都增加 isNull 字段），从而消除 magic number

class DataInt
{
private:
	bool isNull;
	int value;
public:
	DataInt();
	DataInt(int value);
};

#endif

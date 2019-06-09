#include <string>

const std::string hiddenPrimaryKey = "^HiddenKey"; // 没有主键时自动插入一个默认主键 (int, auto_increment)；该名称由于带了特殊符号因此不会出现在输入中

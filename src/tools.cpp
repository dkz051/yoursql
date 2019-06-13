#include "tools.h"

std::string trimString(std::string str)
{
	unsigned left = 0, right = str.length();
	while (left < str.length() && (str[left] == ' ' || str[left] == '\n')) ++left;
	if (left == str.length()) return "";
	while (right >= left && str[right - 1] == ' ' || str[right - 1] == '\n') --right;
	return str.substr(left, right - left);
}

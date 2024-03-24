
#include "DBManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>


namespace db
{
	const std::string split_maohao = ":";
	const std::string split_fenhao = ";";
	const std::string split_jiange = ",";

	std::vector<std::string> split(const std::string &str, const std::string &pattern)
	{
		char * strc = new char[strlen(str.c_str()) + 1];
		strcpy(strc, str.c_str());
		std::vector<string> resultVec;
		char* tmpStr = strtok(strc, pattern.c_str());
		while (tmpStr != NULL)
		{
			resultVec.push_back(string(tmpStr));
			tmpStr = strtok(NULL, pattern.c_str());
		}

		delete[] strc;

		return resultVec;
	}
	void Replace(std::string& src, char c)
	{
		string re = "*";
		int pos = src.find(c);////查找指定的串 
		while (pos != -1)
		{
			src.replace(pos, 1, re);
			pos = src.find(c);
		}
	}
	//******************************************************************************

	
}
#include <fstream>
#include <string>
#include <map>
#include <filesystem>
#include <iostream>
#include "../SZKZZTrader1.0/INPUT_STRUCT.h"
namespace fs = std::filesystem;
std::map<int, std::vector<std::string>> group_of_ins;
std::map<int, std::string> insid_string;
int groupids[20] = { 1, 2, 3, 4, 5, 6, 20,
2011, 2012, 2013, 2014,  2021, 2022, 2023, 2024,  2031, 2032, 2033, 2034, 2061 };
std::string groupids_string[20] = { "SH1", "SH2", "SH3", "SH4", "SH5", "SH6", "SH20",
"SZ2011", "SZ2012", "SZ2013", "SZ2014",  "SZ2021", "SZ2022", "SZ2023", "SZ2024",  "SZ2031", "SZ2032", "SZ2033", "SZ2034", "SZ2061" };
void Initialization()
{
	for (int i = 0; i < 20; i++)
	{
		std::vector<std::string> tmp;
		group_of_ins.insert({ groupids[i],tmp });
		insid_string.insert({ groupids[i], groupids_string[i] });
	}
}
void InsSortByGroup(std::string fpath)
{//默认打开的都是order文件
	std::ifstream ord_in;
	int ordsize = sizeof(ORDER_STRUCT);
	ORDER_STRUCT* ord = (ORDER_STRUCT*)malloc(ordsize);
	for (const auto& entry : fs::directory_iterator(fpath))
	{
		memset(ord, 0, sizeof(ORDER_STRUCT));
		if (entry.path().stem().string()[0] == '6')
			break;
		ord_in.open(entry.path(),std::ios::in|std::ios::binary);
		if (!ord_in.is_open())
		{//文件打开失败就报错
			std::cout << entry.path().string() << "open fail" << std::endl;
			continue;
		}
		ord_in.read((char*)ord, ordsize);
		std::string filename = entry.path().filename().string();
		if (filename == "000546.dat")
			std::cout << ord->OrderGroupID;
		group_of_ins[ord->OrderGroupID].push_back(entry.path().filename().string());
		ord_in.close();
	}
	//std::ofstream tab2csv;
	//std::string outname = fpath + "instrument's group.csv";
	//tab2csv.open(outname);
	//if (!tab2csv.is_open()) //如果输出的文件没能正确打开
	//{
	//	std::cout << outname << "open fail." << std::endl;
	//	return;
	//}
	//tab2csv << "合约代码,组别" << std::endl;
	//for (auto it = group_of_ins.begin(); it != group_of_ins.end(); it++)
	//	tab2csv << it->first << "," << it->second << std::endl;
}

void SortNCopy(std::string root, std::string date)
{
	//根据传入的orderpath
	for (auto it = group_of_ins.begin(); it != group_of_ins.end(); it++)
	{//每个组号对应的vector下存储着相应的合约代码
		//新建一个合约专属的文件夹
		std::string new_dir = root + "\\" + date + insid_string[it->first];
		fs::create_directory(new_dir);
		//打开传入的文件夹
		for (auto filename : it->second)
		{
			std::string file = root + "\\" + date + "\\" + filename;
			try {
				fs::copy(file, new_dir);
			}
			catch (std::filesystem::filesystem_error const& ex) {
				std::cout
					<< "what():  " << ex.what() << '\n'
					<< "path1(): " << ex.path1() << '\n'
					<< "path2(): " << ex.path2() << '\n'
					<< "code().value():    " << ex.code().value() << '\n'
					<< "code().message():  " << ex.code().message() << '\n'
					<< "code().category(): " << ex.code().category().name() << '\n';
			}
		}
	}
}

int main(int argc,char* argv[])
{
	std::string fpath = "D:\\chen1ting\\StockRawData\\stockdata";
	std::string date = "20210902";
	if (argc == 2)	
		fpath = argv[1];
	std::string ordpath = fpath + "\\ord";
	std::string txnpath = fpath + "\\txn";
	std::string mktpath = fpath + "\\mkt";
	Initialization();
	//鉴于组别不会轻易更改,所以先写一个函数读取某合约的组别,存入某结构体中,输出该结构体为excel表格
	InsSortByGroup(ordpath + "\\" + date);
	//之后在各自的文件目录下面创建新文件夹,将老的文件复制过去
	SortNCopy(ordpath, date);
	SortNCopy(txnpath, date);
	SortNCopy(mktpath, date);
}
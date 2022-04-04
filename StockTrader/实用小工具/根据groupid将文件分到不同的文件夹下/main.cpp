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
{//Ĭ�ϴ򿪵Ķ���order�ļ�
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
		{//�ļ���ʧ�ܾͱ���
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
	//if (!tab2csv.is_open()) //���������ļ�û����ȷ��
	//{
	//	std::cout << outname << "open fail." << std::endl;
	//	return;
	//}
	//tab2csv << "��Լ����,���" << std::endl;
	//for (auto it = group_of_ins.begin(); it != group_of_ins.end(); it++)
	//	tab2csv << it->first << "," << it->second << std::endl;
}

void SortNCopy(std::string root, std::string date)
{
	//���ݴ����orderpath
	for (auto it = group_of_ins.begin(); it != group_of_ins.end(); it++)
	{//ÿ����Ŷ�Ӧ��vector�´洢����Ӧ�ĺ�Լ����
		//�½�һ����Լר�����ļ���
		std::string new_dir = root + "\\" + date + insid_string[it->first];
		fs::create_directory(new_dir);
		//�򿪴�����ļ���
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
	//������𲻻����׸���,������дһ��������ȡĳ��Լ�����,����ĳ�ṹ����,����ýṹ��Ϊexcel���
	InsSortByGroup(ordpath + "\\" + date);
	//֮���ڸ��Ե��ļ�Ŀ¼���洴�����ļ���,���ϵ��ļ����ƹ�ȥ
	SortNCopy(ordpath, date);
	SortNCopy(txnpath, date);
	SortNCopy(mktpath, date);
}
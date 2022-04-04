#include "MARKET_MANAGER.h"
#include <fstream>
#include <Windows.h>

int main(int argc, char* argv[])
{
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);
    mode &= ~ENABLE_QUICK_EDIT_MODE;
    mode &= ~ENABLE_INSERT_MODE;
    mode &= ~ENABLE_MOUSE_INPUT;
    SetConsoleMode(hStdin, mode);

    std::string sample_day = "20210805";
    std::string user_input_paths[3] = {
    "D:\\chen1ting\\StockRawData\\kzzdata\\ord",
    "D:\\chen1ting\\StockRawData\\kzzdata\\txn",
    "D:\\chen1ting\\StockRawData\\kzzdata\\mkt", };

    std::vector<std::string> ins_ids;
    for (const auto& dirEntry : fs::directory_iterator(fs::path(user_input_paths[0]) / sample_day))
    {
        auto cur_file_name = dirEntry.path().stem().string();
        if (cur_file_name[0] == '1' && cur_file_name[1] == '2')
            ins_ids.emplace_back(cur_file_name);
    }

    std::vector<std::string> dates;
    for (const auto& dirEntry : fs::directory_iterator(fs::path(user_input_paths[0])))
        dates.emplace_back(dirEntry.path().filename().string());

    for (auto dates_it = dates.begin(); dates_it != --dates.end(); ++dates_it)
    {
        std::string yesterday = *dates_it; // "20210803";//
        std::string today =  *(dates_it + 1); // "20210804"; //
        for (auto it = ins_ids.begin(); it != ins_ids.end(); ++it)
        {
            //��ͣ�۸��쳣
            if (today == "20210825" && *it == "123027")     //bug:����ʱ���Ͼ��۳���,��������ͣ�۸�δͣ��
                continue;
            //������伯�Ͼ��۵��쳣���
            if (today == "20210813" && *it == "123015")
                continue;
            if (today == "20210820" && *it == "123030")
                continue;
            
            std::string ins_id =  *it; //"123093";//
            std::string file_name = ins_id + ".dat";
            MarketManager MktMgr(KZZ);
            MktMgr.GetAllOrd(fs::path(user_input_paths[0]) / today / file_name);
            MktMgr.GetAllTxn(fs::path(user_input_paths[1]) / today / file_name);
            MktMgr.GetLstClosePrice(fs::path(user_input_paths[2]) / yesterday / file_name); //��ʼ�����ռۺͽ���ͣ�Ƽ۸�
            if (MktMgr.FileOpenUnsuccesssful)          //ĳ�յ��ļ�δ��ȷ��
                continue;
            MktMgr.StartTrade();

            //���ƴ���Ƿ���ȷ
           if (MktMgr.CheckSimCorrectness() == false)
            {
                std::cout << "�쳣���ڣ�" << today << "\t�쳣��Լ��" << ins_id << std::endl << std::endl;
                system((std::string("D:\\chen1ting\\CppProj\\binary2csv\\Release\\binary2csv.exe ") + ins_id + " " + today).c_str());
                MktMgr.Trades2File();
            }
            else
                std::cout << "�������ڣ�" << today << "\t������Լ��" << ins_id << std::endl;
        }
    }
}
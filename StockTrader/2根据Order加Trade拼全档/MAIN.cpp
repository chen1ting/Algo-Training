//该TradeSim仅用于处理上海可转债
#include "MARKET_SIMULATOR.h"
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

    std::string sample_day = "20210901";
    std::string ordpath = "D:\\chen1ting\\StockRawData\\stockdata\\ord";
    std::string txnpath = "D:\\chen1ting\\StockRawData\\stockdata\\txn" ;
    std::string mktpath = "D:\\chen1ting\\StockRawData\\stockdata\\mkt" ;

    std::vector<std::string> ins_ids;
    for (const auto& dirEntry : fs::directory_iterator(fs::path(ordpath) / sample_day))
            ins_ids.emplace_back(dirEntry.path().stem().string());

    std::vector<std::string> dates;
    for (const auto& dirEntry : fs::directory_iterator(fs::path(ordpath)))
        dates.emplace_back(dirEntry.path().filename().string());

    for (auto dates_it = dates.begin(); dates_it != --dates.end(); ++dates_it)
    {
        if (*dates_it == "20210902")
            continue;
        std::string yesterday =  *dates_it; //"20210903";//
        std::string today =  *(dates_it + 1); //"20210906"; // 
        std::cout << today << std::endl;
        for (auto it = ins_ids.begin(); it != ins_ids.end(); ++it)
        {
            //*it = "300011";
            //if ((*it)[0] == '0' || (*it)[0] == '2' || (*it)[0] == '3')
            //    continue; //debug时用于跳过检查深圳
            MarketSimulator MktSimu;
            MktSimu.GetAllOrd(fs::path(ordpath) / today / (*it + ".dat"));
            MktSimu.GetAllTxn(fs::path(txnpath) / today / (*it + ".dat"));
            MktSimu.GetLstClosePrice(fs::path(txnpath) / yesterday / (*it + ".dat")); //初始化昨收价和今日停牌价格
            if (MktSimu.FileOpenUnsuccesssful)          //某日的文件未正确打开
                continue;

            if ((*it)[0] == '0' || (*it)[0] == '2' || (*it)[0] == '3')
            {
                MktSimu.Start(SZ);
                if (MktSimu.CheckCorrectness(fs::path(mktpath) / today / (*it + ".dat")) == false)
                {
                    system((std::string("D:\\chen1ting\\CppProj\\binary2csv\\Release\\binary2csv.exe ") + *it + " " + today).c_str());
                    system("PAUSE");
                    return 0;
                }
                else
                    std::cout<< *it << "无误\n";
            }
            else
            {
                MktSimu.Start(SH);
                if (MktSimu.CheckCorrectness(fs::path(mktpath) / today / (*it + ".dat")) == false)
                {
                    system((std::string("D:\\chen1ting\\CppProj\\binary2csv\\Release\\binary2csv.exe ") + *it + " " + today).c_str());
                    system("PAUSE");
                    return 0;
                }
                else
                    std::cout << *it << "无误\n";
            }
        }
        std::cout << today << "无误" << std::endl;
    }
}
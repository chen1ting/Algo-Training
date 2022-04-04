#include "标头.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <unordered_set>
using namespace std;
namespace fs = std::filesystem;
static char ContBidBegTime[13] = "09:25:00.000";
static char MktOpen[13] = "09:30:00.000";
static char ContBidEndTime[13] = "14:57:00.000";
static char MktCloseTime[13] = "15:00:00.000";
static string output_file = "D:\\chen1ting\\StockRawData\\bin2csvOutput\\";
bool Time1BiggerEql(char* time1, char* time2)
{
    for (int i = 0; i < 12; i++)
    {
        if (time1[i] > time2[i])
            return true;
        else if (time1[i] < time2[i])
            return false;
    }
    return true; //默认等于为true,即传入两个时间戳相同的订单时，选择先传入的那个
}
ORDER_STRUCT* ord2csv(string input_file, string date, string filename)
{
    ifstream ordf;
    fs::path fp = fs::path(input_file)/ "ord" / date / filename += ".dat";
    ordf.open(fp, ios::in | ios::binary);

    if (!ordf.is_open())
        cout << "StockFile open unsuccessful." << endl;
    if (ordf.tellg() % sizeof(ORDER_STRUCT) != 0)
        cout << "StockFile read unsuccessful." << endl;

    ordf.seekg(0, ios::end);
    int Count = ordf.tellg() / sizeof(ORDER_STRUCT); //初始化Count,用于记录合理的数据范围
    ORDER_STRUCT* buffer = (ORDER_STRUCT*)malloc(sizeof(ORDER_STRUCT) * (Count)); //初始化buffer

    ordf.seekg(0, ios::beg);
    ordf.read((char*)buffer, sizeof(ORDER_STRUCT) * (Count));

    ofstream ord2csv;
    string outname = filename + "_" + date + "order.csv";
    ord2csv.open(fs::path(output_file + outname));
    ord2csv << "OrderTime,OrderIndex,BizIndex,FunctionCode,OrderKind,Price,Volume,InsID,OrdGroup" << endl;
    for (int i = 0; i < Count; i++)
    {
        ORDER_STRUCT ord = buffer[i];
        ord2csv << ord.OrderTime << "," <<
            ord.OrderIndex << "," <<
            ord.BizIndex << "," <<
            ord.FunctionCode << "," <<
            ord.OrderKind << "," <<
            ord.Price << "," <<
            ord.Volume << "," <<
            ord.InstrumentID << "," <<
            ord.OrderGroupID << endl;
    }
    return buffer;
}
TRADE_STRUCT* txn2csv(string input_file, string date, string filename)
{
    ifstream txnf;
    fs::path fp = fs::path(input_file)/ "txn" / date / filename += ".dat";
    txnf.open(fp, ios::in | ios::binary);

    if (!txnf.is_open())
        cout << "StockFile open unsuccessful." << endl;
    if (txnf.tellg() % sizeof(TRADE_STRUCT) != 0)
        cout << "StockFile read unsuccessful." << endl;

    txnf.seekg(0, ios::end);
    int Count = txnf.tellg() / sizeof(TRADE_STRUCT); //初始化Count,用于记录合理的数据范围
    TRADE_STRUCT* buffer = (TRADE_STRUCT*)malloc(sizeof(TRADE_STRUCT) * (Count)); //初始化buffer

    txnf.seekg(0, ios::beg);
    txnf.read((char*)buffer, sizeof(TRADE_STRUCT) * (Count));

    ofstream txn2csv;
    string outname = filename + "_" + date + "txn.csv";
    txn2csv.open(fs::path(output_file + outname));
    txn2csv << "TradeTime,TradeIndex,FunctionCode,OrderBSFlag,Price,Volume,BuyIndex,SellIndex,TradeGroup,InsID" << endl;
    for (int i = 0; i < Count; i++)
    {
        TRADE_STRUCT txn = buffer[i];
        txn2csv << txn.TradeTime << "," <<
            txn.TradeIndex << "," <<
            txn.FunctionCode << "," <<
            txn.OrderBSFlag << "," <<
            txn.Price << "," <<
            txn.Volume << "," <<
            txn.BuyIndex << "," <<
            txn.SellIndex <<","<<
            txn.TradeGroupID <<","<<
            txn.InstrumentID << endl;
    }
    return buffer;
}
void splitTrade(string input_file, string date, string filename)
{
    ifstream txnf;
    fs::path fp = fs::path(input_file)/ "txn" / date / filename += ".dat";
    txnf.open(fp, ios::in | ios::binary);

    if (!txnf.is_open())
        cout << "StockFile open unsuccessful." << endl;
    if (txnf.tellg() % sizeof(TRADE_STRUCT) != 0)
        cout << "StockFile read unsuccessful." << endl;

    txnf.seekg(0, ios::end);
    int Count = txnf.tellg() / sizeof(TRADE_STRUCT); //初始化Count,用于记录合理的数据范围
    TRADE_STRUCT* buffer = (TRADE_STRUCT*)malloc(sizeof(TRADE_STRUCT) * (Count)); //初始化buffer

    txnf.seekg(0, ios::beg);
    txnf.read((char*)buffer, sizeof(TRADE_STRUCT) * (Count));


    ofstream txn2csvdropped;
    string outname = filename + "_" + date + "撤单.csv";
    txn2csvdropped.open(fs::path(output_file + outname));
    txn2csvdropped << "TradeTime,TradeIndex,FunctionCode,OrderBSFlag,Price,Volume,BuyIndex,SellIndex" << endl;
    for (int i = 0; i < Count; i++)
    {
        if (buffer[i].FunctionCode == '4')
        {
            TRADE_STRUCT txn = buffer[i];
            txn2csvdropped << txn.TradeTime << "," <<
                txn.TradeIndex << "," <<
                txn.FunctionCode << "," <<
                txn.OrderBSFlag << "," <<
                txn.Price << "," <<
                txn.Volume << "," <<
                txn.BuyIndex << "," <<
                txn.SellIndex << endl;
        }
    }

    ofstream txn2csvfilled;
    outname = filename + "_" + date + "成交单.csv";
    txn2csvfilled.open(fs::path(output_file + outname));
    txn2csvfilled << "TradeTime,TradeIndex,FunctionCode,OrderBSFlag,Price,Volume,BuyIndex,SellIndex" << endl;

    for (int i = 0; i < Count; i++)
    {
        if (buffer[i].FunctionCode == 'F')
        {
            TRADE_STRUCT txn = buffer[i];
            txn2csvfilled << txn.TradeTime << "," <<
                txn.TradeIndex << "," <<
                txn.FunctionCode << "," <<
                txn.OrderBSFlag << "," <<
                txn.Price << "," <<
                txn.Volume << "," <<
                txn.BuyIndex << "," <<
                txn.SellIndex << endl;
        }
    }
}
void mkt2csv(string input_file, string date, string filename)
{
    ifstream mktf;
    fs::path fp = fs::path(input_file) / "mkt" / date / filename += ".dat";
    mktf.open(fp, ios::in | ios::binary);

    if (!mktf.is_open())
        cout << "StockFile open unsuccessful." << endl;
    if (mktf.tellg() % sizeof(MARKET_STRUCT) != 0)
        cout << "StockFile read unsuccessful." << endl;

    mktf.seekg(0, ios::end);
    int Count = mktf.tellg() / sizeof(MARKET_STRUCT); //初始化Count,用于记录合理的数据范围
    MARKET_STRUCT* buffer = (MARKET_STRUCT*)malloc(sizeof(MARKET_STRUCT) * (Count)); //初始化buffer

    mktf.seekg(0, ios::beg);
    mktf.read((char*)buffer, sizeof(MARKET_STRUCT) * (Count));

    ofstream mkt2csv;
    string outname = filename + "_" + date + "mkt.csv";
    mkt2csv.open(fs::path(output_file + outname));
    mkt2csv << "TimeStamp,LastPrice,BidPrice1,BidVolume1,BidPrice2,BidVolume2,BidPrice3,BidVolume3,BidPrice4,BidVolume4,BidPrice5,BidVolume5,OfferPrice1,OfferVolume1,OfferPrice2,OfferVolume2,OfferPrice3,OfferVolume3,OfferPrice4,OfferVolume4,OfferPrice5,OfferVolume5," << endl;
    for (int i = 0; i < Count; i++)
    {
        MARKET_STRUCT mkt = buffer[i];
        mkt2csv << mkt.TimeStamp << "," <<
            mkt.LastPrice << "," <<
            mkt.BidPrice1 << "," <<
            mkt.BidVolume1 << "," <<
            mkt.BidPrice2 << "," <<
            mkt.BidVolume2 << "," <<
            mkt.BidPrice3 << "," <<
            mkt.BidVolume3 << "," <<
            mkt.BidPrice4 << "," <<
            mkt.BidVolume4 << "," <<
            mkt.BidPrice5 << "," <<
            mkt.BidVolume5 << "," <<
            mkt.OfferPrice1 << "," <<
            mkt.OfferVolume1 << "," <<
            mkt.OfferPrice2 << "," <<
            mkt.OfferVolume2 << "," <<
            mkt.OfferPrice3 << "," <<
            mkt.OfferVolume3 << "," <<
            mkt.OfferPrice4 << "," <<
            mkt.OfferVolume4 << "," <<
            mkt.OfferPrice5 << "," <<
            mkt.OfferVolume5 << "," <<
            mkt.TotalTradeVolume <<  endl;
    }
}
void MorningCallAuc(string input_file, string date, string filename)
{
    ifstream txnf;
    fs::path fp = fs::path(input_file) / "txn" / date / filename += ".dat";
    txnf.open(fp, ios::in | ios::binary);

    if (!txnf.is_open())
        cout << "StockFile open unsuccessful." << endl;
    if (txnf.tellg() % sizeof(TRADE_STRUCT) != 0)
        cout << "StockFile read unsuccessful." << endl;

    txnf.seekg(0, ios::end);
    int Count = txnf.tellg() / sizeof(TRADE_STRUCT); //初始化Count,用于记录合理的数据范围
    TRADE_STRUCT* buffer = (TRADE_STRUCT*)malloc(sizeof(TRADE_STRUCT) * (Count)); //初始化buffer

    txnf.seekg(0, ios::beg);
    txnf.read((char*)buffer, sizeof(TRADE_STRUCT) * (Count));

    ofstream txn2csv;
    string outname = filename + "_" + date + "MorningAuc.csv";
    txn2csv.open(fs::path(output_file + outname));
    txn2csv << "TradeTime,TradeIndex,FunctionCode,OrderBSFlag,Price,Volume,BuyIndex,SellIndex" << endl;
    for (int i = 0; i < Count; i++)
    {
        if (Time1BiggerEql(buffer[i].TradeTime, MktOpen))
            break;
        if (buffer[i].FunctionCode == 'F')
        {
            TRADE_STRUCT txn = buffer[i];
            txn2csv << txn.TradeTime << "," <<
                txn.TradeIndex << "," <<
                txn.FunctionCode << "," <<
                txn.OrderBSFlag << "," <<
                txn.Price << "," <<
                txn.Volume << "," <<
                txn.BuyIndex << "," <<
                txn.SellIndex << endl;
        }
    }
}
int main(int argc, char* argv[])
{
    string inst = "301019";
    string date = "20210903";
    string inputf = "D:\\chen1ting\\StockRawData\\stockdata\\";

    if (argc == 2)
    {
        inst = argv[1];
        date = argv[2];
    }
    else if (argc == 3)
    {
        inst = argv[1];
        date = argv[2];
        inputf = argv[3];
    }
    if ((inst[0] == '0' && inst[1] != '1') || (inst[0] == '1' && inst[1] != '1') || inst[0] == '2' || inst[0] == '3') //深圳行情推送,01和11是上海的债券
        splitTrade(inputf, date, inst);
    ord2csv(inputf, date, inst);
    txn2csv(inputf, date, inst);
    mkt2csv(inputf, date, inst);
}




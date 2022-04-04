#pragma once

#include"INPUT_STRUCT.h"
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <unordered_set>
#include <unordered_map>
namespace fs = std::filesystem;
constexpr int LEVELDISPLAY = 10;        //输出接口显示的盘口深度
constexpr int SH = 606890;              //上海 用整型数字表示
static char SZ = 'z';                   //深圳 用字符表示

struct Level
{
    //LevelType Type;			         //盘口类型
    double Price;			             //盘口价格
    double TotalVolume;		             //盘口总单量
    std::unordered_set<long long> OrdIDX;    //该盘口的订单队列
};
struct MktInfo
{
    double LstClose;      //昨收
    double TodayOpen;     //今开
    double TodayClose;    //今收

    double NewestPrice;                   //最新成交价
    double BestOffer;                     //市场卖一价
    double BestBid;                       //市场买一价
    double BidPrices[LEVELDISPLAY];       //买盘价格
    double BidVolumes[LEVELDISPLAY];      //买盘量
    double OfferPrices[LEVELDISPLAY];     //卖盘价格
    double OfferVolumes[LEVELDISPLAY];    //卖盘量

    int OrdsCount;                        //买单总单量
    int TxnCount;                         //卖单总单数
    double TotalTradeVol;                 //成交总手数
    double TotalTradeNumber;              //成交总单数
    double TotalVol;                      //订单总手数
};
class MarketSimulator
{
public:
    MarketSimulator();                              //初始化所有成员
    ~MarketSimulator();
    void GetAllOrd(fs::path ordfilepath);           //读入所有订单，存到order_buffer里，初始化current_ord指向第一个订单,如果是创业板股票,初始化PriceBound
    void GetAllTxn(fs::path txnfilepath);           //读入所有订单，存到txn_buffer里，初始化current_txn指向第一个订单 
    void GetLstClosePrice(fs::path txnfilepath);    //读取昨收价，设置有效竞价范围和四档临停价格
    void Start(int sh);                             //上交所的驱动函数:设置买卖旗帜\今开今收\模拟实时盘口模拟
    void Start(char sz);                            //深交所的驱动函数:设置买卖旗帜\今开今收\模拟实时盘口模拟
    //输出函数
    void PrintMktInfo();                            //打印盘口信息
    inline MktInfo* ReturnMktInfo();                //返回包含实时Mkt信息的MktInfo结构体
    bool CheckCorrectness(fs::path mktfilepath);    //检查最后一条盘口信息是否正确
    MktInfo Mkt = { 0 };                            //储存市场信息的结构体，声明时初始化
    bool FileOpenUnsuccesssful = false;             //true: 有文件未能正确打开

private:
    void HangOrd();         //挂起传入的订单，mode = BUYFLAG挂买盘，mode = SELLFLAG挂卖盘，更新买一卖一; new_ord为真,订单第一次进入盘口,更新price_index的哈希表
    void DealTrades(long long idx, int sh);                //找到当前OrderIndex之前所有的撤单，call CancelOrd()进行处理
    void DealTrades(long long idx, char sz);               //找到当前OrderIndex之前所有的撤单，call CancelOrd()进行处理
    void DeleteOrd(long long idx, double volume);          //找到并删除订单
    void UpdateMktInfo();                                  //更新买一价(Mkt.BestBid)
    inline bool Time1Bigger(char* time1, char* time2);     //true: time1 > time2; false: time1 <= time2
    inline bool IsValidPrice(double price, char mode);
    inline void UpdateBestBid();
    inline void UpdateBestOffer();

    ORDER_STRUCT* order_buffer;             //所有订单池
    TRADE_STRUCT* txn_buffer;               //所有交易池
    ORDER_STRUCT* current_ord;              //指向当前正在处理的订单
    int ord_counter;
    TRADE_STRUCT* current_txn;              //指向下一个交易单
    int txn_counter;
    std::map<double, Level> bids_book;                      //买盘数据
    std::map<double, Level> offers_book;                    //卖盘数据
    std::unordered_map<long long, ORDER_STRUCT> all_ords;   //根据OrderIndex查找订单的订单池
    std::map<double, Level>* book;

    char BUYFLAG;          //买单Function Code
    char SELLFLAG;         //卖单Function Code
    double PRICEBOUND;     //有效竞价范围
};

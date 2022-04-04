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
namespace fs = std::filesystem;
constexpr char KZZ = 'k';         //可转债标记
constexpr char CYB = 'c';         //创业板标记
constexpr char ZB = 'z';          //主板标记
constexpr int LEVELDISPLAY = 10;  //输出接口显示的盘口深度
struct Level
{
    double Price;			             //盘口价格
    double TotalVolume;		             //盘口总单量
    std::deque<ORDER_STRUCT> OrdList;    //该盘口的订单队列
};
struct MktInfo
{
    double LstClose;      //昨收
    double TxnPauseBound[4]; //今日临停价格限制: 0-跌30%停牌 1-跌20%停牌 2-涨20%停牌 3-涨30%停牌        
    double DynaHi;         //有效价格范围上限
    double DynaLo;         //有效价格范围下限
    double LimitHi;        //涨停上限
    double LimitLo;        //跌停下限
    double TodayOpen;      //今开
    double TodayClose;     //今收

    double NewestPrice;                   //最新成交价
    double PreNewestPrice;                //前成交价
    double BestBid;                       //买一价
    double BestOffer;                     //卖一价
    double BidPrices[LEVELDISPLAY];       //买盘价格
    double OfferPrices[LEVELDISPLAY];     //卖盘价格
    double BidVolumes[LEVELDISPLAY];      //买盘量
    double OfferVolumes[LEVELDISPLAY];    //卖盘量

    int OrdsCount;              //买单总单量
    int TxnCount;               //卖单总单数
    double TotalTradeVol;       //成交总手数
    double TotalTradeNumber;    //成交总单数
    double TotalVol;            //订单总手数

    bool StopTrade;             //true: 停牌30分钟; 
    bool FirstTimeRiseStop;     //true: 第一次涨20%导致临停
    bool FirstTimeFallStop;     //true: 第一次跌20%导致临停
    char StopTradeUntil[13];    //临停到几点
    long long TxnBizIdx;        //成交单BizIndex
};
class MarketManager
{
public:
    //MarketManager();        //可转债交易
    MarketManager(char a);  //创业板交易
    //MarketManager(int a);   //主板交易
    ~MarketManager();
    void GetAllOrd(fs::path ordfilepath);           //读入所有订单，存到order_buffer里，初始化current_ord指向第一个订单
    void GetAllTxn(fs::path txnfilepath);           //读入所有订单，存到txn_buffer里，初始化current_txn指向第一个订单 
    void GetLstClosePrice(fs::path mktfilepath);    //读取昨收价，设置有效竞价范围和四档临停价格
    void StartTrade();                              //开始可转债交易 - 相当于MarketManager的main函数
    //void StartTrade(char a);                        //开始创业板股票交易 - 相当于MarketManager的main函数
    //void StartTrade(int a);                         //开始主板股票交易 - 相当于MarketManager的main函数
    //输出函数
    void PrintTrades();                             //打印成交单
    void PrintMktInfo();                            //打印盘口信息
    MktInfo* ReturnMktInfo();                       //返回包含实时Mkt信息的MktInfo结构体
    void Trades2File();                             //输出成交单到csv
    void Levels2File(std::string filename);         //输出当前盘口到csv
    bool CheckSimCorrectness();                     //对比当天撮合的成交单和真实成交单

    MktInfo Mkt = { 0 };                            //跟踪市场信息，声明时初始化
    bool FileOpenUnsuccesssful = false;             //true: 有文件未能正确打开

private:
    void CallAuction(char mode);                                     //集合竞价,mode用来标记开盘、复牌、收盘集合竞价等
    void ContBid();                                 //用于连续竞价的函数
    //传入当前撮合单和撮合队列，若撮合队列被平完则删除该盘口，有任一方被平完则返回，返回最新价
    bool MakeOneTrade(ORDER_STRUCT* curord, Level* match_lev);           //成交单个订单,如果match_lev被平完删除,返回true
    void HangOrd(ORDER_STRUCT someord, char mode, bool new_ord);         //挂起传入的订单，mode = BUYFLAG挂买盘，mode = SELLFLAG挂卖盘，更新买一卖一; new_ord为真,订单第一次进入盘口,更新price_index的哈希表
    void DealDroppedOrder(long long idx);                                //找到当前OrderIndex之前所有的撤单，call CancelOrd()进行处理
    void CancelOrd(long long idx, double price, double vol, char mode);  //找到并撤销订单
    void DeleteLevel(double price, char mode);                           //删除盘口，更新买一卖一
    inline bool IsValidPrice(double Price);                              //true:有效竞价                            //true:有效竞价
    inline void UpdateBound(double Price);                               //根据传入价格reset竞价范围（Mkt.DynaHi & Mkt.DynaLo)
    inline void UpdateBestOffer();                                       //更新卖一（Mkt.BestOffer)
    inline void UpdateBestBid();                                         //更新买一价(Mkt.BestBid)
    void UpdateMktInfo();                                                //更新买一价(Mkt.BestBid)
    bool Time1Bigger(char* time1, char* time2);  //true: time1 > time2; false: time1 <= time2
    bool CheckStopTrade();                       //检查是否需要临停并设置市场状态

    char STOCKTYPE;                         //标记交易类型
    float MINGAP;                           //价格最小变动范围
    double TRADEPB;                         //Trade Price Bound：连续竞价、收盘集合竞价的价格涨跌幅限制
    double TEMPSTOP1;                       //临停价格1：停30min
    double TEMPSTOP2;                       //临停价格2：停到14：57
    double LIMIT;                           //涨跌幅限制：停到14：57
    ORDER_STRUCT* order_buffer;             //所有订单池
    TRADE_STRUCT* txn_buffer;               //所有交易池
    ORDER_STRUCT* current_ord;              //指向当前正在处理的订单
    int ord_counter;
    TRADE_STRUCT* current_txn;              //指向下一个交易单
    int txn_counter;
    std::vector<TRADE_STRUCT> trades;       //撮合的成交单容器
    std::vector<TRADE_STRUCT> real_trades;  //真实的成交单容器
    std::map<double, Level> offers_order_book;      //卖盘
    std::map<double, Level> bids_order_book;        //买盘
    std::map<long long, double> idx2price;  //index和价格一一对应的哈希表
};

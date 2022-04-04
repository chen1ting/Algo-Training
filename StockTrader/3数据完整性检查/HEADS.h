#pragma once
#include "INPUT_STRUCT.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <thread>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include <chrono>
#include <Windows.h>
#include <condition_variable>
namespace fs = std::filesystem;
extern HANDLE hStdout;
extern COORD destCoord;
constexpr int ORD_SLOTS = 12;
constexpr int TXN_SLOTS = 12;
constexpr int ORDSTRUCT_SIZE = sizeof(ORDER_STRUCT);
constexpr int TXNSTRUCT_SIZE = sizeof(TRADE_STRUCT);
constexpr int NINSCOUNT = 6000;
constexpr int MAXORDNUM = 1150000;
constexpr int BITMAPSIZE = 1150000;
struct WRONGSEQORD
{
    double cur_idx;
    double pre_idx;
    std::string insid;
    int groupid;
    WRONGSEQORD(double cur, double pre, std::string ins, int group)
    {
        cur_idx = cur;
        pre_idx = pre;
        insid = ins;
        groupid = group;
    }
};
struct ORDBUFF
{
    ORDER_STRUCT* buf;//当前合约所有订单 (假设单合约一天最多收到1,000,000个订单)
    std::atomic<bool> empty = true; //true:当前buf为空; false:当前buf非空
    int cnt;     
    int fileidx;
    int groupid;
};
struct TXNBUFF
{
    TRADE_STRUCT* buf;//当前合约所有订单 (假设单合约一天最多收到1,000,000个订单)
    std::atomic<bool> empty = true; //true:当前buf为空; false:当前buf非空
    int cnt;
    int fileidx;
    int groupid;
    double total_trade_vol;         //从mkt中读取的正确成交总额
};

extern std::string ins_file_names[NINSCOUNT];
extern double trade_volumes[NINSCOUNT];
extern double sim_volumes[NINSCOUNT];
extern std::string groupids_string[20];
extern int ins_id_cnt;          
extern ORDBUFF ordbufs[ORD_SLOTS];
extern TXNBUFF txnbufs[TXN_SLOTS];
extern std::atomic<int> ord_read_ptr;
extern std::atomic<int> txn_read_ptr;
extern std::atomic<int> mkt_read_ptr;
extern std::atomic<int> ord_dealed;
extern std::atomic<int> txn_dealed;     
extern std::mutex bitmap_write_mtx;         
//extern std::mutex cout_mtx;                    
extern std::mutex txn_vol_mtx;
extern std::mutex read_vol_mtx;
extern std::mutex wrong_ord_mtx;
extern unsigned int bitmaps[20][BITMAPSIZE];
extern std::unordered_map<int, int> bmap_hash;
extern std::unordered_map<int, std::vector<long long>> missing_ord;
extern std::unordered_map<int, std::vector<long long>> wrong_sequence_ord;

int GetDates(std::string time, std::string ordpath, std::string* dates);    //传入:1.order文件路径 2.储存日期的字符串数组, 返回要处理的日期天数
void GetInsIds(std::string ordpath, std::string txnpath, std::string mktpath); //传入三个文件路径(带当前日期),检查文件完整性
void ReadOrd(std::string ordfilepath, ORDBUFF* slot, int PID);
void ReadTxn(std::string txnfilepath, TXNBUFF* slot, int PID);
void ReadMkt(std::string mktfilepath);
void DealOrd(int start, int end, int PID);
void DealTxn(int start, int end, int PID); //从start开始in charge of
void SetBitPosTo1(long long n, unsigned int* map);
bool CheckMissingOrd();
bool CheckTradeVol();
bool CheckWrongSeq();
void PrintMissingOrd();
void ProgressBar(int line);
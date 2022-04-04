#include "HEADS.h"
/*
这个project主要检查拆分后文件的订单是否有如下异常:
1. order或trade乱序
2. order或trade缺失(序列号丢失)
3. 丢trade(总成交额不符合)

优化方向:
1. 写入bitmap时,20个位图(一个位图一个组)共用一个函数,效率较低
2. 处理order和txn的多个进程共用一个函数,非常容易阻塞
*/


//一. 遍历合约,读取order\trade\market文件
std::string ins_file_names[NINSCOUNT];         //存储当天需要检查的合约文件名
int ins_id_cnt = 0;                            //存储当天需要检查的合约数
//1.读order
ORDBUFF ordbufs[ORD_SLOTS];                     //order缓冲池, 其中包含ORD_SLOTS个buffer, 每个buffer独立储存某一合约所有的order_struct, 记录该合约的order总数和对应文件名在ins_file_names中的位置
std::atomic<int> ord_read_ptr = 0;              //指向ord最后一个已经读取的合约
//2.读trade
TXNBUFF txnbufs[TXN_SLOTS];                     //txn缓冲池,其中包含TXN_SLOTS个buffer 
std::atomic<int> txn_read_ptr = 0;              //指向txn最后一个已经读取的合约
//3.读mkt
double trade_volumes[NINSCOUNT];                //mkt文件中读取的总成交额
std::mutex read_vol_mtx;                        //mkt read时写入volume的保护锁
std::atomic<int> mkt_read_ptr = 0;              //指向mkt下一个待读取的合约

//二 检查单个合约文件
std::atomic<int> ord_dealed = 0;                //记录已检查完的ord数量
std::atomic<int> txn_dealed = 0;                //记录已检查完的txn数量
//1.乱序检查
std::unordered_map<int, std::vector<long long>> wrong_sequence_ord; //存储乱序的订单编号
std::mutex wrong_ord_mtx;               //写入wrong_sequence_ord的保护锁
//2.bitmap订单缺失检查
unsigned int bitmaps[20][BITMAPSIZE];           //用于记录每个组别的order+txnindex的位置,检查是否有中间丢失的订单. 一个bitmap是一个数组,20个组需要20个bitmap,所以所有位图存储为一个二维数组. 
std::unordered_map<int, int> bmap_hash;         //方便通过groupID直接找到对应位图在bitmaps中的位置 1:0, 2:1, 3:2, 4:3, 5:4, 6:5, 20:6, 2011:7, 2012:8 ...
int groupids[20] = { 1, 2, 3, 4, 5, 6, 20,
2011, 2012, 2013, 2014,  2021, 2022, 2023, 2024,  2031, 2032, 2033, 2034, 2061 };//用于初始化bmap_hash
std::mutex bitmap_write_mtx;                    //bitmap写入保护锁
std::unordered_map<int, std::vector<long long>> missing_ord(20);//存储失踪的订单号
//3.总成交量检查
double sim_volumes[NINSCOUNT];                  //加和得到的总成交额
std::mutex txn_vol_mtx;                         //trade volume写入保护锁

//三 输出检查信息
std::string groupids_string[20] = { "SH1", "SH2", "SH3", "SH4", "SH5", "SH6", "SH20",
"SZ2011", "SZ2012", "SZ2013", "SZ2014",  "SZ2021", "SZ2022", "SZ2023", "SZ2024",  "SZ2031", "SZ2032", "SZ2033", "SZ2034", "SZ2061" };
HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);//控制台输出时,确定窗口的句柄
COORD destCoord;                                 //一个全局可用cursor坐标

int main(int argc, char* argv[])
{
    for (size_t i = 0; i < ORD_SLOTS; i++)
        ordbufs[i].buf = (ORDER_STRUCT*)malloc(sizeof(ORDER_STRUCT) * MAXORDNUM);

    for (size_t i = 0; i < TXN_SLOTS; i++)
        txnbufs[i].buf = (TRADE_STRUCT*)malloc(sizeof(TRADE_STRUCT) * MAXORDNUM);
    //初始化bmap_hash,使得groupID对应bitmaps中的下标
    for (size_t i = 0; i < 20; i++)
        bmap_hash[groupids[i]] = i;
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);
    mode &= ~ENABLE_QUICK_EDIT_MODE;
    mode &= ~ENABLE_INSERT_MODE;
    mode &= ~ENABLE_MOUSE_INPUT;
    SetConsoleMode(hStdin, mode);
    std::string time = "202109";
    std::string group = "";
    std::string ordpath = "D:\\chen1ting\\StockRawData\\stockdata\\ord";
    std::string txnpath = "D:\\chen1ting\\StockRawData\\stockdata\\txn";
    std::string mktpath = "D:\\chen1ting\\StockRawData\\stockdata\\mkt";
    //1. 遍历文件夹,得到所有符合用户要求的日期及该日期下的合约
    if (argc == 5)
    {
        //input: string date; string ord_path; string txn_path; string mkt_path;
        //其中date 可以是 yyyy yyyymm yyyymmdd 三种格式。
        time = argv[1];
        ordpath = argv[2];
        txnpath = argv[3];
        mktpath = argv[4];
    }
    std::string dates[270];		    //所有需要检查的天数(一年中股票交易日最多250天左右)
    int dates_count = GetDates(time, ordpath, dates);
    for (size_t i = 0; i < dates_count; i++)
    {
        txn_read_ptr = 0;
        ord_read_ptr = 0;
        mkt_read_ptr = 0;
        txn_dealed = 0;  
        ord_dealed = 0;  
        memset(bitmaps, 0, sizeof(bitmaps));
        memset(trade_volumes, 0, sizeof(double) * NINSCOUNT);
        memset(sim_volumes, 0, sizeof(double) * NINSCOUNT);
        std::cout << "当前检查日期:" << dates[i] << std::endl;
        GetInsIds(ordpath + "\\" + dates[i]+group, txnpath + "\\" + dates[i]+group, mktpath + "\\" + dates[i]+group);
        auto start_timing = std::chrono::high_resolution_clock::now();
        CONSOLE_SCREEN_BUFFER_INFO cbsi;
        int line;
        if (GetConsoleScreenBufferInfo(hStdout, &cbsi))
            line = cbsi.dwCursorPosition.Y;
        std::thread progress_bar(ProgressBar, line);
        std::thread read_ord[ORD_SLOTS];
        std::thread read_txn[TXN_SLOTS];
        for (size_t read_p = 0; read_p < ORD_SLOTS; read_p++)
            read_ord[read_p] = std::thread(ReadOrd, ordpath + "\\" + dates[i] + group + "\\", &ordbufs[read_p], read_p + 1);
        for (size_t read_p = 0; read_p < TXN_SLOTS; read_p++)
            read_txn[read_p] = std::thread(ReadTxn, txnpath + "\\" + dates[i] + group + "\\", &txnbufs[read_p], read_p + 1);
        std::thread read_mkt1(ReadMkt, mktpath + "\\" + dates[i] + "\\");
        std::thread read_mkt2(ReadMkt, mktpath + "\\" + dates[i] + "\\");
        //std::thread deal_ord1(DealOrd, 0, ORD_SLOTS, 1);
        std::thread deal_ord1(DealOrd, 0, ORD_SLOTS / 2, 1);
        std::thread deal_ord2(DealOrd, ORD_SLOTS / 2, ORD_SLOTS, 2);
        std::thread deal_txn1(DealTxn, 0, TXN_SLOTS / 3, 1);
        std::thread deal_txn2(DealTxn, TXN_SLOTS / 3, TXN_SLOTS / 3 * 2, 2);
        std::thread deal_txn3(DealTxn, TXN_SLOTS / 3 * 2, TXN_SLOTS, 3);
        for (size_t read_p = 0; read_p < ORD_SLOTS; read_p++)
            read_ord[read_p].join();
        for (size_t read_p = 0; read_p < TXN_SLOTS; read_p++)
            read_txn[read_p].join();
        read_mkt1.join();
        read_mkt2.join();
        deal_ord1.join();
        deal_ord2.join();
        deal_txn1.join();
        deal_txn2.join();
        deal_txn3.join();
        progress_bar.join();

        if (CheckMissingOrd() && CheckTradeVol() && CheckWrongSeq())
            std::cout << std::endl << dates[i] << "check ok!";
        auto stop_timing = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop_timing - start_timing);
        std::cout << " Deal Time: " << duration.count() << std::endl;
    }
    for (size_t i = 0; i < ORD_SLOTS; i++)
        free(ordbufs[i].buf);
    for (size_t i = 0; i < TXN_SLOTS; i++)
        free(txnbufs[i].buf);
    return 0;
}
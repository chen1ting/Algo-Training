#include "HEADS.h"
/*
���project��Ҫ����ֺ��ļ��Ķ����Ƿ��������쳣:
1. order��trade����
2. order��tradeȱʧ(���кŶ�ʧ)
3. ��trade(�ܳɽ������)

�Ż�����:
1. д��bitmapʱ,20��λͼ(һ��λͼһ����)����һ������,Ч�ʽϵ�
2. ����order��txn�Ķ�����̹���һ������,�ǳ���������
*/


//һ. ������Լ,��ȡorder\trade\market�ļ�
std::string ins_file_names[NINSCOUNT];         //�洢������Ҫ���ĺ�Լ�ļ���
int ins_id_cnt = 0;                            //�洢������Ҫ���ĺ�Լ��
//1.��order
ORDBUFF ordbufs[ORD_SLOTS];                     //order�����, ���а���ORD_SLOTS��buffer, ÿ��buffer��������ĳһ��Լ���е�order_struct, ��¼�ú�Լ��order�����Ͷ�Ӧ�ļ�����ins_file_names�е�λ��
std::atomic<int> ord_read_ptr = 0;              //ָ��ord���һ���Ѿ���ȡ�ĺ�Լ
//2.��trade
TXNBUFF txnbufs[TXN_SLOTS];                     //txn�����,���а���TXN_SLOTS��buffer 
std::atomic<int> txn_read_ptr = 0;              //ָ��txn���һ���Ѿ���ȡ�ĺ�Լ
//3.��mkt
double trade_volumes[NINSCOUNT];                //mkt�ļ��ж�ȡ���ܳɽ���
std::mutex read_vol_mtx;                        //mkt readʱд��volume�ı�����
std::atomic<int> mkt_read_ptr = 0;              //ָ��mkt��һ������ȡ�ĺ�Լ

//�� ��鵥����Լ�ļ�
std::atomic<int> ord_dealed = 0;                //��¼�Ѽ�����ord����
std::atomic<int> txn_dealed = 0;                //��¼�Ѽ�����txn����
//1.������
std::unordered_map<int, std::vector<long long>> wrong_sequence_ord; //�洢����Ķ������
std::mutex wrong_ord_mtx;               //д��wrong_sequence_ord�ı�����
//2.bitmap����ȱʧ���
unsigned int bitmaps[20][BITMAPSIZE];           //���ڼ�¼ÿ������order+txnindex��λ��,����Ƿ����м䶪ʧ�Ķ���. һ��bitmap��һ������,20������Ҫ20��bitmap,��������λͼ�洢Ϊһ����ά����. 
std::unordered_map<int, int> bmap_hash;         //����ͨ��groupIDֱ���ҵ���Ӧλͼ��bitmaps�е�λ�� 1:0, 2:1, 3:2, 4:3, 5:4, 6:5, 20:6, 2011:7, 2012:8 ...
int groupids[20] = { 1, 2, 3, 4, 5, 6, 20,
2011, 2012, 2013, 2014,  2021, 2022, 2023, 2024,  2031, 2032, 2033, 2034, 2061 };//���ڳ�ʼ��bmap_hash
std::mutex bitmap_write_mtx;                    //bitmapд�뱣����
std::unordered_map<int, std::vector<long long>> missing_ord(20);//�洢ʧ�ٵĶ�����
//3.�ܳɽ������
double sim_volumes[NINSCOUNT];                  //�Ӻ͵õ����ܳɽ���
std::mutex txn_vol_mtx;                         //trade volumeд�뱣����

//�� ��������Ϣ
std::string groupids_string[20] = { "SH1", "SH2", "SH3", "SH4", "SH5", "SH6", "SH20",
"SZ2011", "SZ2012", "SZ2013", "SZ2014",  "SZ2021", "SZ2022", "SZ2023", "SZ2024",  "SZ2031", "SZ2032", "SZ2033", "SZ2034", "SZ2061" };
HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);//����̨���ʱ,ȷ�����ڵľ��
COORD destCoord;                                 //һ��ȫ�ֿ���cursor����

int main(int argc, char* argv[])
{
    for (size_t i = 0; i < ORD_SLOTS; i++)
        ordbufs[i].buf = (ORDER_STRUCT*)malloc(sizeof(ORDER_STRUCT) * MAXORDNUM);

    for (size_t i = 0; i < TXN_SLOTS; i++)
        txnbufs[i].buf = (TRADE_STRUCT*)malloc(sizeof(TRADE_STRUCT) * MAXORDNUM);
    //��ʼ��bmap_hash,ʹ��groupID��Ӧbitmaps�е��±�
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
    //1. �����ļ���,�õ����з����û�Ҫ������ڼ��������µĺ�Լ
    if (argc == 5)
    {
        //input: string date; string ord_path; string txn_path; string mkt_path;
        //����date ������ yyyy yyyymm yyyymmdd ���ָ�ʽ��
        time = argv[1];
        ordpath = argv[2];
        txnpath = argv[3];
        mktpath = argv[4];
    }
    std::string dates[270];		    //������Ҫ��������(һ���й�Ʊ���������250������)
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
        std::cout << "��ǰ�������:" << dates[i] << std::endl;
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
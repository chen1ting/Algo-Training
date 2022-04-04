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
constexpr char KZZ = 'k';         //��תծ���
constexpr char CYB = 'c';         //��ҵ����
constexpr char ZB = 'z';          //������
constexpr int LEVELDISPLAY = 10;  //����ӿ���ʾ���̿����
struct Level
{
    double Price;			             //�̿ڼ۸�
    double TotalVolume;		             //�̿��ܵ���
    std::deque<ORDER_STRUCT> OrdList;    //���̿ڵĶ�������
};
struct MktInfo
{
    double LstClose;      //����
    double TxnPauseBound[4]; //������ͣ�۸�����: 0-��30%ͣ�� 1-��20%ͣ�� 2-��20%ͣ�� 3-��30%ͣ��        
    double DynaHi;         //��Ч�۸�Χ����
    double DynaLo;         //��Ч�۸�Χ����
    double LimitHi;        //��ͣ����
    double LimitLo;        //��ͣ����
    double TodayOpen;      //��
    double TodayClose;     //����

    double NewestPrice;                   //���³ɽ���
    double PreNewestPrice;                //ǰ�ɽ���
    double BestBid;                       //��һ��
    double BestOffer;                     //��һ��
    double BidPrices[LEVELDISPLAY];       //���̼۸�
    double OfferPrices[LEVELDISPLAY];     //���̼۸�
    double BidVolumes[LEVELDISPLAY];      //������
    double OfferVolumes[LEVELDISPLAY];    //������

    int OrdsCount;              //���ܵ���
    int TxnCount;               //�����ܵ���
    double TotalTradeVol;       //�ɽ�������
    double TotalTradeNumber;    //�ɽ��ܵ���
    double TotalVol;            //����������

    bool StopTrade;             //true: ͣ��30����; 
    bool FirstTimeRiseStop;     //true: ��һ����20%������ͣ
    bool FirstTimeFallStop;     //true: ��һ�ε�20%������ͣ
    char StopTradeUntil[13];    //��ͣ������
    long long TxnBizIdx;        //�ɽ���BizIndex
};
class MarketManager
{
public:
    //MarketManager();        //��תծ����
    MarketManager(char a);  //��ҵ�彻��
    //MarketManager(int a);   //���彻��
    ~MarketManager();
    void GetAllOrd(fs::path ordfilepath);           //�������ж������浽order_buffer���ʼ��current_ordָ���һ������
    void GetAllTxn(fs::path txnfilepath);           //�������ж������浽txn_buffer���ʼ��current_txnָ���һ������ 
    void GetLstClosePrice(fs::path mktfilepath);    //��ȡ���ռۣ�������Ч���۷�Χ���ĵ���ͣ�۸�
    void StartTrade();                              //��ʼ��תծ���� - �൱��MarketManager��main����
    //void StartTrade(char a);                        //��ʼ��ҵ���Ʊ���� - �൱��MarketManager��main����
    //void StartTrade(int a);                         //��ʼ�����Ʊ���� - �൱��MarketManager��main����
    //�������
    void PrintTrades();                             //��ӡ�ɽ���
    void PrintMktInfo();                            //��ӡ�̿���Ϣ
    MktInfo* ReturnMktInfo();                       //���ذ���ʵʱMkt��Ϣ��MktInfo�ṹ��
    void Trades2File();                             //����ɽ�����csv
    void Levels2File(std::string filename);         //�����ǰ�̿ڵ�csv
    bool CheckSimCorrectness();                     //�Աȵ����ϵĳɽ�������ʵ�ɽ���

    MktInfo Mkt = { 0 };                            //�����г���Ϣ������ʱ��ʼ��
    bool FileOpenUnsuccesssful = false;             //true: ���ļ�δ����ȷ��

private:
    void CallAuction(char mode);                                     //���Ͼ���,mode������ǿ��̡����ơ����̼��Ͼ��۵�
    void ContBid();                                 //�����������۵ĺ���
    //���뵱ǰ��ϵ��ʹ�϶��У�����϶��б�ƽ����ɾ�����̿ڣ�����һ����ƽ���򷵻أ��������¼�
    bool MakeOneTrade(ORDER_STRUCT* curord, Level* match_lev);           //�ɽ���������,���match_lev��ƽ��ɾ��,����true
    void HangOrd(ORDER_STRUCT someord, char mode, bool new_ord);         //������Ķ�����mode = BUYFLAG�����̣�mode = SELLFLAG�����̣�������һ��һ; new_ordΪ��,������һ�ν����̿�,����price_index�Ĺ�ϣ��
    void DealDroppedOrder(long long idx);                                //�ҵ���ǰOrderIndex֮ǰ���еĳ�����call CancelOrd()���д���
    void CancelOrd(long long idx, double price, double vol, char mode);  //�ҵ�����������
    void DeleteLevel(double price, char mode);                           //ɾ���̿ڣ�������һ��һ
    inline bool IsValidPrice(double Price);                              //true:��Ч����                            //true:��Ч����
    inline void UpdateBound(double Price);                               //���ݴ���۸�reset���۷�Χ��Mkt.DynaHi & Mkt.DynaLo)
    inline void UpdateBestOffer();                                       //������һ��Mkt.BestOffer)
    inline void UpdateBestBid();                                         //������һ��(Mkt.BestBid)
    void UpdateMktInfo();                                                //������һ��(Mkt.BestBid)
    bool Time1Bigger(char* time1, char* time2);  //true: time1 > time2; false: time1 <= time2
    bool CheckStopTrade();                       //����Ƿ���Ҫ��ͣ�������г�״̬

    char STOCKTYPE;                         //��ǽ�������
    float MINGAP;                           //�۸���С�䶯��Χ
    double TRADEPB;                         //Trade Price Bound���������ۡ����̼��Ͼ��۵ļ۸��ǵ�������
    double TEMPSTOP1;                       //��ͣ�۸�1��ͣ30min
    double TEMPSTOP2;                       //��ͣ�۸�2��ͣ��14��57
    double LIMIT;                           //�ǵ������ƣ�ͣ��14��57
    ORDER_STRUCT* order_buffer;             //���ж�����
    TRADE_STRUCT* txn_buffer;               //���н��׳�
    ORDER_STRUCT* current_ord;              //ָ��ǰ���ڴ���Ķ���
    int ord_counter;
    TRADE_STRUCT* current_txn;              //ָ����һ�����׵�
    int txn_counter;
    std::vector<TRADE_STRUCT> trades;       //��ϵĳɽ�������
    std::vector<TRADE_STRUCT> real_trades;  //��ʵ�ĳɽ�������
    std::map<double, Level> offers_order_book;      //����
    std::map<double, Level> bids_order_book;        //����
    std::map<long long, double> idx2price;  //index�ͼ۸�һһ��Ӧ�Ĺ�ϣ��
};

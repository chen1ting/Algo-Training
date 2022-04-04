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
constexpr int LEVELDISPLAY = 10;        //����ӿ���ʾ���̿����
constexpr int SH = 606890;              //�Ϻ� ���������ֱ�ʾ
static char SZ = 'z';                   //���� ���ַ���ʾ

struct Level
{
    //LevelType Type;			         //�̿�����
    double Price;			             //�̿ڼ۸�
    double TotalVolume;		             //�̿��ܵ���
    std::unordered_set<long long> OrdIDX;    //���̿ڵĶ�������
};
struct MktInfo
{
    double LstClose;      //����
    double TodayOpen;     //��
    double TodayClose;    //����

    double NewestPrice;                   //���³ɽ���
    double BestOffer;                     //�г���һ��
    double BestBid;                       //�г���һ��
    double BidPrices[LEVELDISPLAY];       //���̼۸�
    double BidVolumes[LEVELDISPLAY];      //������
    double OfferPrices[LEVELDISPLAY];     //���̼۸�
    double OfferVolumes[LEVELDISPLAY];    //������

    int OrdsCount;                        //���ܵ���
    int TxnCount;                         //�����ܵ���
    double TotalTradeVol;                 //�ɽ�������
    double TotalTradeNumber;              //�ɽ��ܵ���
    double TotalVol;                      //����������
};
class MarketSimulator
{
public:
    MarketSimulator();                              //��ʼ�����г�Ա
    ~MarketSimulator();
    void GetAllOrd(fs::path ordfilepath);           //�������ж������浽order_buffer���ʼ��current_ordָ���һ������,����Ǵ�ҵ���Ʊ,��ʼ��PriceBound
    void GetAllTxn(fs::path txnfilepath);           //�������ж������浽txn_buffer���ʼ��current_txnָ���һ������ 
    void GetLstClosePrice(fs::path txnfilepath);    //��ȡ���ռۣ�������Ч���۷�Χ���ĵ���ͣ�۸�
    void Start(int sh);                             //�Ͻ�������������:������������\�񿪽���\ģ��ʵʱ�̿�ģ��
    void Start(char sz);                            //�������������:������������\�񿪽���\ģ��ʵʱ�̿�ģ��
    //�������
    void PrintMktInfo();                            //��ӡ�̿���Ϣ
    inline MktInfo* ReturnMktInfo();                //���ذ���ʵʱMkt��Ϣ��MktInfo�ṹ��
    bool CheckCorrectness(fs::path mktfilepath);    //������һ���̿���Ϣ�Ƿ���ȷ
    MktInfo Mkt = { 0 };                            //�����г���Ϣ�Ľṹ�壬����ʱ��ʼ��
    bool FileOpenUnsuccesssful = false;             //true: ���ļ�δ����ȷ��

private:
    void HangOrd();         //������Ķ�����mode = BUYFLAG�����̣�mode = SELLFLAG�����̣�������һ��һ; new_ordΪ��,������һ�ν����̿�,����price_index�Ĺ�ϣ��
    void DealTrades(long long idx, int sh);                //�ҵ���ǰOrderIndex֮ǰ���еĳ�����call CancelOrd()���д���
    void DealTrades(long long idx, char sz);               //�ҵ���ǰOrderIndex֮ǰ���еĳ�����call CancelOrd()���д���
    void DeleteOrd(long long idx, double volume);          //�ҵ���ɾ������
    void UpdateMktInfo();                                  //������һ��(Mkt.BestBid)
    inline bool Time1Bigger(char* time1, char* time2);     //true: time1 > time2; false: time1 <= time2
    inline bool IsValidPrice(double price, char mode);
    inline void UpdateBestBid();
    inline void UpdateBestOffer();

    ORDER_STRUCT* order_buffer;             //���ж�����
    TRADE_STRUCT* txn_buffer;               //���н��׳�
    ORDER_STRUCT* current_ord;              //ָ��ǰ���ڴ���Ķ���
    int ord_counter;
    TRADE_STRUCT* current_txn;              //ָ����һ�����׵�
    int txn_counter;
    std::map<double, Level> bids_book;                      //��������
    std::map<double, Level> offers_book;                    //��������
    std::unordered_map<long long, ORDER_STRUCT> all_ords;   //����OrderIndex���Ҷ����Ķ�����
    std::map<double, Level>* book;

    char BUYFLAG;          //��Function Code
    char SELLFLAG;         //����Function Code
    double PRICEBOUND;     //��Ч���۷�Χ
};

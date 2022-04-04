#include "MARKET_SIMULATOR.h"
constexpr float MINGAP = 0.01;          //�۸���С�䶯��Χ
static char NINE_TWENTY_FIVE_AM[13] = "09:25:00.000";
static char NINE_THIRTY_AM[13] = "09:30:00.000";
static char TWELVE[13] = "12:00:00.000";
static char TWO_FIFTY_SEVEN_PM[13] = "14:57:00.000";
static char THREE_PM[13] = "15:00:01.999";

MarketSimulator::MarketSimulator()
{
    ord_counter = 0;
    txn_counter = 0;
    PRICEBOUND = DBL_MAX;    
    Mkt.BestOffer = DBL_MAX;
}
MarketSimulator::~MarketSimulator()
{
    free(order_buffer);
    free(txn_buffer);
}
void MarketSimulator::GetAllOrd(fs::path ordfilepath)
{
    std::ifstream order_input;
    order_input.open(ordfilepath, std::ios::in | std::ios::binary);

    if (!order_input.is_open() || order_input.tellg() % sizeof(ORDER_STRUCT) != 0)
    {
        FileOpenUnsuccesssful = true;
        std::cout << ordfilepath << "open/read unsuccessful." << std::endl;
        return;
    }
    order_input.seekg(0, std::ios::end);
    Mkt.OrdsCount = order_input.tellg() / sizeof(ORDER_STRUCT); //��ʼ��Count,���ڼ�¼��������ݷ�Χ
    order_buffer = (ORDER_STRUCT*)malloc(sizeof(ORDER_STRUCT) * (Mkt.OrdsCount)); //��ʼ��buffer
    order_input.seekg(0, std::ios::beg);
    order_input.read((char*)order_buffer, sizeof(ORDER_STRUCT) * (Mkt.OrdsCount));
    current_ord = order_buffer;
    if (order_buffer->InstrumentID[0] == '3')//���ڴ�ҵ��
        PRICEBOUND = 0.02;  //�ٷ�֮������Ч���۷�Χ
}
void MarketSimulator::GetAllTxn(fs::path txnfilepath)
{
    std::ifstream trade_input;
    trade_input.open(txnfilepath, std::ios::in | std::ios::binary);

    if (!trade_input.is_open() || trade_input.tellg() % sizeof(TRADE_STRUCT) != 0)
    {
        FileOpenUnsuccesssful = true;
        std::cout << txnfilepath << "open/read unsuccessful." << std::endl;
        return;
    }
    trade_input.seekg(0, std::ios::end);
    Mkt.TxnCount = trade_input.tellg() / sizeof(TRADE_STRUCT); //��ʼ��Count,���ڼ�¼��������ݷ�Χ
    txn_buffer = (TRADE_STRUCT*)malloc(sizeof(TRADE_STRUCT) * (Mkt.TxnCount)); //��ʼ��buffer
    trade_input.seekg(0, std::ios::beg);
    trade_input.read((char*)txn_buffer, sizeof(TRADE_STRUCT) * (Mkt.TxnCount));
    current_txn = txn_buffer;
}
void MarketSimulator::GetLstClosePrice(fs::path txnfilepath)
{
    std::ifstream LstTxnIn;
    LstTxnIn.open(txnfilepath, std::ios::in | std::ios::binary);
    if (!LstTxnIn.is_open() || LstTxnIn.tellg() % sizeof(ORDER_STRUCT) != 0)
    {
        FileOpenUnsuccesssful = true;
        std::cout << txnfilepath << " read/open unsuccessful." << std::endl;
        return;
    }
    TRADE_STRUCT lst_txn;
    LstTxnIn.seekg(0, std::ios::end);
    int count = LstTxnIn.tellg() / sizeof(TRADE_STRUCT);
    LstTxnIn.seekg((count - 1) * sizeof(TRADE_STRUCT), std::ios::beg);    //LstMktIn.seekg(0 - sizeof(MARKET_STRUCT), std::ios::end);
    LstTxnIn.read((char*)&lst_txn, sizeof(TRADE_STRUCT));
    Mkt.LstClose = lst_txn.Price;
}
void MarketSimulator::Start(int sh)
{
    BUYFLAG = 'B';
    SELLFLAG = 'S';
    //�ӽ�����Ϣ�л�ȡ�񿪺ͽ���(���Ͼ����޳ɽ�,���̼���ʲô? -  �˴�ѡ�����ռ�)
    Mkt.TodayOpen = Time1Bigger(NINE_THIRTY_AM, current_txn->TradeTime) ? txn_buffer->Price : Mkt.LstClose;
    Mkt.TodayClose = !Time1Bigger((current_txn + Mkt.TxnCount - 1)->TradeTime, TWO_FIFTY_SEVEN_PM) ? (txn_buffer + Mkt.TxnCount - 1)->Price : Mkt.NewestPrice;
    //ʵʱ�̿���Ϣ����
    for (; ord_counter < Mkt.OrdsCount; ord_counter++, current_ord++)
    {
        DealTrades(current_ord->BizIndex, sh);
        if (current_ord->OrderKind == 'D') //�Ϻ�����
            DeleteOrd(current_ord->OrderIndex, current_ord->Volume);
        else
            HangOrd();
    }
    DealTrades((txn_buffer + Mkt.TxnCount - 1)->BizIndex, sh);
}
void MarketSimulator::DealTrades(long long idx, int sh)
{
    if (txn_counter == Mkt.TxnCount || current_txn->BizIndex > idx)
        return;
    while (current_txn->BizIndex <= idx)
    {
        Mkt.NewestPrice = current_txn->Price;
        Mkt.TotalTradeNumber++;
        Mkt.TotalTradeVol += current_txn->Volume;
        DeleteOrd(current_txn->SellIndex, current_txn->Volume);
        DeleteOrd(current_txn->BuyIndex, current_txn->Volume);
        if (++txn_counter >= Mkt.TxnCount)
            break;
        current_txn++;
    }
}

void MarketSimulator::Start(char sz)
{    //�ӽ�����Ϣ�л�ȡ�񿪺ͽ���
    BUYFLAG = '1';
    SELLFLAG = '2';
    auto open_txn_finder = current_txn;
    while (open_txn_finder->FunctionCode == '4')
        open_txn_finder++;
    Mkt.TodayOpen = open_txn_finder->Price;
    auto close_txn_finder = (txn_buffer + Mkt.TxnCount - 1);
    while (close_txn_finder->FunctionCode == '4')
        close_txn_finder--;
    Mkt.TodayClose = close_txn_finder->Price;
    //ʵʱ�̿���Ϣ����
    double pre_volume = 0;
    double cur_volume;
    for (; ord_counter < Mkt.OrdsCount; ord_counter++, current_ord++)
    {
        //if (Time1Bigger(current_ord->OrderTime, (char*)"14:21:56.999"))
        //    std::cout << current_ord->OrderIndex << std::endl;
        //if (current_ord->OrderIndex == 35818489)
        //    std::cout << current_ord->OrderIndex << std::endl;
        DealTrades(current_ord->OrderIndex, sz);
        if (current_ord->OrderKind == 'U')  //���������걨
            current_ord->Price = current_ord->FunctionCode == BUYFLAG? Mkt.BestBid : Mkt.BestOffer;
        if (current_ord->OrderKind == '1')  //�Է������걨
            current_ord->Price = current_ord->FunctionCode == BUYFLAG ? Mkt.BestOffer : Mkt.BestBid;
        HangOrd();
        //auto find_14 = bids_book.find(14.0);
        //if (find_14 != bids_book.end())
        //{
        //    if (pre_volume == 0)
        //    {
        //        pre_volume = find_14->second.TotalVolume;
        //        std::cout << current_ord->OrderTime << " " << pre_volume << std::endl;
        //    }
        //    cur_volume = find_14->second.TotalVolume;
        //    if (cur_volume != pre_volume)
        //    {
        //        std::cout << current_ord->OrderTime << " " << cur_volume << std::endl;
        //        pre_volume = cur_volume;
        //    }
        //}
    }
    DealTrades((txn_buffer + Mkt.TxnCount - 1)->TradeIndex, sz);
}
void MarketSimulator::DealTrades(long long idx, char sz)
{
    if (txn_counter == Mkt.TxnCount || current_txn->TradeIndex > idx)
        return;
    while (current_txn->TradeIndex <= idx)
    {
        if (current_txn->FunctionCode != '4')
        {
            Mkt.NewestPrice = current_txn->Price;
            Mkt.TotalTradeNumber++;
            Mkt.TotalTradeVol += current_txn->Volume;
            DeleteOrd(current_txn->SellIndex, current_txn->Volume);
            DeleteOrd(current_txn->BuyIndex, current_txn->Volume);
        }
        else
            DeleteOrd(current_txn->SellIndex == 0 ? current_txn->BuyIndex : current_txn->SellIndex, current_txn->Volume);
        if (++txn_counter >= Mkt.TxnCount)
            break;
        current_txn++;
    }
}

void MarketSimulator::HangOrd()
{
    all_ords.insert({ current_ord->OrderIndex, *current_ord });
    book = current_ord->FunctionCode == BUYFLAG ? &bids_book : &offers_book;
    auto it = book->find(current_ord->Price);
    if (it == book->end())
    {
        std::unordered_set<long long> temp_set;
        temp_set.insert(current_ord->OrderIndex);
        Level temp_lev = { current_ord->Price,current_ord->Volume, temp_set };
        book->insert({ current_ord->Price, temp_lev });
        //������һ����һ�۸�
        if (current_ord->FunctionCode == BUYFLAG)
        {
            if (PRICEBOUND != DBL_MAX)
            {
                if (current_ord->Price >= Mkt.BestBid && IsValidPrice(current_ord->Price, BUYFLAG))
                    Mkt.BestBid = current_ord->Price;
            }
            else if (current_ord->Price >= Mkt.BestBid)
                Mkt.BestBid = current_ord->Price;
        }
        else
        {
            if (PRICEBOUND != DBL_MAX)
            {
                if (current_ord->Price <= Mkt.BestOffer && IsValidPrice(current_ord->Price, SELLFLAG))
                    Mkt.BestOffer = current_ord->Price;
            }
            else if (current_ord->Price <= Mkt.BestOffer)
                Mkt.BestOffer = current_ord->Price;
        }
    }
    else
    {
        it->second.TotalVolume += current_ord->Volume;
        it->second.OrdIDX.insert(current_ord->OrderIndex);
    }
}
void MarketSimulator::DeleteOrd(long long idx, double volume)
{
    auto it = all_ords.find(idx);
    if (it == all_ords.end())
        return;
    double price = it->second.Price;
    char flag = it->second.FunctionCode;
    it->second.Volume -= volume;
    book = flag == BUYFLAG ? &bids_book : &offers_book;
    auto cur_level = book->find(price);
    if (cur_level == book->end())
        return;
    if (it->second.Volume <= 0)
    {
        cur_level->second.OrdIDX.erase(idx);
        all_ords.erase(it);
    }
    if (cur_level->second.OrdIDX.empty())
    {
        book->erase(cur_level);
        if (PRICEBOUND != DBL_MAX)
        {
            if (flag == BUYFLAG && price == Mkt.BestBid)    //����������̿ڱ�ɾ��,�ᵼ�����̸���
            {
                UpdateBestBid();
                UpdateBestOffer();
            }
            else if(flag == SELLFLAG && price == Mkt.BestOffer)
            {
                UpdateBestOffer();
                UpdateBestBid();
            }
        }
        else //���彻��,��������Ч������ı���
        {
            if (flag == BUYFLAG && price == Mkt.BestBid)
                Mkt.BestBid = bids_book.empty()? 0.0 : bids_book.rbegin()->first;
            else if (flag == SELLFLAG && price == Mkt.BestOffer)
                Mkt.BestOffer = offers_book.empty()? DBL_MAX : offers_book.begin()->first;
        }
        return;
    }
    cur_level->second.TotalVolume -= volume;
}
void MarketSimulator::UpdateMktInfo()
{
    int i = 0;
    for (auto it = bids_book.find(Mkt.BestBid); it != --bids_book.begin(); --it )
    {
        Mkt.BidPrices[i] = it->first;
        Mkt.BidVolumes[i] = it->second.TotalVolume;
        if (i++ >= 9)
            break;
    }
    while (i < LEVELDISPLAY)
    {
        Mkt.BidPrices[i] = DBL_MAX;
        Mkt.BidVolumes[i] = DBL_MAX;
        i++;
    }
    i = 0;
    for (auto it = offers_book.find(Mkt.BestOffer); it != offers_book.end(); ++it)
    {
        Mkt.OfferPrices[i] = it->first;
        Mkt.OfferVolumes[i] = it->second.TotalVolume;
        if (i++ >= 9)
            break;
    }
    while (i < LEVELDISPLAY)
    {
        Mkt.OfferPrices[i] = DBL_MAX;
        Mkt.OfferVolumes[i] = DBL_MAX;
        i++;
    }
}
void MarketSimulator::PrintMktInfo()
{
    UpdateMktInfo();
    std::cout << order_buffer->InstrumentID << "\t";
    if (ord_counter < Mkt.OrdsCount)
        std::cout << current_ord->OrderTime << std::endl;
    std::cout << "���գ�" << Mkt.LstClose << "\t"
        << "�񿪣�" << Mkt.TodayOpen << "\t"
        << "���գ�" << Mkt.TodayClose << std::endl
        << "���¼�: " << Mkt.NewestPrice << "\t"
        << "�ܳɽ�������" << Mkt.TotalTradeVol << "\t"
        << "�ܳɽ�������" << Mkt.TotalTradeNumber << std::endl;
    for (int i = 0; i < LEVELDISPLAY; i++)
    {
        std::cout << "��" << i + 1 << "��: " << Mkt.BidPrices[i] << "\t" <<
            "��" << i + 1 << "��: " << Mkt.BidVolumes[i] << "\t" <<
            "��" << i + 1 << "��: " << Mkt.OfferPrices[i] << "\t" <<
            "��" << i + 1 << "��: " << Mkt.OfferVolumes[i] << std::endl;
    }
    std::cout << std::endl;
}
inline MktInfo* MarketSimulator::ReturnMktInfo()
{
    UpdateMktInfo();
    return &Mkt;
}
bool MarketSimulator::CheckCorrectness(fs::path mktfilepath)
{
    std::ifstream LstMktIn;
    LstMktIn.open(mktfilepath, std::ios::in | std::ios::binary);
    if (!LstMktIn.is_open() || LstMktIn.tellg() % sizeof(ORDER_STRUCT) != 0)
    {
        FileOpenUnsuccesssful = true;
        std::cout << mktfilepath << " read/open unsuccessful." << std::endl;
        return true;
    }

    LstMktIn.seekg(0, std::ios::end);
    int count = LstMktIn.tellg() / sizeof(MARKET_STRUCT);
    MARKET_STRUCT lst_mkt;
    if (order_buffer->InstrumentID[0] == '6' && order_buffer->InstrumentID[1] == '8')
    { //�Ϻ��ƴ����Ʊ���г����һ��ָ��������֮��۸�Ϊ0��ǰһ��
        MARKET_STRUCT* mkt_buffer = (MARKET_STRUCT*)malloc(sizeof(MARKET_STRUCT) * count);
        LstMktIn.seekg(0, std::ios::beg);
        LstMktIn.read((char*)mkt_buffer, sizeof(MARKET_STRUCT) * (count));
        MARKET_STRUCT* mkt_ptr = mkt_buffer + count - 1;
        for (int i = count - 1; i >= 0; i--) //�Ӻ���ǰ����
        {
            if (mkt_ptr->OfferPrice2 != DBL_MAX || mkt_ptr->BidPrice2 != DBL_MAX)
            {
                lst_mkt = *mkt_ptr;
                break;
            }
            if (Time1Bigger(THREE_PM, (--mkt_ptr)->TimeStamp)) //ǰһ��������������Ļ����˳�
                break;
        }
        free(mkt_buffer);
    }
    else
    {
        LstMktIn.seekg((count - 1) * sizeof(MARKET_STRUCT), std::ios::beg);    //LstMktIn.seekg(0 - sizeof(MARKET_STRUCT), std::ios::end);
        LstMktIn.read((char*)&lst_mkt, sizeof(MARKET_STRUCT));
    }
    
    UpdateMktInfo();
    double* real_ptr;
    double* sim_ptr = &Mkt.BidPrices[0];
    for (int i = 0; i < 5; i++)
    {
        if (i == 2) { continue; }
        real_ptr = &lst_mkt.BidPrice1 + i; //0:BidPrices1 1:BidVolume1 2:BidCount1 3:OfferPrice1 4:OfferVolume1
        for (int j = 0; j < LEVELDISPLAY; j++, real_ptr += 6, sim_ptr += 1)
        {
            if (*real_ptr == 0)
                *real_ptr = DBL_MAX;
            if (*real_ptr != *sim_ptr)
            {
                std::cout << i << ":\t��ʵ:" << *real_ptr << "\tģ��:" << *sim_ptr << std::endl;
                PrintMktInfo();
                return false;
            }
        }
    }                        
    return true;
}                                                             
inline bool MarketSimulator::Time1Bigger(char* time1, char* time2)
{
    for (int i = 0; i < 12; i++)
    {
        if (time1[i] > time2[i])
            return true;
        else if (time1[i] < time2[i])
            return false;
    }
    return false; //Ĭ�ϵ���Ϊfalse,���ϸ���ڲ�Ϊ��
}
inline bool MarketSimulator::IsValidPrice(double price, char mode)
{
    /* ���ڴ�ҵ����Ч���۷�Χ:
    ��һ�������걨�۸�<=102%��һ��
    �����������걨�۸�>=98%��һ�� 
    �޼�ʱ��ʾ�����������������룩�걨�۸�ģ�Ϊ��ʱ��ʾ��������루����������걨�۸�
    �޼�ʱ��ʾ��������루����������걨�۸�ģ�Ϊ����ɽ��ۣ�
    �����޳ɽ��ģ�Ϊǰ���̼ۡ�
    �����ڼ���ʱͣ�ƽ׶ε��޼��걨��������ǰ����涨��
    */
    double cpr_price;
    if (mode == BUYFLAG)
    {
        cpr_price = Mkt.BestOffer == DBL_MAX ? Mkt.BestBid : Mkt.BestOffer;
        cpr_price = cpr_price == 0.0 ? Mkt.NewestPrice : cpr_price;
        cpr_price = cpr_price == 0.0 ? Mkt.LstClose : cpr_price;
        return price <= (round(cpr_price * (1 + PRICEBOUND) * 100.0) / 100.0 + 0.0001);
    }
    else
    {
        cpr_price = Mkt.BestBid == 0.0 ? Mkt.BestOffer : Mkt.BestBid;
        cpr_price = cpr_price == DBL_MAX ? Mkt.NewestPrice : cpr_price;
        cpr_price = cpr_price == 0.0 ? Mkt.LstClose : cpr_price;
        return price >= (round(cpr_price * (1 - PRICEBOUND) * 100.0) / 100.0 - 0.0001);
    }
}
inline void MarketSimulator::UpdateBestBid()
{
    if (bids_book.empty() || !IsValidPrice(bids_book.begin()->first, BUYFLAG))
    {
        Mkt.BestBid = 0.0;
        return;
    }
    auto best_bid_finder = bids_book.rbegin();
    while (best_bid_finder != --bids_book.rend() && !IsValidPrice(best_bid_finder->first, BUYFLAG))
        best_bid_finder++;
    Mkt.BestBid = best_bid_finder->first;
}
inline void MarketSimulator::UpdateBestOffer()
{
    if (offers_book.empty() || !IsValidPrice(offers_book.rbegin()->first, SELLFLAG))
    {
        Mkt.BestOffer = DBL_MAX;
        return;
    }
    auto best_offer_finder = offers_book.begin();
    while (best_offer_finder != --offers_book.end() && !IsValidPrice(best_offer_finder->first, SELLFLAG))
        best_offer_finder++;
    Mkt.BestOffer = best_offer_finder->first;
}
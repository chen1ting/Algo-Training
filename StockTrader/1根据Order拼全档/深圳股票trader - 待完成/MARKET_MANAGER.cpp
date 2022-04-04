#include "MARKET_MANAGER.h"
constexpr double MAX = 10000000.0;      //BestOffer�ĳ�ʼֵ
constexpr double MIN = -1.0;            //�м�۸�BestBid�ĳ�ʼֵ
constexpr char BUYFLAG = '1';           //��Function Code
constexpr char SELLFLAG = '2';          //����Function Code
constexpr char OPENAUC = 'M';           //���̼��Ͼ���
constexpr char STOPAUC = 'C';           //��ͣ���Ƶļ��Ͼ���
constexpr char CLOSEAUC = 'O';          //���̼��Ͼ���
static char NINE_TWENTY_FIVE_AM[13] = "09:25:00.000";
static char TWELVE[13] = "12:00:00.000";
static char TWO_FIFTY_SEVEN_PM[13] = "14:57:00.000";
static char THREE_PM[13] = "15:00:00.000";
inline bool operator<(const ORDER_STRUCT& a, const ORDER_STRUCT& b) { return a.OrderIndex < b.OrderIndex; }

MarketManager::MarketManager(char mode)
{
    //��ʼ��Mkt�ĳ�Ա
    Mkt.TodayOpen = MIN;
    Mkt.TodayClose = MIN;
    Mkt.BestOffer = MAX;
    ord_counter = 0;
    txn_counter = 0;
    Mkt.FirstTimeRiseStop = true;
    Mkt.FirstTimeFallStop = true;
    STOCKTYPE = mode;
    switch (mode)
    {
    KZZ:
        MINGAP = 0.001;
        TRADEPB = 0.1;
        TEMPSTOP1 = 0.2;
        TEMPSTOP2 = 0.3;
        LIMIT = MAX;
        break;
    CYB:
        MINGAP = 0.01;
        TRADEPB = 0.02;
        TEMPSTOP1 = 0.3;
        TEMPSTOP2 = 0.6;
        LIMIT = 0.2;
        break;
    default:    //Ĭ��Ϊ�����Ʊ����
        MINGAP = 0.01;
        TRADEPB = MAX;
        TEMPSTOP1 = MAX;
        TEMPSTOP2 = MAX;
        LIMIT = 0.1;
        break;
    }
}
MarketManager::~MarketManager()
{
    free(order_buffer);
    free(txn_buffer);
}
void MarketManager::GetAllOrd(fs::path ordfilepath)
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
}
void MarketManager::GetAllTxn(fs::path txnfilepath)
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
void MarketManager::GetLstClosePrice(fs::path mktfilepath)
{
    std::ifstream LstMktIn;
    LstMktIn.open(mktfilepath, std::ios::in | std::ios::binary);
    if (!LstMktIn.is_open() || LstMktIn.tellg() % sizeof(ORDER_STRUCT) != 0)
    {
        FileOpenUnsuccesssful = true;
        std::cout << mktfilepath << " read/open unsuccessful." << std::endl;
        return;
    }

    MARKET_STRUCT lst_mkt;
    LstMktIn.seekg(0, std::ios::end);
    int count = LstMktIn.tellg() / sizeof(MARKET_STRUCT);
    LstMktIn.seekg((count - 1) * sizeof(MARKET_STRUCT), std::ios::beg);    //LstMktIn.seekg(0 - sizeof(MARKET_STRUCT), std::ios::end);
    LstMktIn.read((char*)&lst_mkt, sizeof(MARKET_STRUCT));

    Mkt.LstClose = lst_mkt.ClosePrice;
    if (STOCKTYPE == KZZ)
    {
        Mkt.TxnPauseBound[0] = round(Mkt.LstClose * (1 - TEMPSTOP2) * 1000.0) / 1000.0 + 0.00001;
        Mkt.TxnPauseBound[1] = round(Mkt.LstClose * (1 - TEMPSTOP1) * 1000.0) / 1000.0 + 0.00001;
        Mkt.TxnPauseBound[2] = round(Mkt.LstClose * (1 + TEMPSTOP1) * 1000.0) / 1000.0 - 0.00001;
        Mkt.TxnPauseBound[3] = round(Mkt.LstClose * (1 + TEMPSTOP2) * 1000.0) / 1000.0 - 0.00001;
        Mkt.LimitHi = MAX;
        Mkt.LimitLo = MIN;
        UpdateBound(Mkt.LstClose);
    }
    else
    {
        Mkt.LimitHi = round(Mkt.LstClose * (1 + LIMIT) * 100.0) / 100.0 + 0.0001;
        Mkt.LimitLo = round(Mkt.LstClose * (1 - LIMIT) * 100.0) / 100.0 + 0.0001;
    }
}
void MarketManager::StartTrade()
{
    //����ʱ��С�ڵ���9:25��ֱ�ӹ���
    while (Time1Bigger(NINE_TWENTY_FIVE_AM, current_ord->OrderTime) && ord_counter < Mkt.OrdsCount)
    {
        HangOrd(*current_ord, current_ord->FunctionCode, true);
        current_ord++;
        ord_counter++;
    }
    //�˳�whileѭ����current_ordʱ�����9:25.�ȼ��д���һ�θö���֮ǰ�ĳ������ټ��Ͼ���
    DealDroppedOrder((current_ord - 1)->OrderIndex);
    CallAuction(OPENAUC);
    //PrintMktInfo();
    //����ʱ��С��14:57,��������
    while (Time1Bigger(TWO_FIFTY_SEVEN_PM, current_ord->OrderTime) && ord_counter < Mkt.OrdsCount)
    {
        if (Mkt.StopTrade == true) //���������ͣ�׶�
        {//���ҵ�ǰ����ʱ��С�ڵ�����ͣ����ʱ��,ֱ�ӹ���
            if (!Time1Bigger((current_ord+1)->OrderTime, Mkt.StopTradeUntil) && (current_ord + 1)->OrderIndex - current_ord->OrderIndex <= 150)
                HangOrd(*current_ord, current_ord->FunctionCode, true);
            else //��ͣ�ڼ��յ������һ������
            {
                DealDroppedOrder(current_ord ->OrderIndex);         //�ȴ���ö���֮ǰ�����е���
                HangOrd(*current_ord, current_ord->FunctionCode, true);   //�ٰ������������
                CallAuction(STOPAUC);   //���Ƽ��Ͼ���
                Mkt.StopTrade = false;  //������ͣ״̬
            }
        }
        else   //������������
        {
            DealDroppedOrder(current_ord->OrderIndex);  //�ȳ���
            ContBid();  //����������
        }
        current_ord++;  //ָ���Ƶ���һ������
        ord_counter++;
    }
    //�˳�ǰһ��whileѭ���ĵ�һ������ʱ��>=14:57
    if (Mkt.StopTrade == true) //����г�״̬����ͣ,˵���ö�������ͣ�׶��µĵ���,Ӧ����
    {
        while (!Time1Bigger(current_ord->OrderTime, TWO_FIFTY_SEVEN_PM)) //���𶩵�ʱ��<=14:57�ĵ���
        {
            HangOrd(*current_ord, current_ord->FunctionCode, true);
            current_ord++;
            ord_counter++;
        }
        //����whileѭ���ĵ�һ������ʱ��>14:57,�����̼��Ͼ��۵ĵ���
        DealDroppedOrder((txn_buffer + (Mkt.TxnCount - 1))->TradeIndex); //����õ���֮ǰ�ĳ���
        CallAuction(STOPAUC);   //���и��Ƽ��Ͼ���
        Mkt.StopTrade = false;  //������ͣ״̬
    }
    while (ord_counter < Mkt.OrdsCount)//���������̼��Ͼ��۵ĵ��ӹ���
    {
        HangOrd(*current_ord, current_ord->FunctionCode, true);
        current_ord++;
        ord_counter++;
    }
    DealDroppedOrder((txn_buffer + (Mkt.TxnCount - 1))->TradeIndex); //��������ʣ�೷��
    CallAuction(CLOSEAUC);   //���̼��Ͼ���
}
//���׺���
void MarketManager::CallAuction(char mode)
{
    //���Ͼ���ʱ�Σ����д��Order Book�����е�ȫ������
    //1. ���¼��Ͼ���ʱ��Ч�����ļ۸�Χ
    Mkt.PreNewestPrice = mode == OPENAUC ? Mkt.LstClose : Mkt.NewestPrice;
    UpdateBound(Mkt.PreNewestPrice);
    UpdateBestBid();
    UpdateBestOffer();
    if (Mkt.BestOffer > Mkt.BestBid || bids_order_book.empty() || offers_order_book.empty())//�޷���Ͻ��ף�������Ч���۷�Χ֮��ֱ�ӷ���
    {
        /*�޼۸��ǵ������Ƶ�֤ȯ�ڼ��Ͼ����ڼ�û�в����ɽ��ģ���������ʱ�������з�ʽ������Ч���۷�Χ��  
        ��һ����Ч���۷�Χ�ڵ���������걨�۸��ڼ�ʱ������ʾ��ǰ���̼ۻ�����ɽ��ۣ�����������걨��Ϊ��׼������Ч���۷�Χ��
        ��������Ч���۷�Χ�ڵ���������걨�۵��ڼ�ʱ������ʾ��ǰ���̼ۻ�����ɽ��ۣ�����������걨��Ϊ��׼������Ч���۷�Χ��        */
        if (IsValidPrice(Mkt.BestBid) && Mkt.BestBid > Mkt.PreNewestPrice)
            UpdateBound(Mkt.BestBid); //������Ч���۷�Χ
        else if (IsValidPrice(Mkt.BestOffer) && Mkt.BestOffer < Mkt.PreNewestPrice)
            UpdateBound(Mkt.BestOffer);
        return;
    }
    //2. ȷ�����д�ϵĳɽ��۸�
    double cur_offer_price = Mkt.BestOffer;        //��ǰ��һ��
    double cur_bid_price = Mkt.BestBid;            //��ǰ��һ��
    double final_price = MIN;                       //���Ͼ��۽׶����ճɽ���
    double total_volume = 0.0;                         //�ü۸��µ��ܳɽ�����
    if (cur_offer_price == cur_bid_price)
        final_price = cur_offer_price;
    else
    {   /*
        3.5.2 ���Ͼ���ʱ���ɽ��۵�ȷ��ԭ��Ϊ��
        ��һ����ʵ�����ɽ�����
        ���������ڸü۸�������걨����ڸü۸�������걨ȫ���ɽ���
        ��������ü۸���ͬ���򷽻�����������һ��ȫ���ɽ���
        */
        std::map<double, Level>::iterator offer_iter = offers_order_book.find(Mkt.BestOffer);   //��ʼ�ļ۸�����һ�۸������ӣ�++��
        std::map<double, Level>::iterator bid_iter = bids_order_book.find(Mkt.BestBid);         //��ʼ�ļ۸�����һ�۸��𽥽��ͣ�--��
        double cur_offer_vol = offer_iter->second.TotalVolume;  //��ǰ��һ��(��̬)
        double cur_bid_vol = bid_iter->second.TotalVolume;     //��ǰ��һ��(��̬)
        while (cur_offer_price < cur_bid_price && bid_iter != --bids_order_book.begin()
            && offer_iter != offers_order_book.end()        //�̿ڲ�Ϊ�����ܼ������
            && IsValidPrice(bid_iter->first)
            && IsValidPrice(offer_iter->first))             //�۸�Ϸ�
        {
            if (cur_offer_vol > cur_bid_vol)    //��ǰ����������������
            {
                total_volume += cur_bid_vol;    //���̳�������
                cur_offer_vol -= cur_bid_vol;
                bid_iter--;                     //ָ���Ƶ���һ�������̿�
                if (bid_iter == --bids_order_book.begin() || bid_iter->first < cur_offer_price || bid_iter->first < Mkt.BestOffer || !IsValidPrice(bid_iter->first))   //���̱�ƽ�꣬������һ���򵥲���ƽ
                {
                    final_price = cur_offer_price;  //���ճɽ���Ϊ��ǰ���̼۸�
                    break;
                }
                else //if (BidIt != BidPrices.rend())  ������ûƽ��
                {
                    cur_bid_price = bid_iter->first;            //���¼������Ҽ۸�һ���ܼ������
                    cur_bid_vol = bid_iter->second.TotalVolume;
                }
            }//��if֮��ֱ�ӿ�ʼ��һ��ѭ��
            else if (cur_offer_vol < cur_bid_vol)   //�߼�ͬ��
            {
                total_volume += cur_offer_vol;
                cur_bid_vol -= cur_offer_vol;
                offer_iter++;
                if (offer_iter == offers_order_book.end() || offer_iter->first > Mkt.BestBid || !IsValidPrice(offer_iter->first) || offer_iter->first > cur_bid_price)
                {
                    final_price = cur_bid_price;
                    break;
                }
                else
                {
                    cur_offer_price = offer_iter->first;
                    cur_offer_vol = offer_iter->second.TotalVolume;
                }
            }
            else //if (cur_offer_vol == cur_bid_vol) //���߸պû���ƽ��
            {
                total_volume += cur_offer_vol;
                bid_iter--;
                offer_iter++;
                if ((offer_iter == offers_order_book.end() && bid_iter == --bids_order_book.begin()) || //�����̽Կ�
                    (!IsValidPrice(offer_iter->first) && !IsValidPrice(bid_iter->first)) ||             //��������һ���۸����Ч
                    (offer_iter == offers_order_book.end() && !IsValidPrice(offer_iter->first)) ||      //���̿��ˣ�������Ч
                    (bid_iter == --bids_order_book.begin() && !IsValidPrice(bid_iter->first)))          //���̿��ˣ�������Ч
                    break;      //���ճɽ���δȷ��
                else if (offer_iter == offers_order_book.end() || !IsValidPrice(offer_iter->first)) //���̿��˻����¸��۸���Ч
                {
                    final_price = cur_bid_price;
                    break;
                }
                else if (bid_iter == --bids_order_book.begin() || !IsValidPrice(bid_iter->first))  //���̿��˻����¸��۸���Ч
                {
                    final_price = cur_offer_price;
                    break;
                }
                cur_bid_price = bid_iter->first;
                cur_offer_price = offer_iter->first;
                cur_bid_vol = bid_iter->second.TotalVolume;
                cur_offer_vol = offer_iter->second.TotalVolume;
                //���cur_bid_price < cur_offer_priceʱ,��һ��ѭ�����ʱ��ͨ��,��ֱ������,�ɽ���δȷ��
            }
        }
        /*
        �������ϼ۸�������������ģ�ȡ�ڸü۸����ϵ������걨�ۼ��������ڸü۸����µ������걨�ۼ�����֮����С�ļ۸�Ϊ�ɽ��ۣ�
        �����걨�ۼ�����֮���Դ����������ģ����̼��Ͼ���ʱȡ��ӽ���ʱ������ʾ��ǰ���̼۵ļ۸�Ϊ�ɽ��ۣ�
        ���С����̼��Ͼ���ʱȡ��ӽ�����ɽ��۵ļ۸�Ϊ�ɽ��ۡ�
        */
        if (final_price == MIN)
        {
            std::vector<double>PossiblePrices;
            int gaps = int(cur_offer_price * 1000 + 0.00001) - int(cur_bid_price * 1000 + 0.00001);
            for (int i = 0; i <= gaps; i++)
                PossiblePrices.emplace_back(cur_bid_price + i * MINGAP);
            double min_delta_volume = MAX; //�ü۸����ϵ������걨�ۼ��������ڸü۸����µ������걨�ۼ�����֮��

            for (auto it = PossiblePrices.begin(); it != PossiblePrices.end(); ++it)
            {
                double bid_volume = 0;          //��ǰ�۸����ϵ��ۼ������걨����
                double offer_volume = 0;        //��ǰ�۸����µ��ۼ������걨����
                double delta_volume = 0;        //���������Ĳ�
                //�õ��ü۸����ϵ��ۼ������걨����
                bid_iter = bids_order_book.find(Mkt.BestBid);
                while (bid_iter->first >= *it - MINGAP * 0.001 && bid_iter != --bids_order_book.begin() && IsValidPrice(bid_iter->first))
                {
                    bid_volume += bid_iter->second.TotalVolume;
                    bid_iter--;
                }
                //�õ��ü۸����µ��ۼ������걨����
                offer_iter = offers_order_book.find(Mkt.BestOffer);
                while (offer_iter->first <= *it + MINGAP * 0.001 && offer_iter != offers_order_book.end() && IsValidPrice(offer_iter->first))
                {
                    offer_volume += offer_iter->second.TotalVolume;
                    offer_iter++;
                }
                //�õ����������Ĳ�ֵ
                delta_volume = abs(bid_volume - offer_volume);
                //�����ǰ��ֵС����С��ֵ,�������ճɽ���
                if (delta_volume < min_delta_volume)
                {
                    min_delta_volume = delta_volume;
                    final_price = *it; 
                }
                //������������ϼ۸�����С��ֵ,���ո��ӽ����³ɽ���/���յ���һ��
                else if (delta_volume == min_delta_volume)
                    final_price = abs(*it - Mkt.PreNewestPrice) > abs(final_price - Mkt.PreNewestPrice) ? final_price : *it;
            }
        }
    }
    //3. ���д�ϵĽ��׼۸�ȷ���󣬴�����пɳɽ�����
    std::deque<ORDER_STRUCT>* cur_ord_deQ;
    std::map<double, Level>::iterator pool_iter = offers_order_book.find(Mkt.BestOffer);
    std::queue<ORDER_STRUCT> callauc_offers;
    while (pool_iter != offers_order_book.end() && pool_iter->first <= final_price + 0.0001)
    {
        cur_ord_deQ = &(pool_iter->second.OrdList);
        while (!cur_ord_deQ->empty())
        {
            callauc_offers.push(cur_ord_deQ->front());
            cur_ord_deQ->pop_front();
        }
        double pre = pool_iter->first;
        pool_iter++;
        DeleteLevel(pre, SELLFLAG);
    }
    pool_iter = bids_order_book.find(Mkt.BestBid);
    std::queue<ORDER_STRUCT> callauc_bids;
    while (pool_iter != --bids_order_book.begin() && pool_iter->first >= final_price - 0.0001)
    {
        cur_ord_deQ = &(pool_iter->second.OrdList);
        while (!cur_ord_deQ->empty())
        {
            callauc_bids.push(cur_ord_deQ->front());
            cur_ord_deQ->pop_front();
        }
        double pre = pool_iter->first;
        --pool_iter;
        DeleteLevel(pre, BUYFLAG);
    }
    ORDER_STRUCT offer = callauc_offers.front(); callauc_offers.pop();
    ORDER_STRUCT bid = callauc_bids.front(); callauc_bids.pop();
    do //��������۸�����ڵ�ȫ������
    {
        TRADE_STRUCT trade;
        trade.BizIndex = 0;
        trade.BuyIndex = bid.OrderIndex;
        memcpy_s(trade.ExchangeID, 4, offer.ExchangeID, 4);
        trade.FunctionCode = '0';
        memcpy_s(trade.InstrumentID, 9, offer.InstrumentID, 9);
        memcpy_s(trade.MDStreamID, 4, offer.MDStreamID, 4);
        trade.OrderBSFlag = 'N'; //���Ͼ���
        trade.Price = final_price;
        trade.SellIndex = offer.OrderIndex;
        trade.TradeGroupID = offer.OrderGroupID; //�ٶ�Ϊ1
        trade.TradeIndex = Mkt.TxnBizIdx++;
        if (mode == OPENAUC)
            memcpy_s(trade.TradeTime, 13, NINE_TWENTY_FIVE_AM, 13);
        else if (mode == STOPAUC)
            memcpy_s(trade.TradeTime, 13, Mkt.StopTradeUntil, 13);
        else
            memcpy_s(trade.TradeTime, 13, THREE_PM, 13);

        if (bid.Volume > offer.Volume)
        {
            trade.Volume = offer.Volume;
            bid.Volume -= offer.Volume;
            offer.Volume = 0;
            if (!callauc_offers.empty())
            {
                offer = callauc_offers.front();
                callauc_offers.pop();
            }
        }
        else if (bid.Volume < offer.Volume)
        {
            trade.Volume = bid.Volume;
            offer.Volume -= bid.Volume;
            bid.Volume = 0;
            if (!callauc_bids.empty())
            {
                bid = callauc_bids.front();
                callauc_bids.pop();
            }
        }
        else
        {
            trade.Volume = offer.Volume;
            offer.Volume = 0;
            if (!callauc_offers.empty())
            {
                offer = callauc_offers.front();
                callauc_offers.pop();
            }
            bid.Volume = 0;
            if (!callauc_bids.empty())
            {
                bid = callauc_bids.front();
                callauc_bids.pop();
            }
        }
        Mkt.TotalTradeNumber++;
        Mkt.TotalTradeVol += trade.Volume;
        trades.emplace_back(trade);
    } while (offer.Volume != 0 && bid.Volume != 0);
    if (bid.Volume != 0) //���һ��ȡ����δȫ���ɽ��Ķ����ɳɽ���������������
        HangOrd(bid, bid.FunctionCode, false);
    if (offer.Volume != 0)
        HangOrd(offer, offer.FunctionCode, false);
    while (!callauc_bids.empty()) //�Ѹü۸������ʣ�ඩ���Ż�ȥ
    {
        HangOrd(callauc_bids.front(), BUYFLAG, false);
        callauc_bids.pop();
    }
    while (!callauc_offers.empty())
    {
        HangOrd(callauc_offers.front(), SELLFLAG, false);
        callauc_offers.pop();
    }
    //4. ����Mkt��Ϣ
    if (mode == OPENAUC)
        Mkt.TodayOpen = final_price;
    if (mode == CLOSEAUC)
        Mkt.TodayClose = final_price;
    Mkt.NewestPrice = final_price;
}
void MarketManager::ContBid()
{
    if (STOCKTYPE != ZB)
    {
        //1. �ȸ������¼۸񣬸�����Ч���۷�Χ
        if (Mkt.NewestPrice != 0.0)
            UpdateBound(Mkt.NewestPrice);
        //2. �����µ���Ч���۷�Χ���ȴ�����пɴ�ϵĶ�����
        if (Mkt.NewestPrice > Mkt.PreNewestPrice) //�³ɽ��۴���ǰһ���ɽ���,���ܻ����µ���Ч��
        {
            UpdateBestBid();
            while (Mkt.BestBid >= Mkt.BestOffer) //�µĿɽ��׶�������,����һ�Ķ����ó���,��Ͻ���
            {
                Level* best_bid_level = &bids_order_book[Mkt.BestBid];
                while (!best_bid_level->OrdList.empty() && Mkt.BestBid >= Mkt.BestOffer)    //ֻҪ��ǰ�̿ڷǿ�
                {
                    ORDER_STRUCT* cur_bid = &best_bid_level->OrdList.front();
                    best_bid_level->OrdList.pop_front();
                    best_bid_level->TotalVolume -= cur_bid->Volume;
                    Level* cur_offer_level = &offers_order_book[Mkt.BestOffer];
                    do //���г����ż۸���
                    {
                        bool offer_level_deleted = MakeOneTrade(cur_bid, cur_offer_level);
                        if (Mkt.StopTrade == true)
                        {
                            if (best_bid_level->OrdList.empty())
                                DeleteLevel(best_bid_level->Price, BUYFLAG);
                            return;
                        }
                        if (offer_level_deleted && Mkt.BestOffer != MAX)
                            cur_offer_level = &offers_order_book[Mkt.BestOffer];
                    } while (cur_bid->Volume > 0 && cur_bid->Price >= Mkt.BestOffer);//���ö���û��ƽ��,�һ��ܼ���ƽʱ
                    if (cur_bid->Volume > 0)
                    {
                        best_bid_level->OrdList.emplace_front(*cur_bid);
                        best_bid_level->TotalVolume += cur_bid->Volume;
                    }
                }
                //�����̿ڱ�ƽ��Ļ�
                if (best_bid_level->TotalVolume == 0)
                    DeleteLevel(best_bid_level->Price, BUYFLAG);
                if (Mkt.BestBid < Mkt.BestOffer)    //��һ����Ч�����ڵļ۸��̿ڶ��Ѿ���ȫ��ƽ����
                {
                    Mkt.PreNewestPrice = Mkt.NewestPrice;
                    UpdateBound(Mkt.NewestPrice);
                    UpdateBestBid();
                }
            }
        }
        else if (Mkt.NewestPrice < Mkt.PreNewestPrice)
        {
            UpdateBestOffer();
            while (Mkt.BestOffer <= Mkt.BestBid)
            {
                Level* best_offer_level = &offers_order_book[Mkt.BestOffer];
                while (!best_offer_level->OrdList.empty() && Mkt.BestOffer <= Mkt.BestBid)    //ֻҪ��ǰ�̿ڷǿ�
                {
                    ORDER_STRUCT* cur_offer = &best_offer_level->OrdList.front();
                    best_offer_level->OrdList.pop_front();
                    best_offer_level->TotalVolume -= cur_offer->Volume;
                    Level* cur_bid_level = &bids_order_book[Mkt.BestBid];
                    do //���г����ż۸���
                    {
                        //if (current_ord->OrderIndex == 2065561)
                        //    std::cout << "blah";
                        bool bid_level_deleted = MakeOneTrade(cur_offer, cur_bid_level);
                        if (Mkt.StopTrade == true)
                        {
                            if (best_offer_level->OrdList.empty())
                                DeleteLevel(best_offer_level->Price, SELLFLAG);
                            return;
                        }
                        if (bid_level_deleted && Mkt.BestBid != MIN)
                            cur_bid_level = &bids_order_book[Mkt.BestBid];
                    } while (cur_offer->Volume > 0 && cur_offer->Price <= Mkt.BestBid);//���ö���û��ƽ��,�һ��ܼ���ƽʱ
                    if (cur_offer->Volume > 0)
                    {
                        best_offer_level->OrdList.emplace_front(*cur_offer);
                        best_offer_level->TotalVolume += cur_offer->Volume;
                    }
                }
                //�����̿ڱ�ƽ��Ļ�
                if (best_offer_level->TotalVolume == 0)
                    DeleteLevel(best_offer_level->Price, SELLFLAG);
                if (Mkt.BestOffer > Mkt.BestBid)
                {
                    Mkt.PreNewestPrice = Mkt.NewestPrice;
                    UpdateBound(Mkt.NewestPrice);
                    UpdateBestOffer();
                }
            }
        }
    }
    //3. �����г������пɴ�϶���������֮�󣬸���ǰ�ɽ��ۣ�����ȡ�¶������н��ף������¶������������۴��
    Mkt.PreNewestPrice = Mkt.NewestPrice;
    if (current_ord->FunctionCode == BUYFLAG)
    {
        if (!IsValidPrice(current_ord->Price) || Mkt.BestOffer > current_ord->Price || offers_order_book.empty()) // �����̿�һ�ļ۸���������,�޷����
        {
            HangOrd(*current_ord, BUYFLAG, true);
            return;
        }
        Level* cur_offer_level = &offers_order_book[Mkt.BestOffer];
        do //���г����ż۸���
        {
            bool offer_level_deleted = MakeOneTrade(current_ord, cur_offer_level);
            if (Mkt.StopTrade == true)
                return;
            if (offer_level_deleted && Mkt.BestOffer != MAX)
                cur_offer_level = &offers_order_book[Mkt.BestOffer];
        } while (current_ord->Volume > 0 && current_ord->Price >= Mkt.BestOffer);//���ö���û��ƽ��,�һ��ܼ���ƽʱ
    }
    else //if current_ord->FunctionCode == SELLFLAG
    {
        if (!IsValidPrice(current_ord->Price) || Mkt.BestBid < current_ord->Price || bids_order_book.empty()) // ������߳��۵��ڸö����۸�
        {
            HangOrd(*current_ord, SELLFLAG, true);
            return;
        }
        double pre_price = Mkt.NewestPrice;
        Level* cur_bid_level = &bids_order_book[Mkt.BestBid];
        do
        {
            bool bid_level_deleted = MakeOneTrade(current_ord, cur_bid_level);
            if (Mkt.StopTrade == true)
                return;
            if (bid_level_deleted && Mkt.BestBid != MIN)
                cur_bid_level = &bids_order_book[Mkt.BestBid];
        } while (current_ord->Volume > 0 && current_ord->Price <= Mkt.BestBid);
    }
    if (current_ord->Volume > 0) //����û��ƽ��
        HangOrd(*current_ord, current_ord->FunctionCode, true);
}
bool MarketManager::MakeOneTrade(ORDER_STRUCT* cur_ord, Level* match_lev)
{
    bool match_level_deleted = false;
    //1. ����³ɽ���������Trades����
    ORDER_STRUCT* match_ord = &match_lev->OrdList.front();
    TRADE_STRUCT trade;
    trade.BizIndex = 0;
    if (cur_ord->FunctionCode == BUYFLAG)
    {
        trade.BuyIndex = cur_ord->OrderIndex;
        trade.SellIndex = match_ord->OrderIndex;
        trade.OrderBSFlag = SELLFLAG; //�������������۸�����
    }
    else
    {
        trade.BuyIndex = match_ord->OrderIndex;
        trade.SellIndex = cur_ord->OrderIndex;
        trade.OrderBSFlag = BUYFLAG; //�����������򷽼۸�����
    }
    memcpy_s(trade.ExchangeID, 4, cur_ord->ExchangeID, 4);
    trade.FunctionCode = '0';
    memcpy_s(trade.InstrumentID, 9, cur_ord->InstrumentID, 9);
    memcpy_s(trade.MDStreamID, 4, cur_ord->MDStreamID, 4);
    trade.Price = match_lev->Price;
    trade.TradeGroupID = cur_ord->OrderGroupID; //�ٶ���order��һ��
    trade.TradeIndex = Mkt.TxnBizIdx++;
    memcpy_s(trade.TradeTime, 13, cur_ord->OrderTime, 13);

    if (match_ord->Volume > cur_ord->Volume)
    {
        trade.Volume = cur_ord->Volume;
        match_ord->Volume -= cur_ord->Volume;
        match_lev->TotalVolume -= cur_ord->Volume;
        cur_ord->Volume = 0;
    }
    else// if(match_ord->Volume <= cur_ord->Volume)
    {
        trade.Volume = match_ord->Volume;
        cur_ord->Volume -= match_ord->Volume;
        match_lev->OrdList.pop_front();
        if (match_lev->OrdList.empty())
        {
            DeleteLevel(match_lev->Price, match_ord->FunctionCode);
            match_level_deleted = true;
        }
        else
            match_lev->TotalVolume -= trade.Volume;
    }
    trades.emplace_back(trade);
    //2.���³������³ɽ���������г���Ϣ
    Mkt.TotalTradeNumber++;
    Mkt.TotalTradeVol += trade.Volume;
    Mkt.NewestPrice = trade.Price;
    //3.����³ɽ��۸��Ƿ񴥷���ͣ
    if (CheckStopTrade() == true)
    {
        UpdateBound(Mkt.NewestPrice);
        if (current_ord->Volume > 0)
            HangOrd(*current_ord, current_ord->FunctionCode, true);
    }
    return match_level_deleted;
}
void MarketManager::HangOrd(ORDER_STRUCT someord, char mode, bool new_ord)
{
    if(new_ord)
        idx2price.insert({ someord.OrderIndex, someord.Price });
    std::map<double, Level>* Pool;
    std::map<double, Level>::iterator it;
    if (mode == BUYFLAG)
    {
        Pool = &bids_order_book;
        if (someord.Price > Mkt.BestBid && IsValidPrice(someord.Price))
            Mkt.BestBid = someord.Price;
    }
    else //if (mode == SELLFLAG)
    {
        Pool = &offers_order_book;
        if (someord.Price < Mkt.BestOffer && IsValidPrice(someord.Price))
            Mkt.BestOffer = someord.Price;
    }
    //����ü۸��̿ڲ������̻��������棬�½��̿ڲ�����
    if (Pool->empty() || Pool->find(someord.Price) == Pool->end())
    {
        Level newlev;
        newlev.Price = someord.Price;
        newlev.TotalVolume = someord.Volume;
        newlev.OrdList.emplace_back(someord);
        Pool->insert({ newlev.Price, newlev });
        return;
    }
    //����ü۸����̿�����
    else
    {
        auto it = Pool->find(someord.Price);
        it->second.OrdList.emplace_back(someord);
        it->second.TotalVolume += someord.Volume;
    }
}
void MarketManager::DealDroppedOrder(long long idx)
{
    //�Ѿ��������һ�ʳɽ�����
    if (txn_counter == Mkt.TxnCount)
        return;
    //�ɽ�����current_order֮ǰ
    while (current_txn->TradeIndex <= idx)
    {
        if (current_txn->FunctionCode == 'F') //��ǰ�����ǳɽ���
            real_trades.emplace_back(*current_txn);
        else //��ǰtxn�Ǹ�����
        {
            if (current_txn->SellIndex == 0)//sell indexΪ0˵��Ҫ����
                CancelOrd(current_txn->BuyIndex, idx2price[current_txn->BuyIndex], current_txn->Volume, BUYFLAG);
            else //if buyindex == 0 ��Ҫ������
                CancelOrd(current_txn->SellIndex, idx2price[current_txn->SellIndex], current_txn->Volume, SELLFLAG);
        }
        txn_counter++;
        if (txn_counter == Mkt.TxnCount)
            break;
        current_txn++;
    }
}
void MarketManager::CancelOrd(long long idx, double price, double vol, char mode)
{
    std::deque<ORDER_STRUCT>* ordInQ;
    std::map<double, Level>::iterator price_st;
    if (mode == BUYFLAG)
    {
        price_st = bids_order_book.find(price);
        if (price_st == bids_order_book.end())
            return;
    }
    else //if(mode == SELLFLAG)
    {
        price_st = offers_order_book.find(price);
        if (price_st == offers_order_book.end())
            return;
    }

    ordInQ = &(price_st->second.OrdList);
    for (auto it = ordInQ->begin(); it != ordInQ->end(); ++it)
    {
        if (it->OrderIndex == idx)
        {
            it->Volume -= vol;
            Mkt.TotalVol -= vol;
            price_st->second.TotalVolume -= vol;
            if (it->Volume <= 0)
                ordInQ->erase(it);
            if (ordInQ->empty())
                DeleteLevel(price, mode);
            return;
        }
    }
}
void MarketManager::DeleteLevel(double price, char mode)
{
    if (mode == BUYFLAG)
    {
        if (price == Mkt.BestBid)
        {
            auto best_bid_pos = bids_order_book.find(Mkt.BestBid);
            if (best_bid_pos != bids_order_book.begin())
                Mkt.BestBid = (--best_bid_pos)->first;
            else
                Mkt.BestBid = MIN;
        }
        bids_order_book.erase(price);
    }
    else //if (mode == SELLFLAG)
    {
        if (price == Mkt.BestOffer)
        {
            auto best_offer_pos = offers_order_book.find(Mkt.BestOffer);
            if (best_offer_pos != --offers_order_book.end())
                Mkt.BestOffer = (++best_offer_pos)->first;
            else
                Mkt.BestOffer = MAX; //��ʼֵ
        }
        offers_order_book.erase(price);
    }
}
//Utility Funcs
inline bool MarketManager::IsValidPrice(double Price)
{
    if(STOCKTYPE == KZZ)
        return (Mkt.DynaLo < Price && Price < Mkt.DynaHi);
    return true;
}
inline void MarketManager::UpdateBound(double Price)
{
    Mkt.DynaHi = round(Price * (1 + TRADEPB) * 1000.0) / 1000.0 + MINGAP * 0.1;
    Mkt.DynaLo = round(Price * (1 - TRADEPB) * 1000.0) / 1000.0 - MINGAP * 0.1;
}
inline void MarketManager::UpdateBestOffer()
{
    auto best_offer_pos = offers_order_book.find(Mkt.BestOffer);
    if (best_offer_pos != offers_order_book.end() && best_offer_pos != offers_order_book.begin())
    {
        if (best_offer_pos->first < Mkt.DynaLo)
        {
            do
                best_offer_pos++;
            while (best_offer_pos != --offers_order_book.end() && best_offer_pos->first < Mkt.DynaLo);
            Mkt.BestOffer = best_offer_pos->first;
        }
        else
        {
            do
                best_offer_pos--;
            while (best_offer_pos != --offers_order_book.begin() && best_offer_pos->first > Mkt.DynaLo);
            Mkt.BestOffer = best_offer_pos == --offers_order_book.begin() ? offers_order_book.begin()->first : (++best_offer_pos)->first;
        }
        if (IsValidPrice(Mkt.BestOffer) == false) //������̳��ۺܸ�,��ͳ��۶����������Ч��Χ,��Mkt.BestOfferΪ0
            Mkt.BestOffer = MAX;
    }
}
inline void MarketManager::UpdateBestBid()
{
    auto best_bid_pos = bids_order_book.find(Mkt.BestBid);
    if (best_bid_pos != bids_order_book.end() && best_bid_pos != --bids_order_book.end())
    {
        if (best_bid_pos->first > Mkt.DynaHi)
        {
            do
                best_bid_pos--;
            while (best_bid_pos != bids_order_book.begin() && best_bid_pos->first > Mkt.DynaHi);
            Mkt.BestBid = best_bid_pos->first;
        }
        else
        {
            do
                best_bid_pos++;
            while (best_bid_pos != bids_order_book.end() && best_bid_pos->first < Mkt.DynaHi);
            Mkt.BestBid = (--best_bid_pos)->first;
        }
        if (IsValidPrice(Mkt.BestBid) == false)
            Mkt.BestBid = MIN;
    }
}
void MarketManager::UpdateMktInfo()
{
    int i = 0;
    if (Mkt.BestBid != MAX)
    {
        auto bid_iter = bids_order_book.find(Mkt.BestBid);
        for (; i < LEVELDISPLAY; i++)
        {
            if (bid_iter == --bids_order_book.begin() || !IsValidPrice(bid_iter->first))
                break;
            Mkt.BidPrices[i] = bid_iter->first;
            Mkt.BidVolumes[i] = bid_iter->second.TotalVolume;
            bid_iter--;
        }
    }
    while (i < LEVELDISPLAY)
    {
        Mkt.BidPrices[i] = MIN;
        Mkt.BidVolumes[i] = MIN;
        i++;
    }
    i = 0;
    if (Mkt.BestOffer != MIN)
    {
        auto offer_iter = offers_order_book.find(Mkt.BestOffer);
        for (; i < LEVELDISPLAY; i++)
        {
            if (offer_iter == offers_order_book.end() || !IsValidPrice(offer_iter->first))
                break;
            Mkt.OfferPrices[i] = offer_iter->first;
            Mkt.OfferVolumes[i] = offer_iter->second.TotalVolume;
            offer_iter++;
        }
    }
    while (i < LEVELDISPLAY)
    {
        Mkt.OfferPrices[i] = MIN;
        Mkt.OfferVolumes[i] = MIN;
        i++;
    }
}
bool MarketManager::Time1Bigger(char* time1, char* time2)
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
bool MarketManager::CheckStopTrade()
{
    if (STOCKTYPE == ZB)    //��������彻��,û����ͣ����,ֱ�ӷ���
        return false;
    //ÿ�õ�һ���¼۸�Ҫȷ���Ƿ񳬹��۸�����
    if (Mkt.NewestPrice <= Mkt.TxnPauseBound[0] ||
        (Mkt.FirstTimeFallStop == true && Mkt.NewestPrice <= Mkt.TxnPauseBound[1]) ||
        (Mkt.FirstTimeRiseStop == true && Mkt.NewestPrice >= Mkt.TxnPauseBound[2]) ||
        Mkt.NewestPrice >= Mkt.TxnPauseBound[3])
    {
        Mkt.StopTrade = true;
        memcpy_s(Mkt.StopTradeUntil, 13, (current_ord + 1)->OrderTime, 13);
        if(Mkt.FirstTimeFallStop == true && Mkt.NewestPrice <= Mkt.TxnPauseBound[1])
            Mkt.FirstTimeFallStop = false;
        else if (Mkt.FirstTimeRiseStop == true && Mkt.NewestPrice >= Mkt.TxnPauseBound[2])
            Mkt.FirstTimeRiseStop = false;
        return true;
    }
    return false;
}
//�������
void MarketManager::PrintTrades()
{
    for (auto it = trades.begin(); it != trades.end(); ++it)
    {
        std::cout << "�����ţ�" << it->TradeIndex << std::endl
            << "ʱ�䣺" << it->TradeTime << std::endl
            << "�۸�" << it->Price << std::endl
            << "������" << it->Volume << std::endl
            << "�򷽣�" << it->BuyIndex << std::endl
            << "������" << it->SellIndex << std::endl << std::endl;
    }
}
void MarketManager::PrintMktInfo()
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
MktInfo* MarketManager::ReturnMktInfo()
{
    UpdateMktInfo();
    return &Mkt;
}
void MarketManager::Trades2File()
{
    std::ofstream trade2f;
    trade2f.open("D:\\chen1ting\\StockRawData\\kzzdata\\bin2csvOutput\\" + std::string(order_buffer->InstrumentID) + "SimTrade.csv");
    trade2f << "TradeTime,OrderBSFlag,Price,Volume,BuyIndex,SellIndex" << std::endl;
    for (auto it = trades.begin(); it != trades.end(); ++it)
    {
        trade2f << it->TradeTime << "," <<
            it->OrderBSFlag << "," <<
            it->Price << "," <<
            it->Volume << "," <<
            it->BuyIndex << "," <<
            it->SellIndex << "," << std::endl;
    }
}
void MarketManager::Levels2File(std::string filename)
{
    std::ofstream level2f;
    level2f.open("D:\\chen1ting\\StockRawData\\kzzdata\\bin2csvOutput\\" + filename + "Levels.csv");
    level2f << "����" << std::endl;
    level2f << "Price,Volume" << std::endl;
    for (auto it = offers_order_book.begin(); it != offers_order_book.end(); ++it)
        level2f << it->first << "," << it->second.TotalVolume << std::endl;
    level2f << "����" << std::endl;
    level2f << "Price,Volume" << std::endl;
    for (auto it = bids_order_book.begin(); it != bids_order_book.end(); ++it)
        level2f << it->first << "," << it->second.TotalVolume << std::endl;
}
bool MarketManager::CheckSimCorrectness()
{//�Ƚ�ÿ���ɽ�������ҡ����ҡ��ɽ��۸������
    bool same = true;
    auto real_trades_iter = real_trades.begin();
    auto trades_iter = trades.begin();
    while (real_trades_iter != real_trades.end())
    {
        if (real_trades_iter->BuyIndex != trades_iter->BuyIndex
            || real_trades_iter->SellIndex != trades_iter->SellIndex
            || real_trades_iter->Volume != trades_iter->Volume)
        {
            same = false;
            std::cout << "��һ������\n"
                << "��ʵ���:" << real_trades_iter->BuyIndex << "\t������:" << trades_iter->BuyIndex << std::endl
                << "��ʵ����:" << real_trades_iter->SellIndex << "\t������:" << trades_iter->SellIndex << std::endl
                << "��ʵ����:" << real_trades_iter->Volume << "\t�������:" << trades_iter->Volume << std::endl
                << "��ʵ�۸�:" << real_trades_iter->Price << "\t��ϼ۸�:" << trades_iter->Price << std::endl;
            break;
        }
        trades_iter++;
        real_trades_iter++;
    }
    return same;
}
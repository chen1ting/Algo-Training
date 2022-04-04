#include "MARKET_MANAGER.h"
constexpr double MAX = 10000000.0;      //BestOffer的初始值
constexpr double MIN = -1.0;            //中间价格及BestBid的初始值
constexpr char BUYFLAG = '1';           //买单Function Code
constexpr char SELLFLAG = '2';          //卖单Function Code
constexpr char OPENAUC = 'M';           //早盘集合竞价
constexpr char STOPAUC = 'C';           //临停后复牌的集合竞价
constexpr char CLOSEAUC = 'O';          //收盘集合竞价
static char NINE_TWENTY_FIVE_AM[13] = "09:25:00.000";
static char TWELVE[13] = "12:00:00.000";
static char TWO_FIFTY_SEVEN_PM[13] = "14:57:00.000";
static char THREE_PM[13] = "15:00:00.000";
inline bool operator<(const ORDER_STRUCT& a, const ORDER_STRUCT& b) { return a.OrderIndex < b.OrderIndex; }

MarketManager::MarketManager(char mode)
{
    //初始化Mkt的成员
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
    default:    //默认为主板股票交易
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
    Mkt.OrdsCount = order_input.tellg() / sizeof(ORDER_STRUCT); //初始化Count,用于记录合理的数据范围
    order_buffer = (ORDER_STRUCT*)malloc(sizeof(ORDER_STRUCT) * (Mkt.OrdsCount)); //初始化buffer
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
    Mkt.TxnCount = trade_input.tellg() / sizeof(TRADE_STRUCT); //初始化Count,用于记录合理的数据范围
    txn_buffer = (TRADE_STRUCT*)malloc(sizeof(TRADE_STRUCT) * (Mkt.TxnCount)); //初始化buffer
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
    //订单时间小于等于9:25，直接挂起
    while (Time1Bigger(NINE_TWENTY_FIVE_AM, current_ord->OrderTime) && ord_counter < Mkt.OrdsCount)
    {
        HangOrd(*current_ord, current_ord->FunctionCode, true);
        current_ord++;
        ord_counter++;
    }
    //退出while循环的current_ord时间大于9:25.先集中处理一次该订单之前的撤单，再集合竞价
    DealDroppedOrder((current_ord - 1)->OrderIndex);
    CallAuction(OPENAUC);
    //PrintMktInfo();
    //订单时间小于14:57,连续竞价
    while (Time1Bigger(TWO_FIFTY_SEVEN_PM, current_ord->OrderTime) && ord_counter < Mkt.OrdsCount)
    {
        if (Mkt.StopTrade == true) //如果处于临停阶段
        {//并且当前订单时间小于等于临停结束时间,直接挂起
            if (!Time1Bigger((current_ord+1)->OrderTime, Mkt.StopTradeUntil) && (current_ord + 1)->OrderIndex - current_ord->OrderIndex <= 150)
                HangOrd(*current_ord, current_ord->FunctionCode, true);
            else //临停期间收到的最后一个订单
            {
                DealDroppedOrder(current_ord ->OrderIndex);         //先处理该订单之前的所有单子
                HangOrd(*current_ord, current_ord->FunctionCode, true);   //再把这个订单挂起
                CallAuction(STOPAUC);   //复牌集合竞价
                Mkt.StopTrade = false;  //跳出临停状态
            }
        }
        else   //正常连续竞价
        {
            DealDroppedOrder(current_ord->OrderIndex);  //先撤单
            ContBid();  //再连续竞价
        }
        current_ord++;  //指针移到下一个订单
        ord_counter++;
    }
    //退出前一个while循环的第一个订单时间>=14:57
    if (Mkt.StopTrade == true) //如果市场状态是临停,说明该订单是临停阶段下的单子,应挂起
    {
        while (!Time1Bigger(current_ord->OrderTime, TWO_FIFTY_SEVEN_PM)) //挂起订单时间<=14:57的单子
        {
            HangOrd(*current_ord, current_ord->FunctionCode, true);
            current_ord++;
            ord_counter++;
        }
        //跳出while循环的第一个订单时间>14:57,是收盘集合竞价的单子
        DealDroppedOrder((txn_buffer + (Mkt.TxnCount - 1))->TradeIndex); //处理该单子之前的撤单
        CallAuction(STOPAUC);   //进行复牌集合竞价
        Mkt.StopTrade = false;  //跳出临停状态
    }
    while (ord_counter < Mkt.OrdsCount)//将所有收盘集合竞价的单子挂起
    {
        HangOrd(*current_ord, current_ord->FunctionCode, true);
        current_ord++;
        ord_counter++;
    }
    DealDroppedOrder((txn_buffer + (Mkt.TxnCount - 1))->TradeIndex); //处理所有剩余撤单
    CallAuction(CLOSEAUC);   //收盘集合竞价
}
//交易函数
void MarketManager::CallAuction(char mode)
{
    //集合竞价时段：集中撮合Order Book中现有的全部订单
    //1. 更新集合竞价时有效报单的价格范围
    Mkt.PreNewestPrice = mode == OPENAUC ? Mkt.LstClose : Mkt.NewestPrice;
    UpdateBound(Mkt.PreNewestPrice);
    UpdateBestBid();
    UpdateBestOffer();
    if (Mkt.BestOffer > Mkt.BestBid || bids_order_book.empty() || offers_order_book.empty())//无法撮合交易，调整有效竞价范围之后直接返回
    {
        /*无价格涨跌幅限制的证券在集合竞价期间没有产生成交的，继续交易时，按下列方式调整有效竞价范围：  
        （一）有效竞价范围内的最高买入申报价高于即时行情显示的前收盘价或最近成交价，以最高买入申报价为基准调整有效竞价范围；
        （二）有效竞价范围内的最低卖出申报价低于即时行情显示的前收盘价或最近成交价，以最低卖出申报价为基准调整有效竞价范围。        */
        if (IsValidPrice(Mkt.BestBid) && Mkt.BestBid > Mkt.PreNewestPrice)
            UpdateBound(Mkt.BestBid); //调整有效竞价范围
        else if (IsValidPrice(Mkt.BestOffer) && Mkt.BestOffer < Mkt.PreNewestPrice)
            UpdateBound(Mkt.BestOffer);
        return;
    }
    //2. 确定集中撮合的成交价格
    double cur_offer_price = Mkt.BestOffer;        //当前卖一价
    double cur_bid_price = Mkt.BestBid;            //当前买一价
    double final_price = MIN;                       //集合竞价阶段最终成交价
    double total_volume = 0.0;                         //该价格下的总成交手数
    if (cur_offer_price == cur_bid_price)
        final_price = cur_offer_price;
    else
    {   /*
        3.5.2 集合竞价时，成交价的确定原则为：
        （一）可实现最大成交量；
        （二）高于该价格的买入申报与低于该价格的卖出申报全部成交；
        （三）与该价格相同的买方或卖方至少有一方全部成交。
        */
        std::map<double, Level>::iterator offer_iter = offers_order_book.find(Mkt.BestOffer);   //开始的价格是卖一价格，逐渐增加（++）
        std::map<double, Level>::iterator bid_iter = bids_order_book.find(Mkt.BestBid);         //开始的价格是买一价格，逐渐降低（--）
        double cur_offer_vol = offer_iter->second.TotalVolume;  //当前卖一量(动态)
        double cur_bid_vol = bid_iter->second.TotalVolume;     //当前买一量(动态)
        while (cur_offer_price < cur_bid_price && bid_iter != --bids_order_book.begin()
            && offer_iter != offers_order_book.end()        //盘口不为空且能继续撮合
            && IsValidPrice(bid_iter->first)
            && IsValidPrice(offer_iter->first))             //价格合法
        {
            if (cur_offer_vol > cur_bid_vol)    //当前卖盘量大于买盘量
            {
                total_volume += cur_bid_vol;    //卖盘吃完买盘
                cur_offer_vol -= cur_bid_vol;
                bid_iter--;                     //指针移到下一个买盘盘口
                if (bid_iter == --bids_order_book.begin() || bid_iter->first < cur_offer_price || bid_iter->first < Mkt.BestOffer || !IsValidPrice(bid_iter->first))   //买盘被平完，或者下一个买单不能平
                {
                    final_price = cur_offer_price;  //最终成交价为当前卖盘价格
                    break;
                }
                else //if (BidIt != BidPrices.rend())  即买盘没平完
                {
                    cur_bid_price = bid_iter->first;            //更新价量，且价格一定能继续撮合
                    cur_bid_vol = bid_iter->second.TotalVolume;
                }
            }//出if之后直接开始下一个循环
            else if (cur_offer_vol < cur_bid_vol)   //逻辑同上
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
            else //if (cur_offer_vol == cur_bid_vol) //两边刚好互相平完
            {
                total_volume += cur_offer_vol;
                bid_iter--;
                offer_iter++;
                if ((offer_iter == offers_order_book.end() && bid_iter == --bids_order_book.begin()) || //买卖盘皆空
                    (!IsValidPrice(offer_iter->first) && !IsValidPrice(bid_iter->first)) ||             //买卖盘下一个价格皆无效
                    (offer_iter == offers_order_book.end() && !IsValidPrice(offer_iter->first)) ||      //卖盘空了，买盘无效
                    (bid_iter == --bids_order_book.begin() && !IsValidPrice(bid_iter->first)))          //买盘空了，卖盘无效
                    break;      //最终成交价未确定
                else if (offer_iter == offers_order_book.end() || !IsValidPrice(offer_iter->first)) //卖盘空了或者下个价格无效
                {
                    final_price = cur_bid_price;
                    break;
                }
                else if (bid_iter == --bids_order_book.begin() || !IsValidPrice(bid_iter->first))  //买盘空了或者下个价格无效
                {
                    final_price = cur_offer_price;
                    break;
                }
                cur_bid_price = bid_iter->first;
                cur_offer_price = offer_iter->first;
                cur_bid_vol = bid_iter->second.TotalVolume;
                cur_offer_vol = offer_iter->second.TotalVolume;
                //如果cur_bid_price < cur_offer_price时,下一个循环检查时不通过,会直接跳出,成交价未确定
            }
        }
        /*
        两个以上价格符合上述条件的，取在该价格以上的买入申报累计数量与在该价格以下的卖出申报累计数量之差最小的价格为成交价；
        买卖申报累计数量之差仍存在相等情况的，开盘集合竞价时取最接近即时行情显示的前收盘价的价格为成交价，
        盘中、收盘集合竞价时取最接近最近成交价的价格为成交价。
        */
        if (final_price == MIN)
        {
            std::vector<double>PossiblePrices;
            int gaps = int(cur_offer_price * 1000 + 0.00001) - int(cur_bid_price * 1000 + 0.00001);
            for (int i = 0; i <= gaps; i++)
                PossiblePrices.emplace_back(cur_bid_price + i * MINGAP);
            double min_delta_volume = MAX; //该价格以上的买入申报累计数量与在该价格以下的卖出申报累计数量之差

            for (auto it = PossiblePrices.begin(); it != PossiblePrices.end(); ++it)
            {
                double bid_volume = 0;          //当前价格以上的累计买入申报数量
                double offer_volume = 0;        //当前价格以下的累计卖出申报数量
                double delta_volume = 0;        //上述数量的差
                //得到该价格以上的累计买入申报数量
                bid_iter = bids_order_book.find(Mkt.BestBid);
                while (bid_iter->first >= *it - MINGAP * 0.001 && bid_iter != --bids_order_book.begin() && IsValidPrice(bid_iter->first))
                {
                    bid_volume += bid_iter->second.TotalVolume;
                    bid_iter--;
                }
                //得到该价格以下的累计卖出申报数量
                offer_iter = offers_order_book.find(Mkt.BestOffer);
                while (offer_iter->first <= *it + MINGAP * 0.001 && offer_iter != offers_order_book.end() && IsValidPrice(offer_iter->first))
                {
                    offer_volume += offer_iter->second.TotalVolume;
                    offer_iter++;
                }
                //得到两个数量的差值
                delta_volume = abs(bid_volume - offer_volume);
                //如果当前差值小于最小差值,更新最终成交价
                if (delta_volume < min_delta_volume)
                {
                    min_delta_volume = delta_volume;
                    final_price = *it; 
                }
                //如果有两个以上价格都有最小差值,按照更接近最新成交价/昨收的那一个
                else if (delta_volume == min_delta_volume)
                    final_price = abs(*it - Mkt.PreNewestPrice) > abs(final_price - Mkt.PreNewestPrice) ? final_price : *it;
            }
        }
    }
    //3. 集中撮合的交易价格确定后，撮合所有可成交订单
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
    do //撮合两个价格队列内的全部订单
    {
        TRADE_STRUCT trade;
        trade.BizIndex = 0;
        trade.BuyIndex = bid.OrderIndex;
        memcpy_s(trade.ExchangeID, 4, offer.ExchangeID, 4);
        trade.FunctionCode = '0';
        memcpy_s(trade.InstrumentID, 9, offer.InstrumentID, 9);
        memcpy_s(trade.MDStreamID, 4, offer.MDStreamID, 4);
        trade.OrderBSFlag = 'N'; //集合竞价
        trade.Price = final_price;
        trade.SellIndex = offer.OrderIndex;
        trade.TradeGroupID = offer.OrderGroupID; //假定为1
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
    if (bid.Volume != 0) //最后一个取出的未全部成交的订单可成交，加入连续竞价
        HangOrd(bid, bid.FunctionCode, false);
    if (offer.Volume != 0)
        HangOrd(offer, offer.FunctionCode, false);
    while (!callauc_bids.empty()) //把该价格队列里剩余订单放回去
    {
        HangOrd(callauc_bids.front(), BUYFLAG, false);
        callauc_bids.pop();
    }
    while (!callauc_offers.empty())
    {
        HangOrd(callauc_offers.front(), SELLFLAG, false);
        callauc_offers.pop();
    }
    //4. 更新Mkt信息
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
        //1. 先根据最新价格，更新有效竞价范围
        if (Mkt.NewestPrice != 0.0)
            UpdateBound(Mkt.NewestPrice);
        //2. 根据新的有效竞价范围，先撮合所有可撮合的订单，
        if (Mkt.NewestPrice > Mkt.PreNewestPrice) //新成交价大于前一个成交价,可能会有新的有效买单
        {
            UpdateBestBid();
            while (Mkt.BestBid >= Mkt.BestOffer) //新的可交易订单出现,把买一的订单拿出来,撮合交易
            {
                Level* best_bid_level = &bids_order_book[Mkt.BestBid];
                while (!best_bid_level->OrdList.empty() && Mkt.BestBid >= Mkt.BestOffer)    //只要当前盘口非空
                {
                    ORDER_STRUCT* cur_bid = &best_bid_level->OrdList.front();
                    best_bid_level->OrdList.pop_front();
                    best_bid_level->TotalVolume -= cur_bid->Volume;
                    Level* cur_offer_level = &offers_order_book[Mkt.BestOffer];
                    do //与市场最优价格撮合
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
                    } while (cur_bid->Volume > 0 && cur_bid->Price >= Mkt.BestOffer);//当该订单没被平完,且还能继续平时
                    if (cur_bid->Volume > 0)
                    {
                        best_bid_level->OrdList.emplace_front(*cur_bid);
                        best_bid_level->TotalVolume += cur_bid->Volume;
                    }
                }
                //整个盘口被平完的话
                if (best_bid_level->TotalVolume == 0)
                    DeleteLevel(best_bid_level->Price, BUYFLAG);
                if (Mkt.BestBid < Mkt.BestOffer)    //上一波有效限制内的价格盘口都已经被全部平完了
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
                while (!best_offer_level->OrdList.empty() && Mkt.BestOffer <= Mkt.BestBid)    //只要当前盘口非空
                {
                    ORDER_STRUCT* cur_offer = &best_offer_level->OrdList.front();
                    best_offer_level->OrdList.pop_front();
                    best_offer_level->TotalVolume -= cur_offer->Volume;
                    Level* cur_bid_level = &bids_order_book[Mkt.BestBid];
                    do //与市场最优价格撮合
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
                    } while (cur_offer->Volume > 0 && cur_offer->Price <= Mkt.BestBid);//当该订单没被平完,且还能继续平时
                    if (cur_offer->Volume > 0)
                    {
                        best_offer_level->OrdList.emplace_front(*cur_offer);
                        best_offer_level->TotalVolume += cur_offer->Volume;
                    }
                }
                //整个盘口被平完的话
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
    //3. 所有市场中现有可撮合订单撮合完成之后，更新前成交价，再收取新订单进行交易，进入新订单的连续竞价撮合
    Mkt.PreNewestPrice = Mkt.NewestPrice;
    if (current_ord->FunctionCode == BUYFLAG)
    {
        if (!IsValidPrice(current_ord->Price) || Mkt.BestOffer > current_ord->Price || offers_order_book.empty()) // 卖盘盘口一的价格高于其出价,无法撮合
        {
            HangOrd(*current_ord, BUYFLAG, true);
            return;
        }
        Level* cur_offer_level = &offers_order_book[Mkt.BestOffer];
        do //与市场最优价格撮合
        {
            bool offer_level_deleted = MakeOneTrade(current_ord, cur_offer_level);
            if (Mkt.StopTrade == true)
                return;
            if (offer_level_deleted && Mkt.BestOffer != MAX)
                cur_offer_level = &offers_order_book[Mkt.BestOffer];
        } while (current_ord->Volume > 0 && current_ord->Price >= Mkt.BestOffer);//当该订单没被平完,且还能继续平时
    }
    else //if current_ord->FunctionCode == SELLFLAG
    {
        if (!IsValidPrice(current_ord->Price) || Mkt.BestBid < current_ord->Price || bids_order_book.empty()) // 买盘最高出价低于该订单价格
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
    if (current_ord->Volume > 0) //单子没被平完
        HangOrd(*current_ord, current_ord->FunctionCode, true);
}
bool MarketManager::MakeOneTrade(ORDER_STRUCT* cur_ord, Level* match_lev)
{
    bool match_level_deleted = false;
    //1. 撮合新成交单并推入Trades容器
    ORDER_STRUCT* match_ord = &match_lev->OrdList.front();
    TRADE_STRUCT trade;
    trade.BizIndex = 0;
    if (cur_ord->FunctionCode == BUYFLAG)
    {
        trade.BuyIndex = cur_ord->OrderIndex;
        trade.SellIndex = match_ord->OrderIndex;
        trade.OrderBSFlag = SELLFLAG; //买方主动以卖方价格买入
    }
    else
    {
        trade.BuyIndex = match_ord->OrderIndex;
        trade.SellIndex = cur_ord->OrderIndex;
        trade.OrderBSFlag = BUYFLAG; //卖方主动以买方价格卖出
    }
    memcpy_s(trade.ExchangeID, 4, cur_ord->ExchangeID, 4);
    trade.FunctionCode = '0';
    memcpy_s(trade.InstrumentID, 9, cur_ord->InstrumentID, 9);
    memcpy_s(trade.MDStreamID, 4, cur_ord->MDStreamID, 4);
    trade.Price = match_lev->Price;
    trade.TradeGroupID = cur_ord->OrderGroupID; //假定与order组一样
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
    //2.更新除了最新成交价以外的市场信息
    Mkt.TotalTradeNumber++;
    Mkt.TotalTradeVol += trade.Volume;
    Mkt.NewestPrice = trade.Price;
    //3.检查新成交价格是否触发临停
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
    //假如该价格盘口不在买盘或卖盘里面，新建盘口并挂起
    if (Pool->empty() || Pool->find(someord.Price) == Pool->end())
    {
        Level newlev;
        newlev.Price = someord.Price;
        newlev.TotalVolume = someord.Volume;
        newlev.OrdList.emplace_back(someord);
        Pool->insert({ newlev.Price, newlev });
        return;
    }
    //如果该价格不在盘口里面
    else
    {
        auto it = Pool->find(someord.Price);
        it->second.OrdList.emplace_back(someord);
        it->second.TotalVolume += someord.Volume;
    }
}
void MarketManager::DealDroppedOrder(long long idx)
{
    //已经读到最后一笔成交单了
    if (txn_counter == Mkt.TxnCount)
        return;
    //成交单在current_order之前
    while (current_txn->TradeIndex <= idx)
    {
        if (current_txn->FunctionCode == 'F') //当前订单是成交单
            real_trades.emplace_back(*current_txn);
        else //当前txn是个撤单
        {
            if (current_txn->SellIndex == 0)//sell index为0说明要撤买单
                CancelOrd(current_txn->BuyIndex, idx2price[current_txn->BuyIndex], current_txn->Volume, BUYFLAG);
            else //if buyindex == 0 即要撤卖单
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
                Mkt.BestOffer = MAX; //初始值
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
        if (IsValidPrice(Mkt.BestOffer) == false) //如果卖盘出价很高,最低出价都高于最高有效范围,那Mkt.BestOffer为0
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
    return false; //默认等于为false,即严格大于才为真
}
bool MarketManager::CheckStopTrade()
{
    if (STOCKTYPE == ZB)    //如果是主板交易,没有临停机制,直接返回
        return false;
    //每得到一个新价格都要确定是否超过价格限制
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
//输出函数
void MarketManager::PrintTrades()
{
    for (auto it = trades.begin(); it != trades.end(); ++it)
    {
        std::cout << "订单号：" << it->TradeIndex << std::endl
            << "时间：" << it->TradeTime << std::endl
            << "价格：" << it->Price << std::endl
            << "手数：" << it->Volume << std::endl
            << "买方：" << it->BuyIndex << std::endl
            << "卖方：" << it->SellIndex << std::endl << std::endl;
    }
}
void MarketManager::PrintMktInfo()
{
    UpdateMktInfo();
    std::cout << order_buffer->InstrumentID << "\t";
    if (ord_counter < Mkt.OrdsCount)
        std::cout << current_ord->OrderTime << std::endl;
    std::cout << "昨收：" << Mkt.LstClose << "\t"
        << "今开：" << Mkt.TodayOpen << "\t"
        << "今收：" << Mkt.TodayClose << std::endl
        << "最新价: " << Mkt.NewestPrice << "\t"
        << "总成交手数：" << Mkt.TotalTradeVol << "\t"
        << "总成交单数：" << Mkt.TotalTradeNumber << std::endl;
    for (int i = 0; i < LEVELDISPLAY; i++)
    {
        std::cout << "买" << i + 1 << "价: " << Mkt.BidPrices[i] << "\t" <<
            "买" << i + 1 << "量: " << Mkt.BidVolumes[i] << "\t" <<
            "卖" << i + 1 << "价: " << Mkt.OfferPrices[i] << "\t" <<
            "卖" << i + 1 << "量: " << Mkt.OfferVolumes[i] << std::endl;
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
    level2f << "卖盘" << std::endl;
    level2f << "Price,Volume" << std::endl;
    for (auto it = offers_order_book.begin(); it != offers_order_book.end(); ++it)
        level2f << it->first << "," << it->second.TotalVolume << std::endl;
    level2f << "买盘" << std::endl;
    level2f << "Price,Volume" << std::endl;
    for (auto it = bids_order_book.begin(); it != bids_order_book.end(); ++it)
        level2f << it->first << "," << it->second.TotalVolume << std::endl;
}
bool MarketManager::CheckSimCorrectness()
{//比较每个成交单的买家、卖家、成交价格和手数
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
            std::cout << "第一个错单：\n"
                << "真实买家:" << real_trades_iter->BuyIndex << "\t撮合买家:" << trades_iter->BuyIndex << std::endl
                << "真实卖家:" << real_trades_iter->SellIndex << "\t撮合买家:" << trades_iter->SellIndex << std::endl
                << "真实手数:" << real_trades_iter->Volume << "\t撮合手数:" << trades_iter->Volume << std::endl
                << "真实价格:" << real_trades_iter->Price << "\t撮合价格:" << trades_iter->Price << std::endl;
            break;
        }
        trades_iter++;
        real_trades_iter++;
    }
    return same;
}
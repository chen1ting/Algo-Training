#include "HEADS.h"

int GetDates(std::string time, std::string ordpath, std::string* dates)
{
    int dates_count = 0;
    for (const auto& dirEntry : fs::directory_iterator(ordpath))
    {
        std::string tmp = dirEntry.path().filename().string();
        size_t i = 0;
        while (i < time.size() && tmp[i] == time[i]) //string matching
            i++;
        if (i == time.size())
            dates[dates_count++] = tmp;
    }
    return dates_count;
}
void GetInsIds(std::string ordpath, std::string txnpath, std::string mktpath)
{
    ins_id_cnt = 0;
    int counts[3] = { 0,0,0 };
    for (const auto& dirEntry : fs::directory_iterator(ordpath))
        counts[0]++;
    for (const auto& dirEntry : fs::directory_iterator(txnpath))
        counts[1]++;
    for (const auto& dirEntry : fs::directory_iterator(mktpath))
        counts[2]++;
    //如果order和trade文件的优先级高于mkt，如果两者文件夹下的文件数量不相等，说明有order没txn或者有txn没order，需要处理
    if (counts[0] != counts[1])
    {
        std::cout << "order和trade文件数量不相等，以数量少的为准\n";
        for (const auto& dirEntry : fs::directory_iterator(counts[0] > counts[1] ? txnpath : ordpath))   //把文件夹中的第一天作为sampling date
            ins_file_names[ins_id_cnt++] = dirEntry.path().filename().string();
    }
    else
    {
        for (const auto& dirEntry : fs::directory_iterator(ordpath))   //把文件夹中的第一天作为sampling date
            ins_file_names[ins_id_cnt++] = dirEntry.path().filename().string();
    }
}
void ReadOrd(std::string ordfilepath, ORDBUFF* slot, int PID)
{
    while (ord_read_ptr < ins_id_cnt)
    {
        if (slot->empty) //如果没有读完,且slot为空,读
        {
            slot->fileidx = ord_read_ptr++;//指针移向该slot需要处理的合约
            std::ifstream order_input;
            order_input.open(ordfilepath + ins_file_names[slot->fileidx], std::ios::in | std::ios::binary);
            if (order_input.is_open() && order_input.tellg() % sizeof(ORDER_STRUCT) == 0)
            {
                order_input.seekg(0, std::ios::end);
                slot->cnt = order_input.tellg() / sizeof(ORDER_STRUCT); //初始化Count,用于记录合理的数据范围
                order_input.seekg(0, std::ios::beg);
                order_input.read((char*)slot->buf, sizeof(ORDER_STRUCT) * (slot->cnt));
                slot->groupid = slot->buf->OrderGroupID;
                slot->empty = false; //该buffer slot已被写入
            }
            order_input.close();
        }
        Sleep(1); //阻塞
    }
}
void ReadTxn(std::string txnfilepath, TXNBUFF* slot, int PID)
{
    while (txn_read_ptr < ins_id_cnt)
    {

        if (slot->empty)                 //如果未读完且当前slot为空,读
        {
            slot->fileidx = txn_read_ptr++;
            //读取成交单存到slot中
            std::ifstream trade_input;
            trade_input.open(txnfilepath + ins_file_names[slot->fileidx], std::ios::in | std::ios::binary);
            if (trade_input.is_open() && trade_input.tellg() % sizeof(TRADE_STRUCT) == 0)
            {
                trade_input.seekg(0, std::ios::end);
                slot->cnt = trade_input.tellg() / sizeof(TRADE_STRUCT); //初始化Count,用于记录合理的数据范围
                trade_input.seekg(0, std::ios::beg);
                trade_input.read((char*)slot->buf, sizeof(TRADE_STRUCT) * slot->cnt);
                slot->groupid = slot->buf->TradeGroupID;
                slot->empty = false;
            }
            trade_input.close();
        }
        Sleep(1);
    }
}
void ReadMkt(std::string mktfilepath)
{
    while (mkt_read_ptr < ins_id_cnt)
    {
        //auto start_timing = std::chrono::high_resolution_clock::now(); //timing
        std::ifstream LstMktIn;
        int now_read = mkt_read_ptr++;
        LstMktIn.open(mktfilepath + ins_file_names[now_read], std::ios::in | std::ios::binary);
        if (!LstMktIn.is_open() || LstMktIn.tellg() % sizeof(ORDER_STRUCT) != 0)
        {//如果mkt文件未能正常打开,将相应的trade_volumes设成-1,continue while循环
            read_vol_mtx.lock();
            trade_volumes[now_read] = -1;
            read_vol_mtx.unlock();
            continue;
        }
        LstMktIn.seekg(0, std::ios::end);
        int count = LstMktIn.tellg() / sizeof(MARKET_STRUCT);
        if (ins_file_names[mkt_read_ptr][0] == '6' && ins_file_names[now_read][1] == '8')
        { //上海科创板股票，有盘后信息(盘后信息买二和卖二盘口皆空)
            int KCB_Read_Last = count < 100 ? count : 100;
            MARKET_STRUCT* mkt_buffer = (MARKET_STRUCT*)malloc(sizeof(MARKET_STRUCT) * KCB_Read_Last);
            LstMktIn.seekg((count - KCB_Read_Last) * sizeof(MARKET_STRUCT), std::ios::beg);
            LstMktIn.read((char*)mkt_buffer, sizeof(MARKET_STRUCT) * KCB_Read_Last);
            for (int i = KCB_Read_Last - 1; i >= 0; i--) //从后往前遍历
            {
                if ((mkt_buffer + i)->InstrumentStatus[0] == 'C' && (mkt_buffer + i)->TotalTradeVolume > 0) //InstrumentStatus == "CLOSE"
                {
                    read_vol_mtx.lock();
                    trade_volumes[now_read] = (mkt_buffer + i)->TotalTradeVolume;
                    read_vol_mtx.unlock();
                    break;
                }
            }
            free(mkt_buffer);
        }
        else
        {
            MARKET_STRUCT lst_mkt;
            LstMktIn.seekg((count - 1) * sizeof(MARKET_STRUCT), std::ios::beg);    //LstMktIn.seekg(0 - sizeof(MARKET_STRUCT), std::ios::end);
            LstMktIn.read((char*)&lst_mkt, sizeof(MARKET_STRUCT));
            trade_volumes[now_read] = lst_mkt.TotalTradeVolume;
        }

        LstMktIn.close();
        //auto stop_timing = std::chrono::high_resolution_clock::now();
        //auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop_timing - start_timing);
        //mtx.lock();
        //std::cout << ins_file_names[slot->fileidx] << " Read Trade Time: " << duration.count() << std::endl;
        //mtx.unlock();
    }
}
void DealOrd(int start, int end, int PID)
{
    while (ord_dealed < ins_id_cnt)//已处理的订单少于总的订单合约数量时,继续处理
    {
        for (int i = start; i < end; i++) //轮询read_slot
        {
            if (!(ordbufs + i)->empty)//如果有已读未处理的合约
            {
                ORDBUFF* buf_ptr = ordbufs + i; //用指针指向该待处理slot
                unsigned int* bitmap_ptr = bitmaps[bmap_hash[buf_ptr->groupid]];//根据ordergroupID,找到当前合约所属组别的bitmap
                //因为检查order时需要用到的只有idx,所以用idx指针指向第一个的orderindex或者bizindex,每次读取下一个时,直接跳跃一个结构体的长度,避免每处理一个订单都需要重新判断使用order还是bizindex
                long long* idx_ptr = buf_ptr->buf->ExchangeID[1] == 'Z' ? &buf_ptr->buf->OrderIndex : &buf_ptr->buf->BizIndex;
                //将第一个订单的idx写入bitmap
                bitmap_write_mtx.lock();
                SetBitPosTo1(*idx_ptr, bitmap_ptr);
                bitmap_write_mtx.unlock();
                for (int i = 1; i < buf_ptr->cnt; i++)
                {//从第二个订单开始处理
                    long long* tmpval_1 = (long long*)((unsigned char*)idx_ptr + (i - 1) * ORDSTRUCT_SIZE);//指向前一个订单的idx
                    long long* tmpval_2 = (long long*)((unsigned char*)idx_ptr + i * ORDSTRUCT_SIZE);//指向当前订单的idx
                    //if (*tmpval_1 == 100 || *tmpval_2 == 100)
                    //    printf("Deal Ord, tmpval_1: %d, tmpval_2: %d, instrument: %s", *tmpval_1, *tmpval_2, ins_file_names[buf_ptr->fileidx]);
                    if (*tmpval_1 >= *tmpval_2)//如果前一个idx大于等于当前的,说明乱序,需要报错
                    {
                        std::vector<long long> tmp;
                        tmp.push_back(*tmpval_1);
                        tmp.push_back(*tmpval_2);
                        wrong_ord_mtx.lock();
                        wrong_sequence_ord.insert({ buf_ptr->fileidx, tmp });
                        wrong_ord_mtx.unlock();
                    }
                    //将当前idx写入bitmap
                    bitmap_write_mtx.lock();
                    SetBitPosTo1(*tmpval_2, bitmap_ptr);
                    bitmap_write_mtx.unlock();
                }
                ord_dealed++;//已处理的合约数+1
                buf_ptr->empty = true;//将当前的slot设为可写状态
            }
        }
        Sleep(1);   //全空或轮询完一波之后阻塞
    }
}
void DealTxn(int start, int end, int PID)
{
    while (txn_dealed < ins_id_cnt)
    {//在所有合约的txn还没有处理完之前,持续处理
        //轮询slot
        for (int i = start; i < end; i++)
        {
            if (!(txnbufs + i)->empty) //如果碰到一个slot非空(待处理)
            {
                TXNBUFF* buf_ptr = txnbufs + i; //创建一个指针指向该slot
                unsigned int* bitmap_ptr = bitmaps[bmap_hash[buf_ptr->groupid]];//通过slot的group id获取bitmap的位置
                //依然使用指针跳跃指向idx避免重复判断
                long long* idx_ptr = buf_ptr->buf->ExchangeID[1] == 'Z' ? &buf_ptr->buf->TradeIndex : &buf_ptr->buf->BizIndex;
                double sum = 0;//用于记录当前trade的成交总数
                //把第一个成交单的trade index写入bitmap
                bitmap_write_mtx.lock(); 
                SetBitPosTo1(*idx_ptr, bitmap_ptr);
                bitmap_write_mtx.unlock();
                if (buf_ptr->buf->FunctionCode != '4')
                    sum += buf_ptr->buf->Volume;
                for (int i = 1; i < buf_ptr->cnt; i++)
                {
                    long long* tmpval_1 = (long long*)((unsigned char*)idx_ptr + (i - 1) * TXNSTRUCT_SIZE);//指向前一个订单的idx
                    long long* tmpval_2 = (long long*)((unsigned char*)idx_ptr + i * TXNSTRUCT_SIZE);//指向当前订单的idx
                    //检查是否乱序
                    //if (*tmpval_1 == 100 || *tmpval_2 == 100)
                    //    printf("Deal Txn, tmpval_1: %d, tmpval_2: %d, instrument: %s", *tmpval_1, *tmpval_2, ins_file_names[buf_ptr->fileidx]);
                    if (*tmpval_1 >= *tmpval_2)//如果前一个idx大于等于当前的,说明乱序,需要报错
                    {
                        std::vector<long long> tmp;
                        tmp.push_back(*tmpval_1);
                        tmp.push_back(*tmpval_2);
                        wrong_ord_mtx.lock();
                        wrong_sequence_ord.insert({ buf_ptr->fileidx, tmp });
                        wrong_ord_mtx.unlock();
                    }
                    //trade volume加和
                    if ((buf_ptr->buf + i)->FunctionCode != '4')//如果一个订单不是撤单,将其volume加入总数
                        sum += (buf_ptr->buf + i)->Volume;
                    //把当前订单的idx写入bitmap
                    bitmap_write_mtx.lock();
                    SetBitPosTo1(*tmpval_2, bitmap_ptr);
                    bitmap_write_mtx.unlock();
                }
                //将处理txn过程中加和的结果记录到一个数组里面,等到最后统一检查
                txn_vol_mtx.lock();
                sim_volumes[buf_ptr->fileidx] = sum;
                txn_vol_mtx.unlock();
                txn_dealed++;//处理的txn数量+1
                buf_ptr->empty = true;//将slot状态改为可写
            }
        }
        Sleep(1);
    }
}

inline void SetBitPosTo1(long long n_index, unsigned int* map)
{
    n_index--;
    int idx = n_index / 32;     //写入第几个int
    int pos = n_index % 32;     //写入该int的第几个bit
    map[idx] = map[idx] ^ (1 << pos);
}
bool CheckMissingOrd()
{
    unsigned int* bitmap_ptr;//指向当前在检查的位图
    bool all_group_ok = true;//如果所有组别的订单都没有问题,输出true
    for (int map_ptr = 0; map_ptr < 20; map_ptr++)
    {
        bitmap_ptr = bitmaps[map_ptr];
        for (int num_ptr = 0; num_ptr<BITMAPSIZE; num_ptr++)
        {
            if (bitmap_ptr[num_ptr] != UINT_MAX)    //如果有一个数字非全1
            {
                bool end = true;//检查当前数字是否因为是最后一位而非全1
                for (int nxtnums = num_ptr + 1; nxtnums < BITMAPSIZE && nxtnums < num_ptr + 5; nxtnums++)
                {//如果该数字之后的5个数字都为0(即接下来的5*32=160的订单号都未出现),则是end=true,否则视end=false;
                    if (bitmap_ptr[nxtnums] != 0)
                    {
                        all_group_ok = false;
                        end = false;
                        break;
                    }
                }
                if (end == true) //如果该订单是最后一个,检查当前数字是否有丢失的单子
                {//如果没有丢失的订单,数字应该是0000000011111(缺的单子都在最前面)
                    unsigned int last = bitmap_ptr[num_ptr];
                    while (last % 2 == 1)    //最后一位是1
                        last >>= 1; //向右移一位
                    if (last == 0)  //说明这是最后一个单子,直接去检查下一合约
                        break;
                }
                unsigned int missing = bitmap_ptr[num_ptr];
                for (int i = 0; i < 32; i++)
                {
                    if (missing % 2 == 0)//最后一位是1
                        missing_ord[map_ptr].push_back(num_ptr * 32 + i + 1);
                    missing >>= 1;
                }
            }
        }
    }
    if (all_group_ok)
        std::cout << "bitmap检查无中间丢失的订单\n";
    else
        PrintMissingOrd();
    return all_group_ok;
}
void PrintMissingOrd()
{
    for (auto it = missing_ord.begin(); it != missing_ord.end(); ++it)
    {
        std::cout << groupids_string[it->first] << "失踪订单:\n";
        for (auto idx : it->second)
            std::cout << idx << "\t";
        std::cout << std::endl;
        it->second.clear();
    }
    missing_ord.clear();
}
bool CheckTradeVol()
{
    bool all_correct = true;
    std::vector<std::string> unopen;
    for (int i = 0; i < ins_id_cnt; i++)
    {
        if (trade_volumes[i] == -1)
            unopen.push_back(ins_file_names[i]);
        else if (trade_volumes[i] != sim_volumes[i])
        {
            std::cout << ins_file_names[i] << "成交额不符合\t" << "真实: " << trade_volumes[i] << "\t加和: " << sim_volumes[i] << std::endl;
            all_correct = false;
        }
    }
    if (!unopen.empty())
    {
        std::cout << "以下mkt文件未能正确打开\n";
        for (auto fname : unopen)
            std::cout << fname << "\t";
    }
    return all_correct;
}
bool CheckWrongSeq()
{
    if (wrong_sequence_ord.empty())
        return true;
    for (auto wrongins : wrong_sequence_ord)
    {
        std::cout << ins_file_names[wrongins.first] << "乱序订单:\n";
        for (auto idx : wrongins.second)
            std::cout << idx << "\t";
        std::cout << std::endl;
    }
    wrong_sequence_ord.clear();
    return false;
}
void ProgressBar(int line)
{
    std::string name[3] = { "ORD: ","TXN: ","MKT: " };
    destCoord.X = 0;
    destCoord.Y = line;
    while (true)
    {
        float progresses[3] = { float(ord_read_ptr) / ins_id_cnt ,  float(txn_read_ptr) / ins_id_cnt, float(mkt_read_ptr) / ins_id_cnt };
        SetConsoleCursorPosition(hStdout, destCoord);
        for (int i = 0; i < 3; i++) {
            int barWidth = 70;
            std::cout << name[i] << "[";
            int pos = barWidth * progresses[i];
            for (int j = 0; j < barWidth; ++j) {
                if (j < pos) std::cout << "=";
                else if (j == pos) std::cout << ">";
                else std::cout << " ";
            }
            printf("] %.3lf%%   %d/%d\n", progresses[i] * 100.0, int(progresses[i] * ins_id_cnt), ins_id_cnt);
        }  
        Sleep(5000);
        if (ord_read_ptr >= ins_id_cnt - 10 && txn_read_ptr >= ins_id_cnt - 10 && mkt_read_ptr >= ins_id_cnt - 10)
            return;
    }
}
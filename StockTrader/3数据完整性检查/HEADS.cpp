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
    //���order��trade�ļ������ȼ�����mkt����������ļ����µ��ļ���������ȣ�˵����orderûtxn������txnûorder����Ҫ����
    if (counts[0] != counts[1])
    {
        std::cout << "order��trade�ļ���������ȣ��������ٵ�Ϊ׼\n";
        for (const auto& dirEntry : fs::directory_iterator(counts[0] > counts[1] ? txnpath : ordpath))   //���ļ����еĵ�һ����Ϊsampling date
            ins_file_names[ins_id_cnt++] = dirEntry.path().filename().string();
    }
    else
    {
        for (const auto& dirEntry : fs::directory_iterator(ordpath))   //���ļ����еĵ�һ����Ϊsampling date
            ins_file_names[ins_id_cnt++] = dirEntry.path().filename().string();
    }
}
void ReadOrd(std::string ordfilepath, ORDBUFF* slot, int PID)
{
    while (ord_read_ptr < ins_id_cnt)
    {
        if (slot->empty) //���û�ж���,��slotΪ��,��
        {
            slot->fileidx = ord_read_ptr++;//ָ�������slot��Ҫ����ĺ�Լ
            std::ifstream order_input;
            order_input.open(ordfilepath + ins_file_names[slot->fileidx], std::ios::in | std::ios::binary);
            if (order_input.is_open() && order_input.tellg() % sizeof(ORDER_STRUCT) == 0)
            {
                order_input.seekg(0, std::ios::end);
                slot->cnt = order_input.tellg() / sizeof(ORDER_STRUCT); //��ʼ��Count,���ڼ�¼��������ݷ�Χ
                order_input.seekg(0, std::ios::beg);
                order_input.read((char*)slot->buf, sizeof(ORDER_STRUCT) * (slot->cnt));
                slot->groupid = slot->buf->OrderGroupID;
                slot->empty = false; //��buffer slot�ѱ�д��
            }
            order_input.close();
        }
        Sleep(1); //����
    }
}
void ReadTxn(std::string txnfilepath, TXNBUFF* slot, int PID)
{
    while (txn_read_ptr < ins_id_cnt)
    {

        if (slot->empty)                 //���δ�����ҵ�ǰslotΪ��,��
        {
            slot->fileidx = txn_read_ptr++;
            //��ȡ�ɽ����浽slot��
            std::ifstream trade_input;
            trade_input.open(txnfilepath + ins_file_names[slot->fileidx], std::ios::in | std::ios::binary);
            if (trade_input.is_open() && trade_input.tellg() % sizeof(TRADE_STRUCT) == 0)
            {
                trade_input.seekg(0, std::ios::end);
                slot->cnt = trade_input.tellg() / sizeof(TRADE_STRUCT); //��ʼ��Count,���ڼ�¼��������ݷ�Χ
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
        {//���mkt�ļ�δ��������,����Ӧ��trade_volumes���-1,continue whileѭ��
            read_vol_mtx.lock();
            trade_volumes[now_read] = -1;
            read_vol_mtx.unlock();
            continue;
        }
        LstMktIn.seekg(0, std::ios::end);
        int count = LstMktIn.tellg() / sizeof(MARKET_STRUCT);
        if (ins_file_names[mkt_read_ptr][0] == '6' && ins_file_names[now_read][1] == '8')
        { //�Ϻ��ƴ����Ʊ�����̺���Ϣ(�̺���Ϣ����������̿ڽԿ�)
            int KCB_Read_Last = count < 100 ? count : 100;
            MARKET_STRUCT* mkt_buffer = (MARKET_STRUCT*)malloc(sizeof(MARKET_STRUCT) * KCB_Read_Last);
            LstMktIn.seekg((count - KCB_Read_Last) * sizeof(MARKET_STRUCT), std::ios::beg);
            LstMktIn.read((char*)mkt_buffer, sizeof(MARKET_STRUCT) * KCB_Read_Last);
            for (int i = KCB_Read_Last - 1; i >= 0; i--) //�Ӻ���ǰ����
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
    while (ord_dealed < ins_id_cnt)//�Ѵ���Ķ��������ܵĶ�����Լ����ʱ,��������
    {
        for (int i = start; i < end; i++) //��ѯread_slot
        {
            if (!(ordbufs + i)->empty)//������Ѷ�δ����ĺ�Լ
            {
                ORDBUFF* buf_ptr = ordbufs + i; //��ָ��ָ��ô�����slot
                unsigned int* bitmap_ptr = bitmaps[bmap_hash[buf_ptr->groupid]];//����ordergroupID,�ҵ���ǰ��Լ��������bitmap
                //��Ϊ���orderʱ��Ҫ�õ���ֻ��idx,������idxָ��ָ���һ����orderindex����bizindex,ÿ�ζ�ȡ��һ��ʱ,ֱ����Ծһ���ṹ��ĳ���,����ÿ����һ����������Ҫ�����ж�ʹ��order����bizindex
                long long* idx_ptr = buf_ptr->buf->ExchangeID[1] == 'Z' ? &buf_ptr->buf->OrderIndex : &buf_ptr->buf->BizIndex;
                //����һ��������idxд��bitmap
                bitmap_write_mtx.lock();
                SetBitPosTo1(*idx_ptr, bitmap_ptr);
                bitmap_write_mtx.unlock();
                for (int i = 1; i < buf_ptr->cnt; i++)
                {//�ӵڶ���������ʼ����
                    long long* tmpval_1 = (long long*)((unsigned char*)idx_ptr + (i - 1) * ORDSTRUCT_SIZE);//ָ��ǰһ��������idx
                    long long* tmpval_2 = (long long*)((unsigned char*)idx_ptr + i * ORDSTRUCT_SIZE);//ָ��ǰ������idx
                    //if (*tmpval_1 == 100 || *tmpval_2 == 100)
                    //    printf("Deal Ord, tmpval_1: %d, tmpval_2: %d, instrument: %s", *tmpval_1, *tmpval_2, ins_file_names[buf_ptr->fileidx]);
                    if (*tmpval_1 >= *tmpval_2)//���ǰһ��idx���ڵ��ڵ�ǰ��,˵������,��Ҫ����
                    {
                        std::vector<long long> tmp;
                        tmp.push_back(*tmpval_1);
                        tmp.push_back(*tmpval_2);
                        wrong_ord_mtx.lock();
                        wrong_sequence_ord.insert({ buf_ptr->fileidx, tmp });
                        wrong_ord_mtx.unlock();
                    }
                    //����ǰidxд��bitmap
                    bitmap_write_mtx.lock();
                    SetBitPosTo1(*tmpval_2, bitmap_ptr);
                    bitmap_write_mtx.unlock();
                }
                ord_dealed++;//�Ѵ���ĺ�Լ��+1
                buf_ptr->empty = true;//����ǰ��slot��Ϊ��д״̬
            }
        }
        Sleep(1);   //ȫ�ջ���ѯ��һ��֮������
    }
}
void DealTxn(int start, int end, int PID)
{
    while (txn_dealed < ins_id_cnt)
    {//�����к�Լ��txn��û�д�����֮ǰ,��������
        //��ѯslot
        for (int i = start; i < end; i++)
        {
            if (!(txnbufs + i)->empty) //�������һ��slot�ǿ�(������)
            {
                TXNBUFF* buf_ptr = txnbufs + i; //����һ��ָ��ָ���slot
                unsigned int* bitmap_ptr = bitmaps[bmap_hash[buf_ptr->groupid]];//ͨ��slot��group id��ȡbitmap��λ��
                //��Ȼʹ��ָ����Ծָ��idx�����ظ��ж�
                long long* idx_ptr = buf_ptr->buf->ExchangeID[1] == 'Z' ? &buf_ptr->buf->TradeIndex : &buf_ptr->buf->BizIndex;
                double sum = 0;//���ڼ�¼��ǰtrade�ĳɽ�����
                //�ѵ�һ���ɽ�����trade indexд��bitmap
                bitmap_write_mtx.lock(); 
                SetBitPosTo1(*idx_ptr, bitmap_ptr);
                bitmap_write_mtx.unlock();
                if (buf_ptr->buf->FunctionCode != '4')
                    sum += buf_ptr->buf->Volume;
                for (int i = 1; i < buf_ptr->cnt; i++)
                {
                    long long* tmpval_1 = (long long*)((unsigned char*)idx_ptr + (i - 1) * TXNSTRUCT_SIZE);//ָ��ǰһ��������idx
                    long long* tmpval_2 = (long long*)((unsigned char*)idx_ptr + i * TXNSTRUCT_SIZE);//ָ��ǰ������idx
                    //����Ƿ�����
                    //if (*tmpval_1 == 100 || *tmpval_2 == 100)
                    //    printf("Deal Txn, tmpval_1: %d, tmpval_2: %d, instrument: %s", *tmpval_1, *tmpval_2, ins_file_names[buf_ptr->fileidx]);
                    if (*tmpval_1 >= *tmpval_2)//���ǰһ��idx���ڵ��ڵ�ǰ��,˵������,��Ҫ����
                    {
                        std::vector<long long> tmp;
                        tmp.push_back(*tmpval_1);
                        tmp.push_back(*tmpval_2);
                        wrong_ord_mtx.lock();
                        wrong_sequence_ord.insert({ buf_ptr->fileidx, tmp });
                        wrong_ord_mtx.unlock();
                    }
                    //trade volume�Ӻ�
                    if ((buf_ptr->buf + i)->FunctionCode != '4')//���һ���������ǳ���,����volume��������
                        sum += (buf_ptr->buf + i)->Volume;
                    //�ѵ�ǰ������idxд��bitmap
                    bitmap_write_mtx.lock();
                    SetBitPosTo1(*tmpval_2, bitmap_ptr);
                    bitmap_write_mtx.unlock();
                }
                //������txn�����мӺ͵Ľ����¼��һ����������,�ȵ����ͳһ���
                txn_vol_mtx.lock();
                sim_volumes[buf_ptr->fileidx] = sum;
                txn_vol_mtx.unlock();
                txn_dealed++;//�����txn����+1
                buf_ptr->empty = true;//��slot״̬��Ϊ��д
            }
        }
        Sleep(1);
    }
}

inline void SetBitPosTo1(long long n_index, unsigned int* map)
{
    n_index--;
    int idx = n_index / 32;     //д��ڼ���int
    int pos = n_index % 32;     //д���int�ĵڼ���bit
    map[idx] = map[idx] ^ (1 << pos);
}
bool CheckMissingOrd()
{
    unsigned int* bitmap_ptr;//ָ��ǰ�ڼ���λͼ
    bool all_group_ok = true;//����������Ķ�����û������,���true
    for (int map_ptr = 0; map_ptr < 20; map_ptr++)
    {
        bitmap_ptr = bitmaps[map_ptr];
        for (int num_ptr = 0; num_ptr<BITMAPSIZE; num_ptr++)
        {
            if (bitmap_ptr[num_ptr] != UINT_MAX)    //�����һ�����ַ�ȫ1
            {
                bool end = true;//��鵱ǰ�����Ƿ���Ϊ�����һλ����ȫ1
                for (int nxtnums = num_ptr + 1; nxtnums < BITMAPSIZE && nxtnums < num_ptr + 5; nxtnums++)
                {//���������֮���5�����ֶ�Ϊ0(����������5*32=160�Ķ����Ŷ�δ����),����end=true,������end=false;
                    if (bitmap_ptr[nxtnums] != 0)
                    {
                        all_group_ok = false;
                        end = false;
                        break;
                    }
                }
                if (end == true) //����ö��������һ��,��鵱ǰ�����Ƿ��ж�ʧ�ĵ���
                {//���û�ж�ʧ�Ķ���,����Ӧ����0000000011111(ȱ�ĵ��Ӷ�����ǰ��)
                    unsigned int last = bitmap_ptr[num_ptr];
                    while (last % 2 == 1)    //���һλ��1
                        last >>= 1; //������һλ
                    if (last == 0)  //˵���������һ������,ֱ��ȥ�����һ��Լ
                        break;
                }
                unsigned int missing = bitmap_ptr[num_ptr];
                for (int i = 0; i < 32; i++)
                {
                    if (missing % 2 == 0)//���һλ��1
                        missing_ord[map_ptr].push_back(num_ptr * 32 + i + 1);
                    missing >>= 1;
                }
            }
        }
    }
    if (all_group_ok)
        std::cout << "bitmap������м䶪ʧ�Ķ���\n";
    else
        PrintMissingOrd();
    return all_group_ok;
}
void PrintMissingOrd()
{
    for (auto it = missing_ord.begin(); it != missing_ord.end(); ++it)
    {
        std::cout << groupids_string[it->first] << "ʧ�ٶ���:\n";
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
            std::cout << ins_file_names[i] << "�ɽ������\t" << "��ʵ: " << trade_volumes[i] << "\t�Ӻ�: " << sim_volumes[i] << std::endl;
            all_correct = false;
        }
    }
    if (!unopen.empty())
    {
        std::cout << "����mkt�ļ�δ����ȷ��\n";
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
        std::cout << ins_file_names[wrongins.first] << "���򶩵�:\n";
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
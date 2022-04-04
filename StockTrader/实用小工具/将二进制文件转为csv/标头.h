#pragma once
#pragma pack(push)
#pragma pack(1)

struct	ORDER_STRUCT //ORDER�ṹ��
{
	double		Price;					// ί�м۸�
	double		Volume;					// ί������
	int			OrderGroupID;			// ί����		//	sh1-6		sz2011-
	long long	OrderIndex;				// ί�����		//	int			OrderIndex;					// ί�����
	char		OrderKind;				// ��������		//  sh A D		sz 2�޼� 1�м� U����
	char		FunctionCode;			// ������		//  sh B S		1 2
	char		ExchangeID[4];			// ����������	//	SSE.		SZE.
	char		InstrumentID[9];		// ��Լ����
	char		OrderTime[13];			// ʱ���
	char		MDStreamID[4];			// �������		//  sh"   "		sz"011" "021"
	long long	BizIndex;				// ҵ�����		//	char		UpdateListFlag;				// ���¶��б�־
};

struct TRADE_STRUCT //�ɽ����ṹ��
{
	double		Price;					// �ɽ��۸�		
	double		Volume;					// �ɽ�����		
	int			TradeGroupID;			// �ɽ���					// sh����ǰ�ɽṹ��
	long long	TradeIndex;				// �ɽ����					//	int		TradeIndex;			// �ɽ����    
	long long	BuyIndex;				// ��ί�����				//	int		BuyIndex;			// ��ί�����  
	long long	SellIndex;				// ����ί�����				//	int		SellIndex;			// ����ί�����
	char		FunctionCode;			// ������		sh0 szF4	//	char	OrderKind;			// ��������    
	char		ExchangeID[4];			// ����������				//	char	FunctionCode;		// ������      
	char		InstrumentID[9];		// ��Լ����					//	char	ExchangeID[4];		// ����������
	char		TradeTime[13];			// ʱ���					//	char	InstrumentID[9];	// ��Լ����  
	char		OrderBSFlag;			// �����̱�־	shBS szN	//	char	TradeTime[13];		// ʱ���    
	char		MDStreamID[4];			// �������					//	char	OrderBSFlag;		// �����̱�־  
	long long	BizIndex;				// ҵ�����					//	char	MDStreamID[4];		// �������  
};

struct MARKET_STRUCT //�г����ݽṹ��
{
	double		LastPrice;				// ���¼�
	double		OpenPrice;				// ���̼�
	double		ClosePrice;				// ���̼�
	double		HighPrice;				// ��߼�
	double		LowPrice;				// ��ͼ�
	double		TotalTradeVolume;		// �ɽ�����
	double		TotalTradeValue;		// �ɽ����
	double		TradeCount;				// �ɽ�����
	double		OpenInterest;			// �ֲ���
	double		IOPV;					// ����ֵ
	double		YieldToMaturity;		// ����������
	double		AuctionPrice;			// ��̬�ο��۸�
	double		TotalBidVolume;			// ��������
	double		WeightedAvgBidPrice;	// �����Ȩ����
	double		AltWeightedAvgBidPrice;		// ծȯ�����Ȩ����
	double		TotalOfferVolume;		// ��������
	double		WeightedAvgOfferPrice;	// ������Ȩ����
	double		AltWeightedAvgOfferPrice;	// ծȯ������Ȩ����
	int			BidPriceLevel;			// ������
	int			OfferPriceLevel;		// �������
	double		BidPrice1;				// �����һ
	double		BidVolume1;				// ������һ
	double		BidCount1;				// �������һ
	double		OfferPrice1;			// ������һ
	double		OfferVolume1;			// ������һ
	double		OfferCount1;			// ��������һ
	double		BidPrice2;				// ����۶�
	double		BidVolume2;				// ��������
	double		BidCount2;				// ���������
	double		OfferPrice2;			// �����۶�
	double		OfferVolume2;			// ��������
	double		OfferCount2;			// ����������
	double		BidPrice3;				// �������
	double		BidVolume3;				// ��������
	double		BidCount3;				// ���������
	double		OfferPrice3;			// ��������
	double		OfferVolume3;			// ��������
	double		OfferCount3;			// ����������
	double		BidPrice4;				// �������
	double		BidVolume4;				// ��������
	double		BidCount4;				// ���������
	double		OfferPrice4;			// ��������
	double		OfferVolume4;			// ��������
	double		OfferCount4;			// ����������
	double		BidPrice5;				// �������
	double		BidVolume5;				// ��������
	double		BidCount5;				// ���������
	double		OfferPrice5;			// ��������
	double		OfferVolume5;			// ��������
	double		OfferCount5;			// ����������
	double		BidPrice6;				// �������
	double		BidVolume6;				// ��������
	double		BidCount6;				// ���������
	double		OfferPrice6;			// ��������
	double		OfferVolume6;			// ��������
	double		OfferCount6;			// ����������
	double		BidPrice7;				// �������
	double		BidVolume7;				// ��������
	double		BidCount7;				// ���������
	double		OfferPrice7;			// ��������
	double		OfferVolume7;			// ��������
	double		OfferCount7;			// ����������
	double		BidPrice8;				// ����۰�
	double		BidVolume8;				// ��������
	double		BidCount8;				// ���������
	double		OfferPrice8;			// �����۰�
	double		OfferVolume8;			// ��������
	double		OfferCount8;			// ����������
	double		BidPrice9;				// ����۾�
	double		BidVolume9;				// ��������
	double		BidCount9;				// ���������
	double		OfferPrice9;			// �����۾�
	double		OfferVolume9;			// ��������
	double		OfferCount9;			// ����������
	double		BidPriceA;				// �����ʮ
	double		BidVolumeA;				// ������ʮ
	double		BidCountA;				// �������ʮ
	double		OfferPriceA;			// ������ʮ
	double		OfferVolumeA;			// ������ʮ
	double		OfferCountA;			// ��������ʮ
	char		ExchangeID[4];			// ����������
	char		InstrumentID[9];		// ֤ȯ����
	char		TimeStamp[13];			// ʱ���
	char		TradingPhase;			// ���׽׶�
	char		OpenRestriction;		// ��������
	char		MDStreamID[4];			// �������
	char		InstrumentStatus[7];	// ��Լ״̬
	double		PreIOPV;				// �����ֵ
	double		PERatio1;				// ��ӯ��һ
	double		PERatio2;				// ��ӯ�ʶ�
	double		UpperLimitPrice;		// ��ͣ��
	double		LowerLimitPrice;		// ��ͣ��
	double		WarrantPremiumRatio;	// Ȩ֤�����
	double		TotalWarrantExecQty;	// Ȩִ֤��������
	double		PriceDiff1;				// ����һ
	double		PriceDiff2;				// ������
	double		ETFBuyNumber;			// ETF�깺����
	double		ETFBuyAmount;			// ETF�깺����
	double		ETFBuyMoney;			// ETF�깺���
	double		ETFSellNumber;			// ETF��ر���
	double		ETFSellAmount;			// ETF�������
	double		ETFSellMoney;			// ETF��ؽ��
	double		WithdrawBuyNumber;		// ���볷������
	double		WithdrawBuyAmount;		// ���볷������
	double		WithdrawBuyMoney;		// ���볷�����
	double		TotalBidNumber;			// �����ܱ���
	double		BidTradeMaxDuration;	// ����ί�гɽ����ȴ�ʱ��
	double		NumBidOrders;			// ��ί�м�λ��
	double		WithdrawSellNumber;		// ������������
	double		WithdrawSellAmount;		// ������������
	double		WithdrawSellMoney;		// �����������
	double		TotalOfferNumber;		// �����ܱ���
	double		OfferTradeMaxDuration;	// ����ί�гɽ����ȴ�ʱ��
	double		NumOfferOrders;			// ����ί�м�λ��
};
#pragma pack(pop)
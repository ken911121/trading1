#include "stdafx.h"
#include "MdSpi.h"
#include <iostream>
#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>
using namespace std;
#pragma warning(disable : 4996)

extern std::condition_variable rb_cv;
extern std::condition_variable io_cv;

extern CThostFtdcMdApi* pUserApi;

// ���ò���
extern char FRONT_ADDR[];
extern TThostFtdcBrokerIDType	BROKER_ID;
extern TThostFtdcInvestorIDType INVESTOR_ID;
extern TThostFtdcPasswordType	PASSWORD;
extern char* ppInstrumentID[];
extern int iInstrumentID;

extern std::vector<min_data> rb01;
extern std::vector<min_data> rb11;
extern std::vector<min_data> io01;
extern std::vector<min_data> io11;
//���ݵı���
extern std::vector<min_data> rb02;
extern std::vector<min_data> rb12;
extern std::vector<min_data> io02;
extern std::vector<min_data> io12;
//�Ƿ�ʹ�ñ���
extern bool rb_is_beifen;
extern bool io_is_beifen;
//�Ƿ��ǵ�һ�θ���
extern std::vector<bool> is_first;
extern min_data rb0;
extern min_data rb1;
extern min_data io0;
extern min_data io1;


// ������
extern int iRequestID;

void MdSpi::OnFrontConnected()
{
	cout << "���ӳɹ�" << endl;
	ReqUserLogin();
}

///���ͻ����뽻�׺�̨ͨ�����ӶϿ�ʱ���÷��������á���������������API���Զ��������ӣ��ͻ��˿ɲ�������
///@param nReason ����ԭ��
///        0x1001 �����ʧ��
///        0x1002 ����дʧ��
///        0x2001 ����������ʱ
///        0x2002 ��������ʧ��
///        0x2003 �յ�������
void MdSpi::OnFrontDisconnected(int nReason)
{
	cout << "���ӶϿ���";
	switch (nReason)
	{
	case  0x1001:
		cout << "�����ʧ��" << endl;
		break;
	case 0x1002:
		cout << "����дʧ��" << endl;
		break;
	case 0x2001:
		cout << "����������ʱ" << endl;
		break;
	case 0x2002:
		cout << "��������ʧ��" << endl;
		break;
	case 0x2003:
		cout << "�յ�������" << endl;
		break;
	default:
		break;
	}
}

///������ʱ���档����ʱ��δ�յ�����ʱ���÷��������á�
///@param nTimeLapse �����ϴν��ձ��ĵ�ʱ��
void OnHeartBeatWarning(int nTimeLapse){};


///��¼������Ӧ
void MdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		cout << "��¼�ɹ�" << endl;
		cout << "�������� : " << pRspUserLogin->TradingDay << endl;
		SubscribeMarketData();
	}
}

///�ǳ�������Ӧ
void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///����Ӧ��
void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///��������Ӧ��
void MdSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo))
	{
		cout << "���鶩�ĳɹ�" << endl;
		cout << "���ĵ�����ID : " << pSpecificInstrument->InstrumentID << endl;
	}
}

///ȡ����������Ӧ��
void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///����ѯ��Ӧ��
void OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///ȡ������ѯ��Ӧ��
void OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///�������֪ͨ
void MdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	if (!strcmp(pDepthMarketData->InstrumentID, *ppInstrumentID))
	{
		if(is_first[0])
		{
			rb0.index = "rb0";
			rb0.max_high = pDepthMarketData->UpperLimitPrice;
			rb0.max_low = pDepthMarketData->LowerLimitPrice;
			rb0.pre_settle = pDepthMarketData->PreSettlementPrice;
			rb0.pre_open_interest = pDepthMarketData->PreOpenInterest;
			if (pDepthMarketData->BidPrice1 == rb0.max_high)
			{
				rb0.ask = rb0.max_high;
				rb0.vask = 0;
			}
			else
			{
				rb0.ask = pDepthMarketData->AskPrice1;
				rb0.vask = pDepthMarketData->AskVolume1;
			}
			if (pDepthMarketData->AskPrice1 == rb0.max_low)
			{
				rb0.bid = rb0.max_low;
				rb0.vbid = 0;
			}
			else
			{
				rb0.bid = pDepthMarketData->BidPrice1;
				rb0.vbid = pDepthMarketData->BidVolume1;
			}
			rb0.last = pDepthMarketData->LastPrice;
			rb0.millsec = pDepthMarketData->UpdateMillisec;
			rb0.open_interest = pDepthMarketData->OpenInterest;
			rb0.time = pDepthMarketData->UpdateTime;
			rb0.volume = pDepthMarketData->Volume;
			rb0.old_volume = NULL;
			rb0.old_open_interest = NULL;
			rb01.push_back(rb0);
			is_first[0] = false;
		}
		else
		{
			if (rb_is_beifen)
			{
				if (rb02.size() == 1000)
				{
					rb_is_beifen = !rb_is_beifen;
					rb_cv.notify_one();
				}
			}
			else
			{
				if (rb01.size() == 1000)
				{
					rb_is_beifen = !rb_is_beifen;
					rb_cv.notify_one();
				}
			}
			if(rb_is_beifen)
			{
				rb0.old_volume = rb0.volume;
				rb0.old_open_interest = rb0.open_interest;
				if (pDepthMarketData->BidPrice1 == rb0.max_high)
				{
					rb0.ask = rb0.max_high;
					rb0.vask = 0;
				}
				else
				{
					rb0.ask = pDepthMarketData->AskPrice1;
					rb0.vask = pDepthMarketData->AskVolume1;
				}
				if (pDepthMarketData->AskPrice1 == rb0.max_low)
				{
					rb0.bid = rb0.max_low;
					rb0.vbid = 0;
				}
				else
				{
					rb0.bid = pDepthMarketData->BidPrice1;
					rb0.vbid = pDepthMarketData->BidVolume1;
				}
				rb0.last = pDepthMarketData->LastPrice;
				rb0.millsec = pDepthMarketData->UpdateMillisec;
				rb0.open_interest = pDepthMarketData->OpenInterest;
				rb0.time = pDepthMarketData->UpdateTime;
				rb0.volume = pDepthMarketData->Volume;
				rb0.high = pDepthMarketData->HighestPrice;
				rb0.low = pDepthMarketData->LowestPrice;
				rb0.open = pDepthMarketData->OpenPrice;
				rb02.push_back(rb0);
			}
			else
			{
				rb0.old_volume = rb0.volume;
				rb0.old_open_interest = rb0.open_interest;
				if (pDepthMarketData->BidPrice1 == rb0.max_high)
				{
					rb0.ask = rb0.max_high;
					rb0.vask = 0;
				}
				else
				{
					rb0.ask = pDepthMarketData->AskPrice1;
					rb0.vask = pDepthMarketData->AskVolume1;
				}
				if (pDepthMarketData->AskPrice1 == rb0.max_low)
				{
					rb0.bid = rb0.max_low;
					rb0.vbid = 0;
				}
				else
				{
					rb0.bid = pDepthMarketData->BidPrice1;
					rb0.vbid = pDepthMarketData->BidVolume1;
				}
				rb0.last = pDepthMarketData->LastPrice;
				rb0.millsec = pDepthMarketData->UpdateMillisec;
				rb0.open_interest = pDepthMarketData->OpenInterest;
				rb0.time = pDepthMarketData->UpdateTime;
				rb0.volume = pDepthMarketData->Volume;
				rb0.high = pDepthMarketData->HighestPrice;
				rb0.low = pDepthMarketData->LowestPrice;
				rb0.open = pDepthMarketData->OpenPrice;
				rb01.push_back(rb0);
			}
		}
	}
	else if (!strcmp(pDepthMarketData->InstrumentID, *(ppInstrumentID + 1)))
	{
		if(is_first[1])
		{
			rb1.index = "rb1";
			rb1.max_high = pDepthMarketData->UpperLimitPrice;
			rb1.max_low = pDepthMarketData->LowerLimitPrice;
			rb1.pre_settle = pDepthMarketData->PreSettlementPrice;
			rb1.pre_open_interest = pDepthMarketData->PreOpenInterest;
			if (pDepthMarketData->BidPrice1 == rb1.max_high)
			{
				rb1.ask = rb1.max_high;
				rb1.vask = 0;
			}
			else
			{
				rb1.ask = pDepthMarketData->AskPrice1;
				rb1.vask = pDepthMarketData->AskVolume1;
			}
			if (pDepthMarketData->AskPrice1 == rb1.max_low)
			{
				rb1.bid = rb1.max_low;
				rb1.vbid = 0;
			}
			else
			{
				rb1.bid = pDepthMarketData->BidPrice1;
				rb1.vbid = pDepthMarketData->BidVolume1;
			}
			rb1.last = pDepthMarketData->LastPrice;
			rb1.millsec = pDepthMarketData->UpdateMillisec;
			rb1.open_interest = pDepthMarketData->OpenInterest;
			rb1.time = pDepthMarketData->UpdateTime;
			rb1.volume = pDepthMarketData->Volume;
			rb1.old_volume = NULL;
			rb1.old_open_interest = NULL;
			rb11.push_back(rb1);
			is_first[1] = false;
		}
		else
		{
			if(rb_is_beifen)
			{
				rb1.old_volume = rb1.volume;
				rb1.old_open_interest = rb1.open_interest;
				if (pDepthMarketData->BidPrice1 == rb1.max_high)
				{
					rb1.ask = rb1.max_high;
					rb1.vask = 0;
				}
				else
				{
					rb1.ask = pDepthMarketData->AskPrice1;
					rb1.vask = pDepthMarketData->AskVolume1;
				}
				if (pDepthMarketData->AskPrice1 == rb1.max_low)
				{
					rb1.bid = rb1.max_low;
					rb1.vbid = 0;
				}
				else
				{
					rb1.bid = pDepthMarketData->BidPrice1;
					rb1.vbid = pDepthMarketData->BidVolume1;
				}
				rb1.last = pDepthMarketData->LastPrice;
				rb1.millsec = pDepthMarketData->UpdateMillisec;
				rb1.open_interest = pDepthMarketData->OpenInterest;
				rb1.time = pDepthMarketData->UpdateTime;
				rb1.volume = pDepthMarketData->Volume; 
				rb1.high = pDepthMarketData->HighestPrice;
				rb1.low = pDepthMarketData->LowestPrice;
				rb1.open = pDepthMarketData->OpenPrice;
				rb12.push_back(rb1);
			}
			else
			{
				rb1.old_volume = rb1.volume;
				rb1.old_open_interest = rb1.open_interest;
				if (pDepthMarketData->BidPrice1 == rb1.max_high)
				{
					rb1.ask = rb1.max_high;
					rb1.vask = 0;
				}
				else
				{
					rb1.ask = pDepthMarketData->AskPrice1;
					rb1.vask = pDepthMarketData->AskVolume1;
				}
				if (pDepthMarketData->AskPrice1 == rb1.max_low)
				{
					rb1.bid = rb1.max_low;
					rb1.vbid = 0;
				}
				else
				{
					rb1.bid = pDepthMarketData->BidPrice1;
					rb1.vbid = pDepthMarketData->BidVolume1;
				}
				rb1.last = pDepthMarketData->LastPrice;
				rb1.millsec = pDepthMarketData->UpdateMillisec;
				rb1.open_interest = pDepthMarketData->OpenInterest;
				rb1.time = pDepthMarketData->UpdateTime;
				rb1.volume = pDepthMarketData->Volume;
				rb1.high = pDepthMarketData->HighestPrice;
				rb1.low = pDepthMarketData->LowestPrice;
				rb1.open = pDepthMarketData->OpenPrice;
				rb11.push_back(rb1);
			}
		}
	}
	else if (!strcmp(pDepthMarketData->InstrumentID, *(ppInstrumentID + 2)))
	{
		if(is_first[2])
		{
			io0.index = "io0";
			io0.max_high = pDepthMarketData->UpperLimitPrice;
			io0.max_low = pDepthMarketData->LowerLimitPrice;
			io0.pre_settle = pDepthMarketData->PreSettlementPrice;
			io0.pre_open_interest = pDepthMarketData->PreOpenInterest;
			if (pDepthMarketData->BidPrice1 == io0.max_high)
			{
				io0.ask = io0.max_high;
				io0.vask = 0;
			}
			else
			{
				io0.ask = pDepthMarketData->AskPrice1;
				io0.vask = pDepthMarketData->AskVolume1;
			}
			if (pDepthMarketData->AskPrice1 == io0.max_low)
			{
				io0.bid = io0.max_low;
				io0.vbid = 0;
			}
			else
			{
				io0.bid = pDepthMarketData->BidPrice1;
				io0.vbid = pDepthMarketData->BidVolume1;
			}
			io0.last = pDepthMarketData->LastPrice;
			io0.millsec = pDepthMarketData->UpdateMillisec;
			io0.open_interest = pDepthMarketData->OpenInterest;
			io0.time = pDepthMarketData->UpdateTime;
			io0.volume = pDepthMarketData->Volume;
			io0.old_volume = NULL;
			io0.old_open_interest = NULL;
			io01.push_back(io0);
			is_first[2] = false;
		}
		else
		{
			if (io_is_beifen)
			{
				if (io02.size() == 1000)
				{
					io_is_beifen = !io_is_beifen;
					io_cv.notify_one();
				}
				//notify here;beifen jia mutex
			}
			else
			{
				if (io01.size() == 1000)
				{
					io_is_beifen = !io_is_beifen;
					io_cv.notify_one();
				}
			}
			if(io_is_beifen)
			{
				io0.old_volume = io0.volume;
				io0.old_open_interest = io0.open_interest;
				if (pDepthMarketData->BidPrice1 == io0.max_high)
				{
					io0.ask = io0.max_high;
					io0.vask = 0;
				}
				else
				{
					io0.ask = pDepthMarketData->AskPrice1;
					io0.vask = pDepthMarketData->AskVolume1;
				}
				if (pDepthMarketData->AskPrice1 == io0.max_low)
				{
					io0.bid = io0.max_low;
					io0.vbid = 0;
				}
				else
				{
					io0.bid = pDepthMarketData->BidPrice1;
					io0.vbid = pDepthMarketData->BidVolume1;
				}
				io0.last = pDepthMarketData->LastPrice;
				io0.millsec = pDepthMarketData->UpdateMillisec;
				io0.open_interest = pDepthMarketData->OpenInterest;
				io0.time = pDepthMarketData->UpdateTime;
				io0.volume = pDepthMarketData->Volume;
				io0.high = pDepthMarketData->HighestPrice;
				io0.low = pDepthMarketData->LowestPrice;
				io0.open = pDepthMarketData->OpenPrice;
				io02.push_back(io0);
			}
			else
			{
				io0.old_volume = io0.volume;
				io0.old_open_interest = io0.open_interest;
				if (pDepthMarketData->BidPrice1 == io0.max_high)
				{
					io0.ask = io0.max_high;
					io0.vask = 0;
				}
				else
				{
					io0.ask = pDepthMarketData->AskPrice1;
					io0.vask = pDepthMarketData->AskVolume1;
				}
				if (pDepthMarketData->AskPrice1 == io0.max_low)
				{
					io0.bid = io0.max_low;
					io0.vbid = 0;
				}
				else
				{
					io0.bid = pDepthMarketData->BidPrice1;
					io0.vbid = pDepthMarketData->BidVolume1;
				}
				io0.last = pDepthMarketData->LastPrice;
				io0.millsec = pDepthMarketData->UpdateMillisec;
				io0.open_interest = pDepthMarketData->OpenInterest;
				io0.time = pDepthMarketData->UpdateTime;
				io0.volume = pDepthMarketData->Volume;
				io0.high = pDepthMarketData->HighestPrice;
				io0.low = pDepthMarketData->LowestPrice;
				io0.open = pDepthMarketData->OpenPrice;
				io01.push_back(io0);
			}
		}
	}
	else if(!strcmp(pDepthMarketData->InstrumentID, *(ppInstrumentID+3)))
	{
		if(is_first[3])
		{
			io1.index = "io1";
			io1.max_high = pDepthMarketData->UpperLimitPrice;
			io1.max_low = pDepthMarketData->LowerLimitPrice;
			io1.pre_settle = pDepthMarketData->PreSettlementPrice;
			io1.pre_open_interest = pDepthMarketData->PreOpenInterest;
			if (pDepthMarketData->BidPrice1 == io1.max_high)
			{
				io1.ask = io1.max_high;
				io1.vask = 0;
			}
			else
			{
				io1.ask = pDepthMarketData->AskPrice1;
				io1.vask = pDepthMarketData->AskVolume1;
			}
			if (pDepthMarketData->AskPrice1 == io1.max_low)
			{
				io1.bid = io1.max_low;
				io1.vbid = 0;
			}
			else
			{
				io1.bid = pDepthMarketData->BidPrice1;
				io1.vbid = pDepthMarketData->BidVolume1;
			}
			io1.last = pDepthMarketData->LastPrice;
			io1.millsec = pDepthMarketData->UpdateMillisec;
			io1.open_interest = pDepthMarketData->OpenInterest;
			io1.time = pDepthMarketData->UpdateTime;
			io1.volume = pDepthMarketData->Volume;
			io1.old_volume = NULL;
			io1.old_open_interest = NULL;
			io11.push_back(io1);
			is_first[3] = false;
		}
		else
		{
			if(io_is_beifen)
			{
				io1.old_volume = io1.volume;
				io1.old_open_interest = io1.open_interest;
				if (pDepthMarketData->BidPrice1 == io1.max_high)
				{
					io1.ask = io1.max_high;
					io1.vask = 0;
				}
				else
				{
					io1.ask = pDepthMarketData->AskPrice1;
					io1.vask = pDepthMarketData->AskVolume1;
				}
				if (pDepthMarketData->AskPrice1 == io1.max_low)
				{
					io1.bid = io1.max_low;
					io1.vbid = 0;
				}
				else
				{
					io1.bid = pDepthMarketData->BidPrice1;
					io1.vbid = pDepthMarketData->BidVolume1;
				}
				io1.last = pDepthMarketData->LastPrice;
				io1.millsec = pDepthMarketData->UpdateMillisec;
				io1.open_interest = pDepthMarketData->OpenInterest;
				io1.time = pDepthMarketData->UpdateTime;
				io1.volume = pDepthMarketData->Volume;
				io1.high = pDepthMarketData->HighestPrice;
				io1.low = pDepthMarketData->LowestPrice;
				io1.open = pDepthMarketData->OpenPrice;
				io12.push_back(io1);
			}
			else
			{
				io1.old_volume = io1.volume;
				io1.old_open_interest = io1.open_interest;
				if (pDepthMarketData->BidPrice1 == io1.max_high)
				{
					io1.ask = io1.max_high;
					io1.vask = 0;
				}
				else
				{
					io1.ask = pDepthMarketData->AskPrice1;
					io1.vask = pDepthMarketData->AskVolume1;
				}
				if (pDepthMarketData->AskPrice1 == io1.max_low)
				{
					io1.bid = io1.max_low;
					io1.vbid = 0;
				}
				else
				{
					io1.bid = pDepthMarketData->BidPrice1;
					io1.vbid = pDepthMarketData->BidVolume1;
				}
				io1.last = pDepthMarketData->LastPrice;
				io1.millsec = pDepthMarketData->UpdateMillisec;
				io1.open_interest = pDepthMarketData->OpenInterest;
				io1.time = pDepthMarketData->UpdateTime;
				io1.volume = pDepthMarketData->Volume;
				io1.high = pDepthMarketData->HighestPrice;
				io1.low = pDepthMarketData->LowestPrice;
				io1.open = pDepthMarketData->OpenPrice;
				io11.push_back(io1);
			}
		}
	}
	else
	{
		std::cout << __FUNCTION__ <<"��������"<< std::endl;
	}
}

///ѯ��֪ͨ
void OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp) {};

void MdSpi::ReqUserLogin()
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.UserID, INVESTOR_ID);
	strcpy(req.Password, PASSWORD);
	int iResult = pUserApi->ReqUserLogin(&req, ++iRequestID);
	switch (iResult)
	{
	case 0:
		cout << "�����û���¼����: �ɹ�" << endl;
		break;
	case -1:
		cout << "�����û���¼����: ʧ�ܣ� ������ԭ����ʧ��" << endl;
		break;
	case -2:
		cout << "�����û���¼����: ʧ�ܣ� δ������������������" << endl;
		break;
	case -3:
		cout << "�����û���¼����: ʧ�ܣ�ÿ�뷢����������������" << endl;
		break;
	default:
		break;
	}
}

bool MdSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (bResult)
		cout << "ErrorID = " << pRspInfo->ErrorID << ", ErrorMsg = " << pRspInfo->ErrorMsg << endl;
	return bResult;
}

void MdSpi::SubscribeMarketData()
{
	int iResult = pUserApi->SubscribeMarketData(ppInstrumentID, iInstrumentID);
	switch (iResult)
	{
	case 0:
		cout << "�����ö�����������: �ɹ�" << endl;
		break;
	case -1:
		cout << "�����ö�����������: ʧ�ܣ� ������ԭ����ʧ��" << endl;
		break;
	case -2:
		cout << "�����ö�����������: ʧ�ܣ� δ������������������" << endl;
		break;
	case -3:
		cout << "�����ö�����������: ʧ�ܣ�ÿ�뷢����������������" << endl;
		break;
	default:
		break;
	}
}
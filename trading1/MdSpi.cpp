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

// 配置参数
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
//数据的备份
extern std::vector<min_data> rb02;
extern std::vector<min_data> rb12;
extern std::vector<min_data> io02;
extern std::vector<min_data> io12;
//是否使用备份
extern bool rb_is_beifen;
extern bool io_is_beifen;
//是否是第一次更新
extern std::vector<bool> is_first;
extern min_data rb0;
extern min_data rb1;
extern min_data io0;
extern min_data io1;


// 请求编号
extern int iRequestID;

void MdSpi::OnFrontConnected()
{
	cout << "链接成功" << endl;
	ReqUserLogin();
}

///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
///@param nReason 错误原因
///        0x1001 网络读失败
///        0x1002 网络写失败
///        0x2001 接收心跳超时
///        0x2002 发送心跳失败
///        0x2003 收到错误报文
void MdSpi::OnFrontDisconnected(int nReason)
{
	cout << "链接断开，";
	switch (nReason)
	{
	case  0x1001:
		cout << "网络读失败" << endl;
		break;
	case 0x1002:
		cout << "网络写失败" << endl;
		break;
	case 0x2001:
		cout << "接收心跳超时" << endl;
		break;
	case 0x2002:
		cout << "发送心跳失败" << endl;
		break;
	case 0x2003:
		cout << "收到错误报文" << endl;
		break;
	default:
		break;
	}
}

///心跳超时警告。当长时间未收到报文时，该方法被调用。
///@param nTimeLapse 距离上次接收报文的时间
void OnHeartBeatWarning(int nTimeLapse){};


///登录请求响应
void MdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		cout << "登录成功" << endl;
		cout << "交易日期 : " << pRspUserLogin->TradingDay << endl;
		SubscribeMarketData();
	}
}

///登出请求响应
void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///错误应答
void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///订阅行情应答
void MdSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo))
	{
		cout << "行情订阅成功" << endl;
		cout << "订阅的行情ID : " << pSpecificInstrument->InstrumentID << endl;
	}
}

///取消订阅行情应答
void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///订阅询价应答
void OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///取消订阅询价应答
void OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///深度行情通知
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
		std::cout << __FUNCTION__ <<"发生问题"<< std::endl;
	}
}

///询价通知
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
		cout << "发送用户登录请求: 成功" << endl;
		break;
	case -1:
		cout << "发送用户登录请求: 失败， 因网络原因发送失败" << endl;
		break;
	case -2:
		cout << "发送用户登录请求: 失败， 未处理请求数量超限制" << endl;
		break;
	case -3:
		cout << "发送用户登录请求: 失败，每秒发送请求数量超限制" << endl;
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
		cout << "发送用订阅行情请求: 成功" << endl;
		break;
	case -1:
		cout << "发送用订阅行情请求: 失败， 因网络原因发送失败" << endl;
		break;
	case -2:
		cout << "发送用订阅行情请求: 失败， 未处理请求数量超限制" << endl;
		break;
	case -3:
		cout << "发送用订阅行情请求: 失败，每秒发送请求数量超限制" << endl;
		break;
	default:
		break;
	}
}
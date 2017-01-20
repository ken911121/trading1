// trading1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ThostFtdcMdApi.h"
#include "MdSpi.h"
#include "mysql_driver.h"
#include "mysql_connection.h"
#include "cppconn/driver.h"
#include "cppconn/statement.h"
#include "cppconn/prepared_statement.h"
#include "cppconn/metadata.h"
#include "cppconn/exception.h"
#include <sstream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <vector>
#include "add_data.h"
#include <afx.h>
#include <windows.h> 

std::mutex rb_mtx;
std::condition_variable rb_cv;
std::mutex io_mtx;
std::condition_variable io_cv;
std::mutex SQL;

CThostFtdcMdApi* pUserApi;
//需要的数据
std::vector<min_data> rb01;
std::vector<min_data> rb11;
std::vector<min_data> io01;
std::vector<min_data> io11;
//数据的备份
std::vector<min_data> rb02;
std::vector<min_data> rb12;
std::vector<min_data> io02;
std::vector<min_data> io12;
//是否使用备份
bool rb_is_beifen = false;
bool io_is_beifen = false;
//是否是第一次更新
std::vector<bool> is_first(4,true);
//其他一些常备数据
min_data rb0;
min_data rb1;
min_data io0;
min_data io1;

char  FRONT_ADDR[] = "tcp://180.166.103.21:51213";		// 前置地址
TThostFtdcBrokerIDType	BROKER_ID = "4040";				// 经纪公司代码
TThostFtdcInvestorIDType INVESTOR_ID = "";			// 注意这里输入你自己的投资者代码
TThostFtdcPasswordType  PASSWORD = "";			// 注意这里输入你自己的用户密码

char *ppInstrumentID[] = { "rb1705","rb1710","i1705","i1709" };			// 行情订阅列表，注意，这个合约ID会过时的，注意与时俱进修改
int iInstrumentID = 4;

// 请求编号
int iRequestID = 0;

//数据库
sql::mysql::MySQL_Driver *driver = 0;
sql::Connection *conn = 0;
sql::Statement* stat = 0;
sql::ResultSet *res = 0;

int _tmain(int argc, _TCHAR* argv[])
{
	rb01.reserve(4000);
	rb11.reserve(4000);
	io01.reserve(4000);
	io11.reserve(4000);
	//数据的备份
	rb02.reserve(4000);
	rb12.reserve(4000);
	io02.reserve(4000);
	io12.reserve(4000);
	
	pUserApi = CThostFtdcMdApi::CreateFtdcMdApi();
	CThostFtdcMdSpi* pUserSpi = new MdSpi();
	pUserApi->RegisterSpi(pUserSpi);
	pUserApi->RegisterFront(FRONT_ADDR);					// connect
	pUserApi->Init();
	try
	{
		driver = sql::mysql::get_mysql_driver_instance();
		conn = driver->connect("tcp://localhost:3306/comidity", "root", "CHenzuidongfeng1!");
		std::cout << "连接成功" << std::endl;
	}
	catch (...)
	{
		std::cout << "连接失败" << std::endl;
	}
	stat = conn->createStatement();
	stat->execute("set names 'gbk'");
	std::thread add_data_thread(add_data_rb);
	std::thread add_data_thread1(add_data_io);
	add_data_thread.join();
	add_data_thread1.join();
	pUserApi->Release();
	Sleep(10000000000000);
	







	/*
	sql::ResultSet *res;
	res = stat->executeQuery("SELECT * FROM f_info");
	while (res->next())
	{
		std::cout << "BOOKNAME:" << res->getString("F_index") << std::endl;
		std::cout << "    SIZE:" << res->getString("comments") <<std::endl;
	}
	int b = 100;
	std::stringstream aa;
	std::string c = "'asd'";
	std::string d = "'2010-11-11'";

	aa << "insert into f_info values (" << c << "," << d << "," << d << "," << c<<")";
	std::string a = aa.str();
	std::cout << a << std::endl;



	stat->executeUpdate(a);
	
	/*
	TThostFtdcTimeType time = "031313";
	if (strcmp(time, "031313") == 0)
	{
		std::cout << time << std::endl;
		std::cout << *(ppInstrumentID+1) << std::endl;
		std::cout << ((time[1] - '0') == 3) << std::endl;
	}
	
	std::string a = "13:13:13";
	std::string b = "13:13:13";
	*/
	//std::cout << (a.substr(0,5) == b.substr(0,5))<< std::endl;
	//std::cout << io0.volume << std::endl;
	//std::cout << (io0.time == "") << std::endl;
	return 0;
}


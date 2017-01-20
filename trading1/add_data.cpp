#include "stdafx.h"
#include "add_data.h"
#include <ctime>  
#include <chrono>  
#include <iomanip>
#pragma warning(disable : 4996)
//yhntyh
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
//数据库
extern sql::mysql::MySQL_Driver *driver;
extern sql::Connection *conn;
extern sql::Statement* stat;
extern sql::ResultSet *res;

extern std::mutex rb_mtx;
extern std::condition_variable rb_cv;
extern std::mutex io_mtx;
extern std::condition_variable io_cv;
extern std::mutex SQL;

void add_data_rb()
{
	update_time date_today;
	min_data_min min_rb0;
	min_data_min min_rb1;
	std::stringstream temp_SQL_min;
	std::stringstream temp_SQL_tick;
	std::string SQL_min;
	std::string SQL_tick;
	std::string pre_rb0_time;
	std::string pre_rb1_time;
	std::chrono::time_point<std::chrono::system_clock> finish_time;;
	//获取当前时间
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	date_today.year = timeinfo->tm_year + 1900;
	date_today.month = timeinfo->tm_mon + 1;
	date_today.day = timeinfo->tm_mday;
	date_today.hour = timeinfo->tm_hour;
	if (timeinfo->tm_hour < 9)
	{
		timeinfo->tm_hour = 15;
		timeinfo->tm_min = 1;
		timeinfo->tm_sec = 0;
		time_t timetThen = mktime(timeinfo);
		finish_time = std::chrono::system_clock::from_time_t(timetThen);
	}
	else if (timeinfo->tm_hour > 17)
	{
		timeinfo->tm_hour = 23;
		timeinfo->tm_min = 1;
		timeinfo->tm_sec = 0;
		time_t timetThen = mktime(timeinfo);
		finish_time = std::chrono::system_clock::from_time_t(timetThen);
	}
	pre_rb0_time = "";
	pre_rb1_time = "";
	    while (true)
		{
			std::unique_lock<std::mutex> lck(rb_mtx);
			if (rb_cv.wait_until(lck, finish_time) != std::cv_status::timeout)
			{
				if (!rb_is_beifen)
				{
					SQL_min = "insert into F_min_data (F_time,F_index, F_open, F_close, F_high, F_low, F_open_interest_change, F_volume) values ";
					SQL_tick = "insert into RB_0 (Time1, Time2, Price, P_ask, P_bid, V_ask, V_bid, Volume, Holding_diff_diff) values ";
					for (int i = 0; i < static_cast<int>(rb02.size()); ++i)
					{
						add_min_data_is_beifen(temp_SQL_min, rb02[i], min_rb0, date_today, pre_rb0_time);
						add_tick_data_is_beifen(temp_SQL_tick, rb02[i], pre_rb0_time, date_today);
						SQL_min += temp_SQL_min.str();
						SQL_tick += temp_SQL_tick.str();
						temp_SQL_min.clear();
						temp_SQL_min.str("");
						temp_SQL_tick.clear();
						temp_SQL_tick.str("");
					}
					SQL.lock();
					SQL_min = SQL_min.substr(0, SQL_min.length() - 1);
					try
					{
						stat->executeUpdate(SQL_min);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					std::cout << "螺纹更新" << rb02[rb02.size() - 1].time << " " << rb02[rb02.size() - 1].millsec << std::endl;
					SQL_tick = SQL_tick.substr(0, SQL_tick.length() - 1);
					try
					{
						stat->executeUpdate(SQL_tick);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					SQL.unlock();
					//update rb_1
					SQL_min.clear();
					SQL_tick.clear();
					SQL_min = "insert into F_min_data (F_time,F_index, F_open, F_close, F_high, F_low, F_open_interest_change, F_volume) values ";
					SQL_tick = "insert into RB_1 (Time1, Time2, Price, P_ask, P_bid, V_ask, V_bid, Volume, Holding_diff_diff) values ";
					for (int i = 0; i < static_cast<int>(rb12.size()); ++i)
					{
						add_min_data_is_beifen(temp_SQL_min, rb12[i], min_rb1, date_today, pre_rb1_time);
						add_tick_data_is_beifen(temp_SQL_tick, rb12[i], pre_rb1_time, date_today);
						SQL_min += temp_SQL_min.str();
						SQL_tick += temp_SQL_tick.str();
						temp_SQL_min.clear();
						temp_SQL_min.str("");
						temp_SQL_tick.clear();
						temp_SQL_tick.str("");
					}
					SQL.lock();
					SQL_min = SQL_min.substr(0, SQL_min.length() - 1);
					try
					{
						stat->executeUpdate(SQL_min);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					SQL_tick = SQL_tick.substr(0, SQL_tick.length() - 1);
					try
					{
						stat->executeUpdate(SQL_tick);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					SQL.unlock();
					SQL_min.clear();
					SQL_tick.clear();
					rb02.clear();
					rb12.clear();
				}
				else
				{
					SQL_min = "insert into F_min_data (F_time,F_index, F_open, F_close, F_high, F_low, F_open_interest_change, F_volume) values ";
					SQL_tick = "insert into RB_0 (Time1, Time2, Price, P_ask, P_bid, V_ask, V_bid, Volume, Holding_diff_diff) values ";
					for (int i = 0; i < static_cast<int>(rb01.size()); ++i)
					{
						add_min_data_not_beifen(temp_SQL_min, rb01[i], min_rb0, date_today, pre_rb0_time);
						add_tick_data_not_beifen(temp_SQL_tick, rb01[i], pre_rb0_time, date_today);
						SQL_min += temp_SQL_min.str();
						SQL_tick += temp_SQL_tick.str();
						temp_SQL_min.clear();
						temp_SQL_min.str("");
						temp_SQL_tick.clear();
						temp_SQL_tick.str("");
					}
					SQL.lock();
					SQL_min = SQL_min.substr(0, SQL_min.length() - 1);
					try
					{
						stat->executeUpdate(SQL_min);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					SQL_tick = SQL_tick.substr(0, SQL_tick.length() - 1);
					try
					{
						stat->executeUpdate(SQL_tick);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					SQL.unlock();
					//update rb_1
					SQL_min.clear();
					SQL_tick.clear();
					SQL_min = "insert into F_min_data (F_time,F_index, F_open, F_close, F_high, F_low, F_open_interest_change, F_volume) values ";
					SQL_tick = "insert into RB_1 (Time1, Time2, Price, P_ask, P_bid, V_ask, V_bid, Volume, Holding_diff_diff) values ";
					for (int i = 0; i < static_cast<int>(rb11.size()); ++i)
					{
						add_min_data_not_beifen(temp_SQL_min, rb11[i], min_rb1, date_today, pre_rb1_time);
						add_tick_data_not_beifen(temp_SQL_tick, rb11[i], pre_rb1_time, date_today);
						SQL_min += temp_SQL_min.str();
						SQL_tick += temp_SQL_tick.str();
						temp_SQL_min.clear();
						temp_SQL_min.str("");
						temp_SQL_tick.clear();
						temp_SQL_tick.str("");
					}
					SQL.lock();
					SQL_min = SQL_min.substr(0, SQL_min.length() - 1);
					try
					{
						stat->executeUpdate(SQL_min);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					SQL_tick = SQL_tick.substr(0, SQL_tick.length() - 1);
					try
					{
						stat->executeUpdate(SQL_tick);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					SQL.unlock();
					SQL_min.clear();
					SQL_tick.clear();
					rb01.clear();
					rb11.clear();
				}
			}
			else
			{
				std::cout << "螺纹更新日终" << std::endl;
				if (rb_is_beifen)
				{
					SQL_min = "insert into F_min_data (F_time,F_index, F_open, F_close, F_high, F_low, F_open_interest_change, F_volume) values ";
					SQL_tick = "insert into RB_0 (Time1, Time2, Price, P_ask, P_bid, V_ask, V_bid, Volume, Holding_diff_diff) values ";
					for (int i = 0; i < static_cast<int>(rb02.size()); ++i)
					{
						if (i == (rb02.size() - 1))
						{
							add_min_data_is_beifen(temp_SQL_min, rb02[i], min_rb0, date_today, pre_rb0_time, true);
						}
						else
						{
							add_min_data_is_beifen(temp_SQL_min, rb02[i], min_rb0, date_today, pre_rb0_time);
						}
						add_tick_data_is_beifen(temp_SQL_tick, rb02[i], pre_rb0_time, date_today);
						SQL_min += temp_SQL_min.str();
						SQL_tick += temp_SQL_tick.str();
						temp_SQL_min.clear();
						temp_SQL_min.str("");
						temp_SQL_tick.clear();
						temp_SQL_tick.str("");
					}
					SQL.lock();
					SQL_min = SQL_min.substr(0, SQL_min.length() - 1);
					try
					{
						stat->executeUpdate(SQL_min);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					SQL_tick = SQL_tick.substr(0, SQL_tick.length() - 1);
					try
					{
						stat->executeUpdate(SQL_tick);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					SQL.unlock();
					//update rb_1
					SQL_min.clear();
					SQL_tick.clear();
					SQL_min = "insert into F_min_data (F_time,F_index, F_open, F_close, F_high, F_low, F_open_interest_change, F_volume) values ";
					SQL_tick = "insert into RB_1 (Time1, Time2, Price, P_ask, P_bid, V_ask, V_bid, Volume, Holding_diff_diff) values ";
					for (int i = 0; i < static_cast<int>(rb12.size()); ++i)
					{
						if (i == (rb12.size() - 1))
						{
							add_min_data_is_beifen(temp_SQL_min, rb12[i], min_rb1, date_today, pre_rb1_time,true);
						}
						else
						{
							add_min_data_is_beifen(temp_SQL_min, rb12[i], min_rb1, date_today,pre_rb1_time);
						}
						add_tick_data_is_beifen(temp_SQL_tick, rb12[i], pre_rb1_time, date_today);
						SQL_min += temp_SQL_min.str();
						SQL_tick += temp_SQL_tick.str();
						temp_SQL_min.clear();
						temp_SQL_min.str("");
						temp_SQL_tick.clear();
						temp_SQL_tick.str("");
					}
					SQL.lock();
					SQL_min = SQL_min.substr(0, SQL_min.length() - 1);
					try
					{
						stat->executeUpdate(SQL_min);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					SQL_tick = SQL_tick.substr(0, SQL_tick.length() - 1);
					try
					{
						stat->executeUpdate(SQL_tick);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					SQL.unlock();
					SQL_min.clear();
					SQL_tick.clear();
				}
				else
				{
					SQL_min = "insert into F_min_data (F_time,F_index, F_open, F_close, F_high, F_low, F_open_interest_change, F_volume) values ";
					SQL_tick = "insert into RB_0 (Time1, Time2, Price, P_ask, P_bid, V_ask, V_bid, Volume, Holding_diff_diff) values ";
					for (int i = 0; i < static_cast<int>(rb01.size()); ++i)
					{
						if (i == (rb01.size() - 1))
						{
							add_min_data_not_beifen(temp_SQL_min, rb01[i], min_rb0, date_today, pre_rb0_time,true);
						}
						else
						{
							add_min_data_not_beifen(temp_SQL_min, rb01[i], min_rb0, date_today,pre_rb0_time);
						}
						add_tick_data_not_beifen(temp_SQL_tick, rb01[i], pre_rb0_time, date_today);
						SQL_min += temp_SQL_min.str();
						SQL_tick += temp_SQL_tick.str();
						temp_SQL_min.clear();
						temp_SQL_min.str("");
						temp_SQL_tick.clear();
						temp_SQL_tick.str("");
					}
					SQL.lock();
					SQL_min = SQL_min.substr(0, SQL_min.length() - 1);
					try
					{
						stat->executeUpdate(SQL_min);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					SQL_tick = SQL_tick.substr(0, SQL_tick.length() - 1);
					try
					{
						stat->executeUpdate(SQL_tick);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					SQL.unlock();
					//update rb_1
					SQL_min.clear();
					SQL_tick.clear();
					SQL_min = "insert into F_min_data (F_time,F_index, F_open, F_close, F_high, F_low, F_open_interest_change, F_volume) values ";
					SQL_tick = "insert into RB_1 (Time1, Time2, Price, P_ask, P_bid, V_ask, V_bid, Volume, Holding_diff_diff) values ";
					for (int i = 0; i < static_cast<int>(rb11.size()); ++i)
					{
						if (i == (rb11.size() - 1))
						{
							add_min_data_not_beifen(temp_SQL_min, rb11[i], min_rb1, date_today, pre_rb1_time, true);
						}
						else
						{
							add_min_data_not_beifen(temp_SQL_min, rb11[i], min_rb1, date_today, pre_rb1_time);
						}
						add_tick_data_not_beifen(temp_SQL_tick, rb11[i], pre_rb1_time, date_today);
						SQL_min += temp_SQL_min.str();
						SQL_tick += temp_SQL_tick.str();
						temp_SQL_min.clear();
						temp_SQL_min.str("");
						temp_SQL_tick.clear();
						temp_SQL_tick.str("");
					}
					SQL.lock();
					SQL_min = SQL_min.substr(0, SQL_min.length() - 1);
					try
					{
						stat->executeUpdate(SQL_min);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					SQL_tick = SQL_tick.substr(0, SQL_tick.length() - 1);
					try
					{
						stat->executeUpdate(SQL_tick);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					SQL.unlock();
					SQL_min.clear();
					SQL_tick.clear();
				}
				time_t temp_time_t = std::chrono::system_clock::to_time_t(finish_time);
				struct tm * temp_tm = localtime(&temp_time_t);
				if (temp_tm->tm_hour == 15)
				{
					if (rb_is_beifen)
					{
						std::stringstream ss;
						std::string SQL_day = "insert into F_daily_data (F_time,F_index, F_open, F_close, F_high, F_low, F_open_interest, F_volume, F_max_high, F_max_low,F_pre_settle) values ";
						min_data temp_min_data = rb02[rb02.size() - 1];
						ss << "('" << date_today.year << "-" << date_today.month << "-" << date_today.day << "','" << temp_min_data.index << "'," << temp_min_data.open << "," << temp_min_data.last << "," << temp_min_data.high << "," << temp_min_data.low << "," << temp_min_data.open_interest << "," << temp_min_data.volume << "," << temp_min_data.max_high << "," << temp_min_data.max_low << "," << temp_min_data.pre_settle << ")";
						SQL_day += ss.str();
						SQL.lock();
						try
						{
							stat->executeUpdate(SQL_day);
						}
						catch (sql::SQLException &e)
						{
							std::cout << e.getSQLState() << e.what() << std::endl;
						}
						SQL.unlock();
						//update rb1
						ss.clear();
						ss.str("");
						SQL_day.clear();
						SQL_day = "insert into F_daily_data (F_time,F_index, F_open, F_close, F_high, F_low, F_open_interest, F_volume, F_max_high, F_max_low,F_pre_settle) values ";
						temp_min_data = rb12[rb12.size() - 1];
						ss << "('" << date_today.year << "-" << date_today.month << "-" << date_today.day << "','" << temp_min_data.index << "'," << temp_min_data.open << "," << temp_min_data.last << "," << temp_min_data.high << "," << temp_min_data.low << "," << temp_min_data.open_interest << "," << temp_min_data.volume << "," << temp_min_data.max_high << "," << temp_min_data.max_low << "," << temp_min_data.pre_settle << ")";
						SQL_day += ss.str();
						SQL.lock();
						try
						{
							stat->executeUpdate(SQL_day);
						}
						catch (sql::SQLException &e)
						{
							std::cout << e.getSQLState() << e.what() << std::endl;
						}
						SQL.unlock();
					}
					else
					{
						std::stringstream ss;
						std::string SQL_day = "insert into F_daily_data (F_time,F_index, F_open, F_close, F_high, F_low, F_open_interest, F_volume, F_max_high, F_max_low,F_pre_settle) values ";
						min_data temp_min_data = rb01[rb01.size() - 1];
						ss << "('" << date_today.year << "-" << date_today.month << "-" << date_today.day << "','" << temp_min_data.index << "'," << temp_min_data.open << "," << temp_min_data.last << "," << temp_min_data.high << "," << temp_min_data.low << "," << temp_min_data.open_interest << "," << temp_min_data.volume << "," << temp_min_data.max_high << "," << temp_min_data.max_low << "," << temp_min_data.pre_settle << ")";
						SQL_day += ss.str();
						SQL.lock();
						try
						{
							stat->executeUpdate(SQL_day);
						}
						catch (sql::SQLException &e)
						{
							std::cout << e.getSQLState() << e.what() << std::endl;
						}
						SQL.unlock();
						//update rb1
						ss.clear();
						ss.str("");
						SQL_day.clear();
						SQL_day = "insert into F_daily_data (F_time,F_index, F_open, F_close, F_high, F_low, F_open_interest, F_volume, F_max_high, F_max_low,F_pre_settle) values ";
						temp_min_data = rb11[rb11.size() - 1];
						ss << "('" << date_today.year << "-" << date_today.month << "-" << date_today.day << "','" << temp_min_data.index << "'," << temp_min_data.open << "," << temp_min_data.last << "," << temp_min_data.high << "," << temp_min_data.low << "," << temp_min_data.open_interest << "," << temp_min_data.volume << "," << temp_min_data.max_high << "," << temp_min_data.max_low << "," << temp_min_data.pre_settle << ")";
						SQL_day += ss.str();
						SQL.lock();
						try
						{
							stat->executeUpdate(SQL_day);
						}
						catch (sql::SQLException &e)
						{
							std::cout << e.getSQLState() << e.what() << std::endl;
						}
						SQL.unlock();
					}
					std::string temp_string;
					std::stringstream ss;
					ss << "delete from rb_0 where time1 > '" << date_today.year << "-" << date_today.month << "-" << date_today.day << " 15:05:00'";
					temp_string = ss.str();
					SQL.lock();
					stat->execute(temp_string);
					SQL.unlock();
					ss.clear();
					ss.str("");
					ss << "delete from rb_1 where time1 > '" << date_today.year << "-" << date_today.month << "-" << date_today.day << " 15:05:00'";
					temp_string = ss.str();
					SQL.lock();
					stat->execute(temp_string);
					SQL.unlock();
					ss.clear();
					ss.str("");
					ss << "delete from f_min_data where f_time > '" << date_today.year << "-" << date_today.month << "-" << date_today.day << " 15:05:00' and (f_index = 'rb0' or f_index = 'rb1')";
					temp_string = ss.str();
					SQL.lock();
					stat->execute(temp_string);
					SQL.unlock();
				}
				else
				{
					std::string temp_date_today;
					std::string temp_string;
					std::stringstream ss;
					ss << date_today.year << "-" << date_today.month << "-" << date_today.day;
					temp_date_today = ss.str();
					ss.clear();
					ss.str("");
					ss << "delete from rb_0 where time1 > '" << temp_date_today << " 23:05:00' or (time1 > '" << temp_date_today << " 15:05:00' and time1 <'" << temp_date_today << " 20:55:00') or (time1 > '" << temp_date_today << " 00:00:00' and time1 <'" << temp_date_today << " 08:55:00')";
					temp_string = ss.str();
					SQL.lock();
					stat->execute(temp_string);
					SQL.unlock();
					ss.clear();
					ss.str("");
					ss << "delete from rb_1 where time1 > '" << temp_date_today << " 23:05:00' or (time1 > '" << temp_date_today << " 15:05:00' and time1 <'" << temp_date_today << " 20:55:00') or (time1 > '" << temp_date_today << " 00:00:00' and time1 <'" << temp_date_today << " 08:55:00')";
					temp_string = ss.str();
					SQL.lock();
					stat->execute(temp_string);
					SQL.unlock();
					ss.clear();
					ss.str("");
					ss << "delete from f_min_data where (f_time > '" << temp_date_today << " 23:05:00' or (f_time > '" << temp_date_today << " 15:05:00' and f_time <'" << temp_date_today << " 20:55:00') or (f_time > '" << temp_date_today << " 00:00:00' and f_time <'" << temp_date_today << " 08:55:00')) and (f_index = 'rb0' or f_index = 'rb1')";
					temp_string = ss.str();
					SQL.lock();
					stat->execute(temp_string);
					SQL.unlock();
				}
				return;
			}
		}
}

void add_data_io()
{
	update_time date_today;
	min_data_min min_io0;
	min_data_min min_io1;
	std::stringstream temp_SQL_min;
	std::stringstream temp_SQL_tick;
	std::string pre_io0_time;
	std::string pre_io1_time;
	std::string SQL_min;
	std::string SQL_tick;
	std::chrono::time_point<std::chrono::system_clock> finish_time;;
	//获取当前时间
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	date_today.year = timeinfo->tm_year + 1900;
	date_today.month = timeinfo->tm_mon + 1;
	date_today.day = timeinfo->tm_mday;
	date_today.hour = timeinfo->tm_hour;
	if (timeinfo->tm_hour < 9)
	{
		timeinfo->tm_hour = 15;
		timeinfo->tm_min = 1;
		timeinfo->tm_sec = 0;
		time_t timetThen = mktime(timeinfo);
		finish_time = std::chrono::system_clock::from_time_t(timetThen);
	}
	else if (timeinfo->tm_hour > 17)
	{
		timeinfo->tm_hour = 23;
		timeinfo->tm_min = 31;
		timeinfo->tm_sec = 0;
		time_t timetThen = mktime(timeinfo);
		finish_time = std::chrono::system_clock::from_time_t(timetThen);
	}
	pre_io0_time = "";
	pre_io1_time = "";
		while (true)
		{
			std::unique_lock<std::mutex> lck(io_mtx);
			if (io_cv.wait_until(lck, finish_time) != std::cv_status::timeout)
			{
				std::cout << "铁矿更新" << std::endl;
				if (!io_is_beifen)
				{
					SQL_min = "insert into F_min_data (F_time,F_index, F_open, F_close, F_high, F_low, F_open_interest_change, F_volume) values ";
					SQL_tick = "insert into IO_0 (Time1, Time2, Price, P_ask, P_bid, V_ask, V_bid, Volume, Holding_diff_diff) values ";
					for (int i = 0; i < static_cast<int>(io02.size()); ++i)
					{
						add_min_data_is_beifen(temp_SQL_min, io02[i], min_io0, date_today, pre_io0_time);
						add_tick_data_is_beifen(temp_SQL_tick, io02[i], pre_io0_time, date_today);
						SQL_min += temp_SQL_min.str();
						SQL_tick += temp_SQL_tick.str();
						temp_SQL_min.clear();
						temp_SQL_min.str("");
						temp_SQL_tick.clear();
						temp_SQL_tick.str("");
					}
					SQL.lock();
					SQL_min = SQL_min.substr(0, SQL_min.length() - 1);
					try
					{
						stat->executeUpdate(SQL_min);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState()<< e.what() << std::endl;
					}
					SQL_tick = SQL_tick.substr(0, SQL_tick.length() - 1);
					try
					{
						stat->executeUpdate(SQL_tick);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					SQL.unlock();
					//update rb_1
					SQL_min.clear();
					SQL_tick.clear();
					SQL_min = "insert into F_min_data (F_time,F_index, F_open, F_close, F_high, F_low, F_open_interest_change, F_volume) values ";
					SQL_tick = "insert into IO_1 (Time1, Time2, Price, P_ask, P_bid, V_ask, V_bid, Volume, Holding_diff_diff) values ";
					for (int i = 0; i < static_cast<int>(io12.size()); ++i)
					{
						add_min_data_is_beifen(temp_SQL_min, io12[i], min_io1, date_today, pre_io1_time);
						add_tick_data_is_beifen(temp_SQL_tick, io12[i], pre_io1_time, date_today);
						SQL_min += temp_SQL_min.str();
						SQL_tick += temp_SQL_tick.str();
						temp_SQL_min.clear();
						temp_SQL_min.str("");
						temp_SQL_tick.clear();
						temp_SQL_tick.str("");
					}
					SQL.lock();
					SQL_min = SQL_min.substr(0, SQL_min.length() - 1);
					try
					{
						stat->executeUpdate(SQL_min);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					SQL_tick = SQL_tick.substr(0, SQL_tick.length() - 1);
					try
					{
						stat->executeUpdate(SQL_tick);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					SQL.unlock();
					SQL_min.clear();
					SQL_tick.clear();
					io02.clear();
					io12.clear();
				}
				else
				{
					SQL_min = "insert into F_min_data (F_time,F_index, F_open, F_close, F_high, F_low, F_open_interest_change, F_volume) values ";
					SQL_tick = "insert into IO_0 (Time1, Time2, Price, P_ask, P_bid, V_ask, V_bid, Volume, Holding_diff_diff) values ";
					for (int i = 0; i < static_cast<int>(io01.size()); ++i)
					{
						add_min_data_not_beifen(temp_SQL_min, io01[i], min_io0, date_today, pre_io0_time);
						add_tick_data_not_beifen(temp_SQL_tick, io01[i], pre_io0_time, date_today);
						SQL_min += temp_SQL_min.str();
						SQL_tick += temp_SQL_tick.str();
						temp_SQL_min.clear();
						temp_SQL_min.str("");
						temp_SQL_tick.clear();
						temp_SQL_tick.str("");
					}
					SQL.lock();
					SQL_min = SQL_min.substr(0, SQL_min.length() - 1);
					try
					{
						stat->executeUpdate(SQL_min);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					SQL_tick = SQL_tick.substr(0, SQL_tick.length() - 1);
					try
					{
						stat->executeUpdate(SQL_tick);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					SQL.unlock();
					//update rb_1
					SQL_min.clear();
					SQL_tick.clear();
					SQL_min = "insert into F_min_data (F_time,F_index, F_open, F_close, F_high, F_low, F_open_interest_change, F_volume) values ";
					SQL_tick = "insert into IO_1 (Time1, Time2, Price, P_ask, P_bid, V_ask, V_bid, Volume, Holding_diff_diff) values ";
					for (int i = 0; i < static_cast<int>(io11.size()); ++i)
					{
						add_min_data_not_beifen(temp_SQL_min, io11[i], min_io1, date_today, pre_io1_time);
						add_tick_data_not_beifen(temp_SQL_tick, io11[i], pre_io1_time, date_today);
						SQL_min += temp_SQL_min.str();
						SQL_tick += temp_SQL_tick.str();
						temp_SQL_min.clear();
						temp_SQL_min.str("");
						temp_SQL_tick.clear();
						temp_SQL_tick.str("");
					}
					SQL.lock();
					SQL_min = SQL_min.substr(0, SQL_min.length() - 1);
					try
					{
						stat->executeUpdate(SQL_min);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					SQL_tick = SQL_tick.substr(0, SQL_tick.length() - 1);
					try
					{
						stat->executeUpdate(SQL_tick);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					SQL.unlock();
					SQL_min.clear();
					SQL_tick.clear();
					io01.clear();
					io11.clear();
				}
			}
			else
			{
				std::cout << "铁矿更新日终" << std::endl;
				if (io_is_beifen)
				{
					SQL_min = "insert into F_min_data (F_time,F_index, F_open, F_close, F_high, F_low, F_open_interest_change, F_volume) values ";
					SQL_tick = "insert into IO_0 (Time1, Time2, Price, P_ask, P_bid, V_ask, V_bid, Volume, Holding_diff_diff) values ";
					for (int i = 0; i < static_cast<int>(io02.size()); ++i)
					{
						if (i == (io02.size() - 1))
						{
							add_min_data_is_beifen(temp_SQL_min, io02[i], min_io0, date_today, pre_io0_time, true);
						}
						else
						{
							add_min_data_is_beifen(temp_SQL_min, io02[i], min_io0, date_today, pre_io0_time);
						}
						add_tick_data_is_beifen(temp_SQL_tick, io02[i], pre_io0_time, date_today);
						SQL_min += temp_SQL_min.str();
						SQL_tick += temp_SQL_tick.str();
						temp_SQL_min.clear();
						temp_SQL_min.str("");
						temp_SQL_tick.clear();
						temp_SQL_tick.str("");
					}
					SQL.lock();
					SQL_min = SQL_min.substr(0, SQL_min.length() - 1);
					try
					{
						stat->executeUpdate(SQL_min);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					SQL_tick = SQL_tick.substr(0, SQL_tick.length() - 1);
					try
					{
						stat->executeUpdate(SQL_tick);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					SQL.unlock();
					//update rb_1
					SQL_min.clear();
					SQL_tick.clear();
					SQL_min = "insert into F_min_data (F_time,F_index, F_open, F_close, F_high, F_low, F_open_interest_change, F_volume) values ";
					SQL_tick = "insert into IO_1 (Time1, Time2, Price, P_ask, P_bid, V_ask, V_bid, Volume, Holding_diff_diff) values ";
					for (int i = 0; i < static_cast<int>(io12.size()); ++i)
					{
						if (i == (io12.size() - 1))
						{
							add_min_data_is_beifen(temp_SQL_min, io12[i], min_io1, date_today, pre_io1_time, true);
						}
						else
						{
							add_min_data_is_beifen(temp_SQL_min, io12[i], min_io1, date_today, pre_io1_time);
						}
						add_tick_data_is_beifen(temp_SQL_tick, io12[i], pre_io1_time, date_today);
						SQL_min += temp_SQL_min.str();
						SQL_tick += temp_SQL_tick.str();
						temp_SQL_min.clear();
						temp_SQL_min.str("");
						temp_SQL_tick.clear();
						temp_SQL_tick.str("");
					}
					SQL.lock();
					SQL_min = SQL_min.substr(0, SQL_min.length() - 1);
					try
					{
						stat->executeUpdate(SQL_min);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					SQL_tick = SQL_tick.substr(0, SQL_tick.length() - 1);
					try
					{
						stat->executeUpdate(SQL_tick);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					SQL.unlock();
					SQL_min.clear();
					SQL_tick.clear();
				}
				else
				{
					SQL_min = "insert into F_min_data (F_time,F_index, F_open, F_close, F_high, F_low, F_open_interest_change, F_volume) values ";
					SQL_tick = "insert into IO_0 (Time1, Time2, Price, P_ask, P_bid, V_ask, V_bid, Volume, Holding_diff_diff) values ";
					for (int i = 0; i < static_cast<int>(io01.size()); ++i)
					{
						if (i == (io01.size() - 1))
						{
							add_min_data_not_beifen(temp_SQL_min, io01[i], min_io0, date_today, pre_io0_time, true);
						}
						else
						{
							add_min_data_not_beifen(temp_SQL_min, io01[i], min_io0, date_today, pre_io0_time);
						}
						add_tick_data_not_beifen(temp_SQL_tick, io01[i], pre_io0_time, date_today);
						SQL_min += temp_SQL_min.str();
						SQL_tick += temp_SQL_tick.str();
						temp_SQL_min.clear();
						temp_SQL_min.str("");
						temp_SQL_tick.clear();
						temp_SQL_tick.str("");
					}
					SQL.lock();
					SQL_min = SQL_min.substr(0, SQL_min.length() - 1);
					try
					{
						stat->executeUpdate(SQL_min);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					SQL_tick = SQL_tick.substr(0, SQL_tick.length() - 1);
					try
					{
						stat->executeUpdate(SQL_tick);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					SQL.unlock();
					//update rb_1
					SQL_min.clear();
					SQL_tick.clear();
					SQL_min = "insert into F_min_data (F_time,F_index, F_open, F_close, F_high, F_low, F_open_interest_change, F_volume) values ";
					SQL_tick = "insert into IO_1 (Time1, Time2, Price, P_ask, P_bid, V_ask, V_bid, Volume, Holding_diff_diff) values ";
					for (int i = 0; i < static_cast<int>(io11.size()); ++i)
					{
						if (i == (io11.size() - 1))
						{
							add_min_data_not_beifen(temp_SQL_min, io11[i], min_io1, date_today, pre_io1_time, true);
						}
						else
						{
							add_min_data_not_beifen(temp_SQL_min, io11[i], min_io1, date_today, pre_io1_time);
						}
						add_tick_data_not_beifen(temp_SQL_tick, io11[i], pre_io1_time, date_today);
						SQL_min += temp_SQL_min.str();
						SQL_tick += temp_SQL_tick.str();
						temp_SQL_min.clear();
						temp_SQL_min.str("");
						temp_SQL_tick.clear();
						temp_SQL_tick.str("");
					}
					SQL.lock();
					SQL_min = SQL_min.substr(0, SQL_min.length() - 1);
					try
					{
						stat->executeUpdate(SQL_min);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					SQL_tick = SQL_tick.substr(0, SQL_tick.length() - 1);
					try
					{
						stat->executeUpdate(SQL_tick);
					}
					catch (sql::SQLException &e)
					{
						std::cout << e.getSQLState() << e.what() << std::endl;
					}
					SQL.unlock();
					SQL_min.clear();
					SQL_tick.clear();
				}
				time_t temp_time_t = std::chrono::system_clock::to_time_t(finish_time);
				struct tm * temp_tm = localtime(&temp_time_t);
				if (temp_tm->tm_hour == 15)
				{
					if (io_is_beifen)
					{
						std::stringstream ss;
						std::string SQL_day = "insert into F_daily_data (F_time,F_index, F_open, F_close, F_high, F_low, F_open_interest, F_volume, F_max_high, F_max_low,F_pre_settle) values ";
						min_data temp_min_data = io02[io02.size() - 1];
						ss << "('" << date_today.year << "-" << date_today.month << "-" << date_today.day << "','" << temp_min_data.index << "'," << temp_min_data.open << "," << temp_min_data.last << "," << temp_min_data.high << "," << temp_min_data.low << "," << temp_min_data.open_interest << "," << temp_min_data.volume << "," << temp_min_data.max_high << "," << temp_min_data.max_low << "," << temp_min_data.pre_settle << ")";
						SQL_day += ss.str();
						SQL.lock();
						try
						{
							stat->executeUpdate(SQL_day);
						}
						catch (sql::SQLException &e)
						{
							std::cout << e.getSQLState() << std::endl;
						}
						SQL.unlock();
						//update rb1
						ss.clear();
						ss.str("");
						SQL_day.clear();
						SQL_day = "insert into F_daily_data (F_time,F_index, F_open, F_close, F_high, F_low, F_open_interest, F_volume, F_max_high, F_max_low,F_pre_settle) values ";
						temp_min_data = io12[io12.size() - 1];
						ss << "('" << date_today.year << "-" << date_today.month << "-" << date_today.day << "','" << temp_min_data.index << "'," << temp_min_data.open << "," << temp_min_data.last << "," << temp_min_data.high << "," << temp_min_data.low << "," << temp_min_data.open_interest << "," << temp_min_data.volume << "," << temp_min_data.max_high << "," << temp_min_data.max_low << "," << temp_min_data.pre_settle << ")";
						SQL_day += ss.str();
						SQL.lock();
						try
						{
							stat->executeUpdate(SQL_day);
						}
						catch (sql::SQLException &e)
						{
							std::cout << e.getSQLState() << e.what() << std::endl;
						}
						SQL.unlock();
					}
					else
					{
						std::stringstream ss;
						std::string SQL_day = "insert into F_daily_data (F_time,F_index, F_open, F_close, F_high, F_low, F_open_interest, F_volume, F_max_high, F_max_low,F_pre_settle) values ";
						min_data temp_min_data = io01[io01.size() - 1];
						ss << "('" << date_today.year << "-" << date_today.month << "-" << date_today.day << "','" << temp_min_data.index << "'," << temp_min_data.open << "," << temp_min_data.last << "," << temp_min_data.high << "," << temp_min_data.low << "," << temp_min_data.open_interest << "," << temp_min_data.volume << "," << temp_min_data.max_high << "," << temp_min_data.max_low << "," << temp_min_data.pre_settle << ")";
						SQL_day += ss.str();
						SQL.lock();
						try
						{
							stat->executeUpdate(SQL_day);
						}
						catch (sql::SQLException &e)
						{
							std::cout << e.getSQLState() << e.what() << std::endl;
						}
						SQL.unlock();
						//update rb1
						ss.clear();
						ss.str("");
						SQL_day.clear();
						SQL_day = "insert into F_daily_data (F_time,F_index, F_open, F_close, F_high, F_low, F_open_interest, F_volume, F_max_high, F_max_low,F_pre_settle) values ";
						temp_min_data = io11[io11.size() - 1];
						ss << "('" << date_today.year << "-" << date_today.month << "-" << date_today.day << "','" << temp_min_data.index << "'," << temp_min_data.open << "," << temp_min_data.last << "," << temp_min_data.high << "," << temp_min_data.low << "," << temp_min_data.open_interest << "," << temp_min_data.volume << "," << temp_min_data.max_high << "," << temp_min_data.max_low << "," << temp_min_data.pre_settle << ")";
						SQL_day += ss.str();
						SQL.lock();
						try
						{
							stat->executeUpdate(SQL_day);
						}
						catch (sql::SQLException &e)
						{
							std::cout << e.getSQLState() << e.what() << std::endl;
						}
						SQL.unlock();
					}
					std::string temp_string;
					std::stringstream ss;
					ss << "delete from io_0 where time1 > '" << date_today.year << "-" << date_today.month << "-" << date_today.day << " 15:05:00'";
					temp_string = ss.str();
					SQL.lock();
					stat->execute(temp_string);
					SQL.unlock();
					ss.clear();
					ss.str("");
					ss << "delete from io_1 where time1 > '" << date_today.year << "-" << date_today.month << "-" << date_today.day << " 15:05:00'";
					temp_string = ss.str();
					SQL.lock();
					stat->execute(temp_string);
					SQL.unlock();
					ss.clear();
					ss.str("");
					ss << "delete from f_min_data where f_time > '" << date_today.year << "-" << date_today.month << "-" << date_today.day << " 15:05:00' and (f_index = 'io0' or f_index = 'io1')";
					temp_string = ss.str();
					SQL.lock();
					stat->execute(temp_string);
					SQL.unlock();
				}
				else
				{
					std::string temp_date_today;
					std::string temp_string;
					std::stringstream ss;
					ss << date_today.year << "-" << date_today.month << "-" << date_today.day;
					temp_date_today = ss.str();
					ss.clear();
					ss.str("");
					ss << "delete from io_0 where time1 > '" << temp_date_today << " 23:35:00' or (time1 > '" << temp_date_today << " 15:05:00' and time1 <'" << temp_date_today << " 20:55:00') or (time1 > '" << temp_date_today << " 00:00:00' and time1 <'" << temp_date_today << " 08:55:00')";
					temp_string = ss.str();
					SQL.lock();
					stat->execute(temp_string);
					SQL.unlock();
					ss.clear();
					ss.str("");
					ss << "delete from io_1 where time1 > '" << temp_date_today << " 23:35:00' or (time1 > '" << temp_date_today << " 15:05:00' and time1 <'" << temp_date_today << " 20:55:00') or (time1 > '" << temp_date_today << " 00:00:00' and time1 <'" << temp_date_today << " 08:55:00')";
					temp_string = ss.str();
					SQL.lock();
					stat->execute(temp_string);
					SQL.unlock();
					ss.clear();
					ss.str("");
					ss << "delete from f_min_data where (f_time > '" << temp_date_today << " 23:35:00' or (f_time > '" << temp_date_today << " 15:05:00' and f_time <'" << temp_date_today << " 20:55:00') or (f_time > '" << temp_date_today << " 00:00:00' and f_time <'" << temp_date_today << " 08:55:00')) and (f_index = 'io0' or f_index = 'io1')";
					temp_string = ss.str();
					SQL.lock();
					stat->execute(temp_string);
					SQL.unlock();
				}
				return;
			}
		}
}


void add_min_data_not_beifen(std::stringstream & SQL_min, min_data & tick_data, min_data_min & min_data, update_time & date, std::string & pre_time, bool rizhong)
{
	if (pre_time == "")
	{
		if (date.hour < 9)
		{
			if (!(tick_data.time >= "08:55:00" && tick_data.time <= "09:05:00"))
			{
				return;
			}
		}
		else if (date.hour >= 16 && date.hour < 21)
		{
			if (!(tick_data.time >= "20:55:00" && tick_data.time <= "21:05:00"))
			{
				return;
			}
		}
	}
	else
	{
		SQL_min << date.year << "-" << date.month << "-" << date.day;
		SQL_min << " " << tick_data.time << " " << tick_data.millsec;
		std::string temp_time = SQL_min.str();
		SQL_min.clear();
		SQL_min.str("");
		if (temp_time <= pre_time)
		{
			return;
		}
	}
	if (min_data.date == "")
	{
		std::stringstream ss;
		ss << date.year << "-" << date.month << "-" << date.day;
		min_data.date = ss.str();
		min_data.time = tick_data.time.substr(0,5) + ":00";
		min_data.open = tick_data.last;
		min_data.high = tick_data.last;
		min_data.low = tick_data.last;
		min_data.close = tick_data.last;
		min_data.open_interest = (tick_data.open_interest - tick_data.pre_open_interest);
		min_data.volume = tick_data.volume - tick_data.old_volume;
	}
	else
	{
		if (min_data.time.substr(0, 5) != tick_data.time.substr(0, 5))
		{
			SQL_min << "('" << date.year << "-" << date.month << "-" << date.day;
			SQL_min << " " << min_data.time << "','" << tick_data.index << "'," << min_data.open << "," << min_data.close << "," << min_data.high << "," << min_data.low << "," << min_data.open_interest << "," << min_data.volume << "),";
			min_data.time = tick_data.time.substr(0, 5) + ":00";
			min_data.open = tick_data.last;
			min_data.high = tick_data.last;
			min_data.low = tick_data.last;
			min_data.close = tick_data.last;
			min_data.open_interest = tick_data.open_interest - tick_data.old_open_interest;
			min_data.volume = tick_data.volume - tick_data.old_volume;
			if (rizhong)
			{
				SQL_min << "('" << date.year << "-" << date.month << "-" << date.day;
				SQL_min << " " << min_data.time << "','" << tick_data.index << "'," << min_data.open << "," << min_data.close << "," << min_data.high << "," << min_data.low << "," << min_data.open_interest << "," << min_data.volume << "),";
			}
		}
		else
		{
			if (tick_data.last > min_data.high)
			{
				min_data.high = tick_data.last;
			}
			if (tick_data.last < min_data.low)
			{
				min_data.low = tick_data.last;
			}
			min_data.close = tick_data.last;
			min_data.open_interest += tick_data.open_interest - tick_data.old_open_interest;
			min_data.volume += tick_data.volume - tick_data.old_volume;
			if (rizhong)
			{
				SQL_min << "('" << date.year << "-" << date.month << "-" << date.day;
				SQL_min << " " << min_data.time << "','" << tick_data.index << "'," << min_data.open << "," << min_data.close << "," << min_data.high << "," << min_data.low << "," << min_data.open_interest << "," << min_data.volume << "),";
			}
		}
	}
}

void add_min_data_is_beifen(std::stringstream & SQL_min, min_data & tick_data, min_data_min & min_data, update_time & date, std::string & pre_time, bool rizhong)
{
	if (pre_time == "")
	{
		if (date.hour < 9)
		{
			if (!(tick_data.time >= "08:55:00" && tick_data.time <= "09:05:00"))
			{
				return;
			}
		}
		else if (date.hour > 16 && date.hour < 21)
		{
			if (!(tick_data.time >= "20:55:00" && tick_data.time <= "21:05:00"))
			{
				return;
			}
		}
	}
	else
	{
		SQL_min << date.year << "-" << date.month << "-" << date.day;
		SQL_min << " " << tick_data.time << " " << tick_data.millsec;
		std::string temp_time = SQL_min.str();
		SQL_min.clear();
		SQL_min.str("");
		if (temp_time <= pre_time)
		{
			return;
		}
	}
	if (min_data.time.substr(0, 5) != tick_data.time.substr(0, 5))
	{
		SQL_min << "('" << date.year << "-" << date.month << "-" << date.day;
		SQL_min <<" " << min_data.time << "','" << tick_data.index << "'," << min_data.open << "," << min_data.close << "," << min_data.high << "," << min_data.low << "," << min_data.open_interest << "," << min_data.volume << "),";
		min_data.time = tick_data.time.substr(0, 5) + ":00";
		min_data.open = tick_data.last;
		min_data.high = tick_data.last;
		min_data.low = tick_data.last;
		min_data.close = tick_data.last;
		min_data.open_interest = tick_data.open_interest - tick_data.old_open_interest;
		min_data.volume = tick_data.volume - tick_data.old_volume;
		if (rizhong)
		{
			SQL_min << "('" << date.year << "-" << date.month << "-" << date.day;
			SQL_min << " " << min_data.time << "','" << tick_data.index << "'," << min_data.open << "," << min_data.close << "," << min_data.high << "," << min_data.low << "," << min_data.open_interest << "," << min_data.volume << "),";
		}
	}
	else
	{
		if (tick_data.last > min_data.high)
		{
			min_data.high = tick_data.last;
		}
		if (tick_data.last < min_data.low)
		{
			min_data.low = tick_data.last;
		}
		min_data.close = tick_data.last;
		min_data.open_interest += tick_data.open_interest - tick_data.old_open_interest;
		min_data.volume += tick_data.volume - tick_data.old_volume;
		if (rizhong)
		{
			SQL_min << "('" << date.year << "-" << date.month << "-" << date.day;
			SQL_min << " " << min_data.time << "','" << tick_data.index << "'," << min_data.open << "," << min_data.close << "," << min_data.high << "," << min_data.low << "," << min_data.open_interest << "," << min_data.volume << "),";
		}
	}
}

void add_tick_data_not_beifen(std::stringstream & SQL_tick, min_data & tick_data, std::string & pre_time, update_time & date)
{
	if (pre_time == "")
	{
		if (date.hour < 9)
		{
			if (tick_data.time >= "08:55:00" && tick_data.time <= "09:05:00")
			{
				SQL_tick << date.year << "-" << date.month << "-" << date.day;
				SQL_tick << " " << tick_data.time << " " << tick_data.millsec;
				std::string temp_time = SQL_tick.str();
				SQL_tick.clear();
				SQL_tick.str("");
				pre_time = temp_time;
			}
			else
			{
				return;
			}
		}
		else if (date.hour > 16 && date.hour < 21)
		{
			if (tick_data.time >= "20:55:00" && tick_data.time <= "21:05:00")
			{
				SQL_tick << date.year << "-" << date.month << "-" << date.day;
				SQL_tick << " " << tick_data.time << " " << tick_data.millsec;
				std::string temp_time = SQL_tick.str();
				SQL_tick.clear();
				SQL_tick.str("");
				pre_time = temp_time;
			}
			else
			{
				return;
			}
		}
	}
	else
	{
		SQL_tick << date.year << "-" << date.month << "-" << date.day;
		SQL_tick << " " << tick_data.time << " " << tick_data.millsec;
		std::string temp_time = SQL_tick.str();
		SQL_tick.clear();
		SQL_tick.str("");
		if (temp_time <= pre_time)
		{
			return;
		}
		pre_time = temp_time;
	}
	if (tick_data.old_open_interest == 0)
	{
		if (tick_data.ask > tick_data.max_high)
		{
			std::cout << "涨停价" << tick_data.ask << std::endl;
			tick_data.ask = tick_data.max_high;
			tick_data.vask = 0;
		}
		if (tick_data.bid < tick_data.max_low)
		{
			std::cout << "跌停价" << tick_data.bid << std::endl;
			tick_data.bid = tick_data.max_low;
			tick_data.vbid = 0;
		}
		SQL_tick << "('" << date.year << "-" << date.month << "-" << date.day;
		SQL_tick << " " << tick_data.time << "'," << tick_data.millsec << "," << tick_data.last << "," << tick_data.ask << "," << tick_data.bid << "," << tick_data.vask << "," << tick_data.vbid << "," << tick_data.volume << "," << (tick_data.open_interest - tick_data.pre_open_interest) << "),";
	}
	else
	{
		if (tick_data.ask > tick_data.max_high)
		{
			std::cout << "涨停价" << tick_data.ask << std::endl;
			tick_data.ask = tick_data.max_high;
			tick_data.vask = 0;
		}
		if (tick_data.bid < tick_data.max_low)
		{
			std::cout << "跌停价" << tick_data.bid << std::endl;
			tick_data.bid = tick_data.max_low;
			tick_data.vbid = 0;
		}
		SQL_tick << "('" <<date.year << "-" << date.month << "-" << date.day;
		SQL_tick << " " << tick_data.time << "'," << tick_data.millsec << "," << tick_data.last << "," << tick_data.ask << "," << tick_data.bid << "," << tick_data.vask << "," << tick_data.vbid << "," << (tick_data.volume - tick_data.old_volume) << "," << (tick_data.open_interest - tick_data.old_open_interest) << "),";
	}
}

void add_tick_data_is_beifen(std::stringstream & SQL_tick, min_data & tick_data, std::string & pre_time, update_time & date)
{
	if (pre_time == "")
	{
		if (date.hour < 9)
		{
			if (tick_data.time >= "08:55:00" && tick_data.time <= "09:05:00")
			{
				SQL_tick << date.year << "-" << date.month << "-" << date.day;
				SQL_tick << " " << tick_data.time << " " << tick_data.millsec;
				std::string temp_time = SQL_tick.str();
				SQL_tick.clear();
				SQL_tick.str("");
				pre_time = temp_time;
			}
			else
			{
				return;
			}
		}
		else if (date.hour > 16 && date.hour < 21)
		{
			if (tick_data.time >= "20:55:00" && tick_data.time <= "21:05:00")
			{
				SQL_tick << date.year << "-" << date.month << "-" << date.day;
				SQL_tick << " " << tick_data.time << " " << tick_data.millsec;
				std::string temp_time = SQL_tick.str();
				SQL_tick.clear();
				SQL_tick.str("");
				pre_time = temp_time;
			}
			else
			{
				return;
			}
		}
	}
	else
	{
		SQL_tick << date.year << "-" << date.month << "-" << date.day;
		SQL_tick << " " << tick_data.time << " " << tick_data.millsec;
		std::string temp_time = SQL_tick.str();
		SQL_tick.clear();
		SQL_tick.str("");
		if (temp_time <= pre_time)
		{
			return;
		}
		pre_time = temp_time;
	}
	if (tick_data.ask > tick_data.max_high)
	{
		std::cout << "涨停价" << tick_data.ask << std::endl;
		tick_data.ask = tick_data.max_high;
		tick_data.vask = 0;
	}
	if (tick_data.bid < tick_data.max_low)
	{
		std::cout << "跌停价" << tick_data.bid << std::endl;
		tick_data.bid = tick_data.max_low;
		tick_data.vbid = 0;
	}
	SQL_tick << "('" << date.year << "-" << date.month << "-" << date.day;
	SQL_tick <<" " << tick_data.time << "'," << tick_data.millsec << "," << tick_data.last << "," << tick_data.ask << "," << tick_data.bid << "," << tick_data.vask << "," << tick_data.vbid << "," << (tick_data.volume - tick_data.old_volume) << "," << (tick_data.open_interest - tick_data.old_open_interest) << "),";
}
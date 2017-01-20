#include "MdSpi.h"
#include <thread>
#include <condition_variable>
#include <mutex>
#include <vector>
#include "mysql_driver.h"
#include "mysql_connection.h"
#include "cppconn/driver.h"
#include "cppconn/statement.h"
#include "cppconn/prepared_statement.h"
#include "cppconn/metadata.h"
#include "cppconn/exception.h"
#include <sstream>

struct update_time
{
	int year;
	int month;
	int day;
	int hour;
};
void add_data_rb();
void add_data_io();
void add_min_data_is_beifen(std::stringstream & SQL_min, min_data & tick_data, min_data_min & min_data, update_time & date, std::string & pre_time, bool rizhong = false);
void add_min_data_not_beifen(std::stringstream & SQL_min, min_data & tick_data, min_data_min & min_data, update_time & date, std::string & pre_time, bool rizhong = false);
void add_tick_data_is_beifen(std::stringstream & SQL_tick, min_data & tick_data, std::string & pre_time, update_time & date);
void add_tick_data_not_beifen(std::stringstream & SQL_tick, min_data & tick_data, std::string & pre_time, update_time & date);
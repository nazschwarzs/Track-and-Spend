#ifndef DB_CONNECTION_H
#define DB_CONNECTION_H

#include <jdbc/mysql_driver.h>
#include <jdbc/cppconn/connection.h>

sql::Connection *connectDB();

#endif
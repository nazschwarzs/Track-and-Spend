#include "db_connection.h"
#include <iostream>

sql::Connection *connectDB()
{
    try
    {
        sql::Driver *driver = get_driver_instance();
        sql::Connection *con = driver->connect("tcp://127.0.0.1:3306", "root", "");
        con->setSchema("trackandspend_db");

        return con;
    }
    catch (sql::SQLException &e)
    {
        std::cout << "DB Connection Error: " << e.what() << std::endl;
        return nullptr;
    }
}
#include "Models.h"
#include <iostream>
#include <string>

using namespace NextFlix;

Models::Models()
{
    try
    {
        pqxx::connection conn("dbname=fetched_initial user=postgres password=password hostaddr=127.0.0.1 port=5432");

        if (conn.is_open())
        {
            std::cout << "Connected to the database successfully." << std::endl;
            pqxx::work txn(conn);
            pqxx::result result = txn.exec("SELECT * FROM \"Table\"");

            for (const auto &row : result)
            {
                std::string username = row["name"].as<std::string>();
                std::cout << "Name: " << username << std::endl;
            }

            txn.commit();
        }
        else
        {
            std::cerr << "Failed to connect to the database." << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
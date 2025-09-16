#include "cli.h"
#include <iostream>

static void console_clear()
{
    #if defined _WIN32
        system("cls");
    #elif defined (__LINUX__) || defined(__gnu_linux__) || defined(__linux__)
        system("clear"); 
    #elif defined (__APPLE__)
        system("clear");
    #endif
}

void DSS::CLI::init()
{
    console_clear();
    input_loop();
}

void DSS::CLI::execute(std::string what)
{
    if (m_bound_executor == nullptr) {return;}
    m_bound_executor->exec(what);
}

void DSS::CLI::input_loop()
{
    while (m_alive == true)
    {
        std::cout << "dss >>>";
        std::string retrieved; 
        getline(std::cin, retrieved);

        if (retrieved == "exit")
        {
            m_alive = false;
        }

        execute(retrieved);
    }
}
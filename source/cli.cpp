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

void cli::CLI::init()
{
    console_clear();
    input_loop();
}

void cli::CLI::execute(std::string what)
{
    if (m_bound_executor == nullptr) {return;}
    m_bound_executor->exec(what);
}

void cli::CLI::input_loop()
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
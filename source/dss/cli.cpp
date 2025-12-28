#include "cli.h"
#include <filesystem>
#include <iostream>

static void console_clear()
{
#if defined _WIN32
	system("cls");
#elif defined(__LINUX__) || defined(__gnu_linux__) || defined(__linux__)
	system("clear");
#elif defined(__APPLE__)
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
	if (m_bound_executor == nullptr)
	{
		return;
	}
	m_bound_executor->exec(what);
}

void DSS::CLI::input_loop()
{
	while (m_alive == true)
	{
		std::cout << "\033[34m" << std::filesystem::current_path().string() << "\033[0m \033[35m" << m_name << "\033[0m \033[1m \033[32m>>>\033[0m ";
		std::string retrieved;
		getline(std::cin, retrieved);

		if (retrieved == "exit")
		{
			console_clear();
			m_alive = false;
		}
		else if (retrieved == "clear")
		{
			console_clear();
		}

		execute(retrieved);
	}
}
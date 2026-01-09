#include <string>

#include "runtime.h"

#ifndef CLI_H
#define CLI_H

namespace DSS
{

const std::string DEFAULT_CLI_NAME = "dss";

class cli_t
{
private:
	std::shared_ptr<DSS::executor_t> m_bound_executor;
	bool m_alive;
	std::string m_name;

public:
	cli_t(std::shared_ptr<DSS::executor_t> bound_executor, std::string name)
	{
		if (bound_executor == nullptr)
		{
			return;
		}
		m_bound_executor = bound_executor;
		m_alive = true;

		m_name = name;
	}

	cli_t(std::shared_ptr<DSS::executor_t> bound_executor)
	{
		if (bound_executor == nullptr)
		{
			return;
		}
		m_bound_executor = bound_executor;
		m_alive = true;

		m_name = DEFAULT_CLI_NAME;
	}

	void init();

	void execute(std::string what);

	void input_loop();
};

} // namespace DSS

#endif
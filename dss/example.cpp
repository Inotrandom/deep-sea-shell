/**
 * Example testing program which uses DSS
 */

#include "DSS.h"

int main()
{
	DSS::environment_t env = DSS::environment_t();

	env.init();
	std::optional<DSS::executor_t> opt_main_executor = env.main_executor();

	if (opt_main_executor.has_value() == false)
	{
		return 1;
	}

	DSS::executor_t main_executor = opt_main_executor.value();

	DSS::cli_t cli = DSS::cli_t(&main_executor);
	cli.init(); // TODO: Execute "src example.dss" in the console to see DSS in action

	return 0;
}
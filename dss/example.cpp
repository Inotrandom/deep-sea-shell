/**
 * Example testing program which uses DSS
 */

#include "DSS.h"

int main()
{
	DSS::environment_t env = DSS::environment_t();

	env.init();
	std::shared_ptr<DSS::executor_t> main_ex = env.main_executor();

	if (main_ex == nullptr)
	{
		return 1;
	}

	DSS::cli_t cli = DSS::cli_t(main_ex);
	cli.init(); // TODO: Execute "src example.dss" in the console to see DSS in action

	return 0;
}
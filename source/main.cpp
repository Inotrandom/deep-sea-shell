/**
 * Example testing program which uses DSS
 */

#include <iostream>
#include "runtime.h"
#include "lang.h"
#include "utils.h"

int main(int argv, char** argc)
{
    runtime::Environment env = runtime::Environment();
    env.connect_preprocessor_definer(lang::preprocessor_definer);
    env.connect_command_definer(lang::command_definer);

    env.init();
    std::optional<runtime::Executor> opt_main_executor = env.main_executor();

    if (opt_main_executor.has_value() == false)
    {
        return 1;
    }

    runtime::Executor main_executor = opt_main_executor.value();
    main_executor.apply_error_key(lang::ERR_KEY);

    main_executor.exec("out DSS Lovingly says \"Hello, world!\"");
    
    //TODO: Uncomment and run the example script
    //main_executor.exec("src ../example.dss");

    return 0;
}
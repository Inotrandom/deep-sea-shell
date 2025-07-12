/**
 * DSS is a high-level data-oriented operative programming language. Among others,
 * its primary purpose is for communication and control of robotic systems.
 * DSS is intended to exist as both an effective means of storing, parsing, and
 * using data, but also as a simple tool for operating systems.
 * 
 * DSS is designed not just for human users, but also for systems to universally
 * send/recieve data packets on a network.
 * 
 * Grafting to DSS is a core language feature; producing your own organized commands
 * and namespaces is important to ensure the effectiveness of the language for your
 * specific needs.
 * 
 * Ideally, DSS should be placed into the main file of your codespace. This file
 * is both an example and a starting point for using DSS.
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
    main_executor.apply_error_key(lang::err_key);

    main_executor.exec("out DSS Lovingly says \"Hello, world!\"");
    main_executor.exec("src ../example.dss");

    return 0;
}
/**
 * This file contains core language functionality
 * charactaristic of ordinary DSS.
 * 
 * Every distribution of DSS should have these
 * commands, as they are fundamental language
 * features.
 */

#ifndef lang
#define lang

#include "utils.h"
#include "runtime.h"

#include <iostream>
#include <format>
#include <any>

const std::string NAME = "lang";

namespace func
{
    auto out(runtime::Executor ex, utils::DSSFuncArgs args) -> utils::DSSReturnType
    {
        for (auto argument : args)
        {
            std::cout << argument;
            std::cout << " ";
        }

        std::cout << std::endl;

        return 0;
    }
}

static std::any definer(Executor *exec)
{
    if (exec == nullptr) {return NULL;}

    exec->define_command(
        func::out,
        "out",
        "outputs to console"
    );

    return NULL;
}

#endif
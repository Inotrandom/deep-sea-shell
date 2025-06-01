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

#include "utils.h" // std:vector, std::string, std::algorithm, and std::iterator
#include "runtime.h" // std::any and std::optional

#include <iostream>
#include <format>

const std::string VAR_ALIAS = "alias";

struct Alias
{
    std::string id;
    std::string value;
};
typedef std::vector<Alias> AliasVarData;

auto create_alias(runtime::Executor& ex, std::string id, std::string value) -> utils::DSSReturnType
{
    runtime::Vars vars = ex.get_vars();
    std::optional<runtime::Var<std::any>> found = vars.get_var(VAR_ALIAS);

    Alias new_alias = Alias();
    new_alias.id = id;
    new_alias.value = value;

    if (found.has_value() == false) 
    {
        AliasVarData new_data = {new_alias};
        vars.init_var(VAR_ALIAS, new_data);
        
        return 0;
    }

    try
    {
        runtime::Var<AliasVarData> var_alias = std::any_cast<Var<AliasVarData>>(found.value());
        var_alias.data.push_back(new_alias);

        return 0;
    }
    catch (std::bad_any_cast& _e)
    {
        return 1;
    }
}

const std::string NAME = "lang";

namespace func
{
    auto out(runtime::Executor& ex, utils::DSSFuncArgs args) -> utils::DSSReturnType
    {
        for (auto argument : args)
        {
            std::cout << argument;
            std::cout << " ";
        }

        std::cout << std::endl;

        return 0;
    }

    auto alias(runtime::Executor& ex, utils::DSSFuncArgs args) -> utils::DSSReturnType
    {
        std::string id = args[0];
        args.erase(args.begin());
        std::string value;

        for (auto argument : args)
        {
            value += argument;
        }

        std::cout << id << value << std::endl;
        return create_alias(ex, id, value);
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

    exec->define_command(
        func::alias,
        "alias",
        "creates an alias",
        2
    );

    return NULL;
}

#endif
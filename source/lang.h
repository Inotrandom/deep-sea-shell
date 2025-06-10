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

const std::string ALIAS_VAR = "alias";
const std::string ALIAS_DEREF = "$";
const std::string ALIAS_USE = "alias";

struct Alias
{
    std::string id;
    std::string value;

    bool operator<=>(const Alias&) const = default;
};
typedef std::vector<Alias> AliasVarData;

/**
 * Applies alias as an automatic preprocessor
 */
void use_alias(runtime::Executor* p_ex)
{
    //std::cout << "Alias is used!" << std::endl;
    runtime::Vars &vars = p_ex->get_vars();
    runtime::Var<std::any> *auto_preproc_var = vars.get_or_add_var(runtime::AUTO_PREPROCESSOR_VAR);

    if (auto_preproc_var == nullptr) {return;}

    auto_preproc_var->append_data(ALIAS_USE);
}

/**
 * Creates an alias.
 * 
 * This function is not intended to be 
 * called directly outside of the
 * `alias` command definitions. As such, it
 * will assume that all pointers passed into
 * it are not nullptr
 * 
 * @param p_ex A pointer to the executor in which the alias will be created.
 * 
 * @param id The name of the alias
 * 
 * @param data The data of the alias
 */
auto create_alias(runtime::Executor* p_ex, std::string id, std::string data) -> utils::DSSReturnType
{
    runtime::Vars &vars = p_ex->get_vars();
    runtime::Var<std::any> *alias_var = vars.get_or_add_var(ALIAS_VAR);

    if (alias_var == nullptr) {return 1;}
    
    Alias new_alias = Alias();
    new_alias.id = id;
    new_alias.value = data;

    alias_var->append_data(new_alias);

    return 0;
}

const std::string NAME = "lang";

namespace func
{
    auto out(runtime::Executor* p_ex, utils::DSSFuncArgs args) -> utils::DSSReturnType
    {
        if (p_ex == nullptr) {return 1;}

        for (auto argument : args)
        {
            std::cout << argument;
            std::cout << " ";
        }

        std::cout << std::endl;

        return 0;
    }

    auto alias_def(runtime::Executor* p_ex, utils::DSSFuncArgs args) -> utils::DSSReturnType
    {
        if (p_ex == nullptr) {return 1;}

        use_alias(p_ex); // Aliases are in use!

        std::string id = args[0];
        args.erase(args.begin());
        std::string value;

        for (auto argument : args)
        {
            value += argument;
        }

        //std::cout << id << value << std::endl;
        return create_alias(p_ex, id, value);
    }

    auto alias(runtime::Executor* p_ex, utils::DSSFuncArgs args) -> utils::DSSReturnType
    {
        //std::cout << "Alias is called!" << std::endl;
        if (p_ex == nullptr) {return 1;}

        runtime::Task *p_current_task = p_ex->get_current_task();
        if (p_current_task == nullptr) {return 1;}

        runtime::Vars &vars = p_ex->get_vars();
        runtime::Var<std::any> *alias_var = vars.get_var(ALIAS_VAR);
        if (alias_var == nullptr) {return 1;}

        std::string &script = p_current_task->get_script();

        for (auto element : alias_var->get_data())
        {
            Alias alias = std::any_cast<Alias>(element);
            
            std::string deref = ALIAS_DEREF + alias.id;
            string_replace(script, deref, alias.value);
            //std::cout << "dereferencing: " << deref << std::endl;
            //std::cout << "the script was modified to: " << script << std::endl;
        }

        return 0;
    }
}

static std::any preprocessor_definer(Executor *exec)
{
    if (exec == nullptr) {return NULL;}

    exec->define_command(
        func::alias_def,
        "alias_def",
        "creates an alias",
        2
    );

    exec->define_command(
        func::alias,
        "alias",
        "applies aliases"
    );

    return NULL;
}

static std::any command_definer(Executor *exec)
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
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
#include <filesystem>

/**
 * Name of the alias environment variable
 */
const std::string ALIAS_VAR = "alias";

/**
 * Key sequence for an alias dereference.
 * A prepend of this and the alias's id
 * signals the parser to complete a dereference.
 */
const std::string ALIAS_DEREF = "$";

/**
 * Command for automatically using an alias.
 */
const std::string ALIAS_USE = "alias";

/**
 * Aliases are versatile preprocessor macros that
 * replace parts of the code before the "command" pass.
 * 
 * This is idiomatic to `#define` in C.
 * 
 * The particular class is a structure to store alias
 * values.
 */
struct Alias
{
    /**
     * The identifier of the alias. This, in combination
     * with `ALIAS_DEREF`, is used by the parser to figure
     * out where aliases should be applied.
     */
    std::string id;

    /**
     * The value of the alias. This is what physically
     * replaces instances of an alias dereference
     */
    std::string value;

    bool operator<=>(const Alias&) const = default;
};
typedef std::vector<Alias> AliasVarData;

/**
 * Applies `alias` as an automatic preprocessor.
 * If no aliases are defined in the executor, then 
 * this function will never get called. Consequently,
 * the `alias` preprocessor will never become automatic.
 */
void use_alias(runtime::Executor* p_ex)
{
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
    /**
     * Out will push arguments into the stdout stream.
     */
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

    /**
     * Alias Define will define an alias and call `use_alias`
     */
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

        return create_alias(p_ex, id, value);
    }

    /**
     * Alias will apply defined aliases throughout
     * the script lazily.
     */
    auto alias(runtime::Executor* p_ex, utils::DSSFuncArgs args) -> utils::DSSReturnType
    {
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
        }

        return 0;
    }

    auto source(runtime::Executor *p_ex, utils::DSSFuncArgs args) -> utils::DSSReturnType
    {
        if (p_ex == nullptr) {return 1;}

        std::optional<std::string> res = utils::file_read(args[0]);

        //std::cout << std::filesystem::current_path();
        //std::cout << res.has_value();

        if (res.has_value() == false) {return 2;}

        runtime::Task task = runtime::Task(res.value());
        p_ex->queue_task(task);

        return 0;
    }
}

/**
 * Definer for `lang` preprocessors
 */
static std::any preprocessor_definer(Executor *exec)
{
    if (exec == nullptr) {return NULL;}

    exec->define_command(
        func::source,
        "src",
        "runs a dss script at path <path>"
    );

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

/**
 * Definer for `command` preprocessors
 */
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
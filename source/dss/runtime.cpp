/**
 * @file DSS.cpp
 * @author Yevgenya Coonan (yacoonan@gmail.com)
 * @brief Deep Sea Shell runtime
 * @version 1.1.1
 * @date 2025-09-15
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "utils.h"
#include "runtime.h"
#include "dss_lang.h"
#include <any>
#include <optional>
#include <iostream>
#include <string>
#include <sstream>

void DSS::push_error(std::string what, int line)
{
    std::cout << std::endl << "error: a critical exception occurred";
    
    if (line > -1)
    {
        std::cout << " on line " << line + 1;
    }

    std::cout << std::endl;
    std::cout << what << std::endl;
}

void DSS::Executor::find_and_push_error(std::string command, int code, int line)
{
    try
    {
        DSS::ErrCodes found = m_lookup_error.at(command);

        std::string error = found.at(code);

        DSS::push_error(error, line);
    }
    catch (std::out_of_range &_e)
    {
        DSS::push_error(DSS::err::UNKNOWN, line);
        return;
    }
}

void DSS::Executor::direct_exec(DSS::StrVec statements)
{
    int line = -1;

    for (auto statement : statements)
    {
        line++;
        DSS::StrVec parsed = utils::string_split(statement, DSS::key::TOKEN_DELIM);

        // No command
        if (parsed.size() == 0)
        {
            continue;
        }

        DSS::DSSDelegateReturnType res = {};

        for (auto command : m_loaded_commands)
        {
            DSS::DSSDelegateReturnType opt_res = command.attempt_parse_and_exec(parsed, line);

            if (opt_res.size() == 0)
            {
                continue; // Failed to parse
            }

            res = opt_res;
        }

        if (res.size() == 0)
        {
            continue; // Command does not exist
        }

        // Successful execution
        if (res[0] == 0)
        {
            continue;
        }

        // The first element of parsed is the keyword, and the first element of res is the returned value
        find_and_push_error(parsed[0], res[0], line);
    }
}

void DSS::Executor::auto_preprocessors()
{
    std::shared_ptr<DSS::Var<std::any>> auto_preprocessor_var = m_exec_vars.get_or_add_var(DSS::AUTO_PREPROCESSOR_VAR);
    if (auto_preprocessor_var == nullptr) {return;} // Impossible

    std::vector<std::string> statements = {};
    for (auto element : auto_preprocessor_var->get_data())
    {
        statements.push_back( std::any_cast<std::string>(element) );
    }

    direct_exec(statements);
}

void DSS::Executor::command_pass(DSS::DefinerDelegate definer, DSS::StrVec statements)
{
    m_loaded_commands.clear(); // Remove all currently defined commands (to mitigate interference)
    definer.call(this);

    direct_exec(statements);
}

auto DSS::Executor::exec_task(DSS::Task task) -> DSS::DSSReturnType
{
    DSS::DSSReturnType res = 1;
    m_current_task = &task;

    std::string &script = task.get_script();
    DSS::StrVec statements = utils::string_split(script, DSS::key::MULTILINE_DELIM);

    command_pass(m_additional_preprocessors, statements);
    auto_preprocessors();

    statements = utils::string_split(script, DSS::key::MULTILINE_DELIM); // Update after the preprocessors are finished

    command_pass(m_additional_commands, statements);

    return 0;
}

auto DSS::Executor::exec_all_tasks(bool recursive) -> DSS::DSSDelegateReturnType
{
    DSS::DSSDelegateReturnType res = {};

    if (m_tasks.size() == 0) {return res;}
    if (m_busy == true) {return res;}
    m_busy = true;

    for (auto task : m_tasks)
    {
        exec_task(task);
    }
    
    m_busy = false;
    m_tasks.clear(); // All tasks are completed, whether successfully or not
    if (recursive == true)
    {
        m_tasks.insert(m_tasks.end(), m_task_buffer.begin(), m_task_buffer.end());
        m_task_buffer.clear();

        exec_all_tasks(DSS::key::FLAG_RECURSIVE_EXECUTION);
    }

    return res;
}

void DSS::Environment::apply_error_key(DSS::ErrKey key)
{
    m_lookup_error.insert(key.begin(), key.end());
}

void DSS::Executor::exec(std::string script)
{
    DSS::Task task = DSS::Task(script);
    m_tasks.push_back(task); // Append a task to the task list
    exec_all_tasks(DSS::key::FLAG_RECURSIVE_EXECUTION); // Invoke the executor
}

auto DSS::Environment::executor_by_id(DSS::RunID id) -> std::optional<Executor>
{
    for (auto executor : m_executors)
    {
        if (executor.get_id() != id) {continue;}

        return executor;
    }

    return std::nullopt;
}

void DSS::Environment::init()
{
    connect_preprocessor_definer(lang::preprocessor_definer);
    connect_command_definer(lang::command_definer);
    
    spawn_executor();
}

const std::string ERROR_TOO_MANY_ARGS = "Excessive amount of arguments provided to command";
const std::string ERROR_TOO_FEW_ARGS = "Too few arguments provided to command";

/**
 * Activates a pipeline which parses and executes
 * a statement.
 * 
 * @param inp The statement to parse
 * 
 * @param delim The delimiter to separate tokens
 */
auto DSS::Command::attempt_parse_and_exec(DSS::StrVec tokens, uint64_t line) -> DSSDelegateReturnType
{
    DSSDelegateReturnType res = {};
    
    if (tokens[0] != m_name) return {};

    tokens.erase(tokens.begin());

    std::size_t arg_count = tokens.size();

    if (m_maximum_args > -1 && arg_count > m_maximum_args)
    {
        std::stringstream msg;
        msg << ERROR_TOO_MANY_ARGS << " \"" << m_name << "\"";
        push_error(msg.str());

        return {};
    }

    if (m_minimum_args > -1 && arg_count < m_minimum_args)
    {
        std::stringstream msg;
        msg << ERROR_TOO_FEW_ARGS << " \"" << m_name << "\"";
        push_error(msg.str());

        return {};
    }

    if (m_parent_ex == nullptr) {return res;}

    res = m_delegate.call(m_parent_ex, tokens);
    return res;
}
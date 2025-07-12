/**
 * Within this file are DSS Runtime utilities.
 * It is critical to include this file to
 * use DSS.
 */
#include "utils.h"
#include "runtime.h"
#include <any>
#include <optional>
#include <iostream>

void runtime::push_error(std::string what, int line)
{
    std::cout << std::endl << "error: a critical exception occurred";
    
    if (line > -1)
    {
        std::cout << " on line " << line;
    }

    std::cout << std::endl;
    std::cout << what << std::endl;
}

void runtime::Executor::find_and_push_error(std::string command, int code, int line)
{
    try
    {
        runtime::ErrCodes found = m_lookup_error.at(command);

        std::string error = found.at(code);

        runtime::push_error(error, line);
    }
    catch (std::out_of_range &_e)
    {
        runtime::push_error(runtime::err::UNKNOWN, line);
        return;
    }
}

void runtime::Executor::direct_exec(runtime::StrVec statements)
{
    int line = -1;

    for (auto statement : statements)
    {
        line++;
        runtime::StrVec parsed = string_split(statement, runtime::key::TOKEN_DELIM);

        // No command
        if (parsed.size() == 0)
        {
            continue;
        }

        runtime::DSSDelegateReturnType res = {};

        for (auto command : m_loaded_commands)
        {
            runtime::DSSDelegateReturnType opt_res = command.attempt_parse_and_exec(parsed);

            if (opt_res.size() == 0)
            {
                continue;
            }

            res = opt_res;
        }

        if (res.size() == 0)
        {
            continue;
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

void runtime::Executor::auto_preprocessors()
{
    runtime::Var<std::any> *auto_preprocessor_var = m_exec_vars.get_or_add_var(runtime::AUTO_PREPROCESSOR_VAR);
    if (auto_preprocessor_var == nullptr) {return;} // Impossible

    std::vector<std::string> statements = {};
    for (auto element : auto_preprocessor_var->get_data())
    {
        statements.push_back( std::any_cast<std::string>(element) );
    }

    direct_exec(statements);
}

void runtime::Executor::command_pass(runtime::DefinerDelegate definer, runtime::StrVec statements)
{
    m_loaded_commands.clear(); // Remove all currently defined commands (to mitigate interference)
    definer.call(this);

    direct_exec(statements);
}

auto runtime::Executor::exec_task(runtime::Task task) -> runtime::DSSReturnType
{
    runtime::DSSReturnType res = 1;
    m_current_task = &task;

    std::string &script = task.get_script();
    runtime::StrVec statements = string_split(script, runtime::key::MULTILINE_DELIM);

    command_pass(m_additional_preprocessors, statements);
    auto_preprocessors();

    statements = string_split(script, runtime::key::MULTILINE_DELIM); // Update after the preprocessors are finished

    command_pass(m_additional_commands, statements);

    return 0;
}

auto runtime::Executor::exec_all_tasks(bool recursive) -> runtime::DSSDelegateReturnType
{
    runtime::DSSDelegateReturnType res = {};

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

        exec_all_tasks(runtime::key::FLAG_RECURSIVE_EXECUTION);
    }

    return res;
}

void runtime::Executor::apply_error_key(runtime::ErrKey key)
{
    m_lookup_error.insert(key.begin(), key.end());
}

void runtime::Executor::exec(std::string script)
{
    runtime::Task task = runtime::Task(script);
    m_tasks.push_back(task); // Append a task to the task list
    exec_all_tasks(runtime::key::FLAG_RECURSIVE_EXECUTION); // Invoke the executor
}

auto runtime::Environment::executor_by_id(runtime::RunID id) -> std::optional<Executor>
{
    for (auto executor : m_executors)
    {
        if (executor.get_id() != id) {continue;}

        return executor;
    }

    return std::nullopt;
}

/**
 * Activates a pipeline which parses and executes
 * a statement.
 * 
 * @param inp The statement to parse
 * 
 * @param delim The delimiter to separate tokens
 */
auto runtime::Command::attempt_parse_and_exec(runtime::StrVec tokens) -> DSSDelegateReturnType
{
    DSSDelegateReturnType res = {};
    
    if (tokens[0] != m_name) {return res;}

    tokens.erase(tokens.begin());

    std::size_t arg_count = tokens.size();

    if (m_parent_ex == nullptr) {return res;}

    res = m_delegate.call(m_parent_ex, tokens);
    return res;
}
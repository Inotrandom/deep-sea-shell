/**
 * This file contains runtime utilities.
 * 
 * Most notably, the DSS environment and
 * constants. This file is essentially
 * "DSS."
 */
#ifndef runtime
#define runtime

#include "utils.h"
#include <any>

namespace err
{
    const std::string NULL_ENV = "error: environment is null, definer for {} is unable to define commands";
}

namespace key
{
    const std::string MULTILINE_DELIM = "\n";
    const std::string TOKEN_DELIM = " ";

    const bool FLAG_RECURSIVE_EXECUTION = true;
}

/**
 * DSSTasks are scripts that require running
 * 
 * They may have metadata in the future
 */
class Task
{
private:
    std::string m_script;
public:
    inline Task(const std::string script)
    {
        m_script = script;
    }

    inline auto get_script() -> std::string
    {
        return m_script;
    }
};


class Environment;

typedef std::any (*Definer)(Environment*);
typedef utils::Delegate<Definer, Environment*, std::vector<std::any>> DefinerDelegate;

/**
 * DSS execution environment. Accepts and executes tasks.
 */
class Environment
{
private:
    /**
     * Every command currently working for this
     * particular "pass" of execution
     */
    std::vector<utils::Command> m_loaded_commands;

    /**
     * Any user-defined preprocessor definers
     */
    DefinerDelegate m_additional_preprocessors;

    /**
     * Any user-defined command definers
     */
    DefinerDelegate m_additional_commands;

    /**
     * Every single task that this environment
     * has yet to complete or is in the process
     * of completing
     */
    std::vector<Task> m_tasks;

    /**
     * Whether or not the environment is actively
     * completing tasks.
     */
    bool m_busy;

    /**
     * Directly executes a script.
     * 
     * This is a poor way to invoke DSS,
     * and this function is intended exclusively
     * for the internals of the interpreter.
     * 
     * @see Environment::exec_task
     */
    inline void direct_exec(utils::DSSFuncArgs statements)
    {
        for (auto statement : statements)
        {
            for (auto command : m_loaded_commands)
            {
                command.attempt_parse_and_exec(statement, key::TOKEN_DELIM);
            }
        }
    }

    /**
     * Command passes describe one singular execution of the program.
     * 
     * @param definer The function pointer that determines which commands
     * are active during the "pass"
     * 
     * @param statements A vector of individual command strings. These will
     * be further split into individual tokens by the command itself.
     */
    inline void command_pass(DefinerDelegate definer, utils::DSSFuncArgs statements)
    {
        if (this == nullptr) {return;}
        m_loaded_commands.clear(); // Remove all currently defined commands (to mitigate interference)
        definer.call(this);

        direct_exec(statements);
    }

    /**
     * Consider this the actual executor- this will
     * commence the parsing and execution process
     * of commands.
     * 
     * @param task The task to be completed
     * 
     * @return The result of executing the task.
     */
    inline auto exec_task(Task task) -> utils::DSSReturnType
    {
        utils::DSSReturnType res = 1;

        if (this == nullptr) {return res;}

        std::string script = task.get_script();
        std::vector<std::string> to_commands = string_split(script, key::MULTILINE_DELIM);

        command_pass(m_additional_preprocessors, to_commands);
        command_pass(m_additional_commands, to_commands);

        return 0;
    }

    /**
     * Executes every single task in the queue.
     * 
     * @param recursive Determines whether or not DSS will continue executing
     * new tasks as they arrive.
     * 
     * @return A vector containing the result of 
     * execution for every single task.
     */
    inline auto exec_all_tasks(bool recursive) -> utils::DSSDelegateReturnType
    {
        utils::DSSDelegateReturnType res = {};

        if (this == nullptr) {return res;}
        if (m_tasks.size() == 0) {return res;}
        if (m_busy == true) {return res;}
        m_busy = true;

        for (auto task : m_tasks)
        {
            exec_task(task);
        }
        
        m_tasks.clear();
        m_busy = false;
        if (recursive == true)
        {
            exec_all_tasks(key::FLAG_RECURSIVE_EXECUTION);
        }

        return res;
    }
public:
    inline Environment()
    {
        m_loaded_commands = {};
        m_additional_preprocessors = DefinerDelegate();
        m_additional_commands = DefinerDelegate();
        m_tasks = {};
    }

    inline void connect_preprocessor_definer(const Definer func)
    {
        if (this == nullptr) {return;}
        m_additional_preprocessors.connect(func);
    }

    inline void connect_command_definer(const Definer func)
    {
        if (this == nullptr) {return;}
        m_additional_commands.connect(func);
    }

    inline void define_command(
        utils::DSSFunc func,
        std::string name,
        std::string description,
        int minimum_args = -1,
        int maximum_args = -1
    )
    {
        if (this == nullptr) {return;}

        utils::Command res = utils::Command(func, name, description, minimum_args, maximum_args);
        m_loaded_commands.push_back(res);
    }

    inline void exec(std::string script)
    {
        if (this == nullptr) {return;}

        Task task = Task(script);
        m_tasks.push_back(task); // Append a task to the task list
        exec_all_tasks(key::FLAG_RECURSIVE_EXECUTION); // Invoke the executor
    }
};

#endif
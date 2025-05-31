/**
 * Within this file are DSS Runtime utilities.
 * It is critical to include this file to
 * use DSS.
 */
#ifndef runtime
#define runtime

#include "utils.h"
#include <any>
#include <optional>

typedef int64_t RunID;

class Executor;

/**
 * Consider that DSS Commands are essentially named Delegates.
 * 
 * A DSS Command has a name and a description. "Name" is the command's
 * keyword, and "description" should contain a basic manual for the command's
 * usage.
 * 
 * @note DSS Commands are fluid- they are defined and deleted at run time.
 * Assume that no value will be constant, as the command may be constructed
 * at arbitrary times.
 */
class Command
{
private:
    Delegate<DSSFunc, DSSFuncArgs, DSSDelegateReturnType> m_delegate;
    std::string m_name;
    std::string m_description;
    int64_t m_minimum_args;
    int64_t m_maximum_args;
    Executor* m_parent_env;

public:
    /**
     * @param name Keyword for the command. This will determine
     * what token invokes this command.
     * 
     * @param description The manual for the command.
     * 
     * @param minimum_args The minimum expected arguments for the command. (optional)
     * 
     * @param maximum_args The maximum of arguments expected for the command. (optional)
     */
    Command(
        DSSFunc func,
        std::string name,
        std::string description,
        int64_t minimum_args = -1,
        int64_t maximum_args = -1,
        Executor* parent_env = nullptr
    )
    {
        //m_delegate = Delegate<DSSFunc, DSSFuncArgs, DSSDelegateReturnType>();
        m_name = name;
        m_delegate.connect(func);
        m_description = description;
        m_minimum_args = minimum_args;
        m_maximum_args = maximum_args;
        m_parent_env = parent_env;
    }

    /**
     * Lazily attempts to run this command if the keyword (first token)
     * matches the command's "name"
     * 
     * @param inp The string command to be executed (should contain arguments)
     * @param delim The separation between individual tokens in a command.
     * 
     * @return The result of calling the delegate associated with this command.
     * Will return an empty vector if an error has occured;
     * 
     * @see DSSDelegate::call
     */
    auto attempt_parse_and_exec(std::string inp, std::string delim) -> DSSDelegateReturnType
    {
        DSSDelegateReturnType res = {};
        
        if (this == nullptr) {return res;}

        std::vector<std::string> tokens = string_split(inp, delim);
        
        if (tokens[0] != m_name) {return res;}

        tokens.erase(tokens.begin());

        std::size_t arg_count = tokens.size();

        //if (arg_count < m_minimum_args) {return res;}
        //if (m_maximum_args > -1 && arg_count > m_maximum_args) {return res;}

        res = m_delegate.call(tokens);
        return res;
    }
};

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
    Task(const std::string script)
    {
        m_script = script;
    }

    auto get_script() -> std::string
    {
        return m_script;
    }
};

typedef std::any (*Definer)(Executor*);
typedef utils::Delegate<Definer, Executor*, std::vector<std::any>> DefinerDelegate;

/**
 * DSS execution environment. Accepts and executes tasks.
 */
class Executor
{
private:
    /**
     * Every command currently working for this
     * particular "pass" of execution
     */
    std::vector<Command> m_loaded_commands;

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

    RunID m_id;

    /**
     * Directly executes a script.
     * 
     * This is a poor way to invoke DSS,
     * and this function is intended exclusively
     * for the internals of the interpreter.
     * 
     * @see Executor::exec_task
     */
    void direct_exec(utils::DSSFuncArgs statements)
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
    void command_pass(DefinerDelegate definer, utils::DSSFuncArgs statements)
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
    auto exec_task(Task task) -> utils::DSSReturnType
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
    auto exec_all_tasks(bool recursive) -> utils::DSSDelegateReturnType
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
        
        m_tasks.clear(); // All tasks are completed, whether successfully or not
        m_busy = false;
        if (recursive == true)
        {
            exec_all_tasks(key::FLAG_RECURSIVE_EXECUTION);
        }

        return res;
    }
public:
    /**
     * Produces a DSS executor.
     * 
     * @param id The unique RunID of this particular executor.
     * @param additional_preprocessors Any additional preprocessor definers
     * @param additional_commands Any additional command definers
     */
    Executor(RunID id, DefinerDelegate additional_preprocessors = DefinerDelegate(), DefinerDelegate additional_commands = DefinerDelegate())
    {
        m_loaded_commands = {};
        m_additional_preprocessors = additional_preprocessors;
        m_additional_commands = additional_commands;
        m_tasks = {};

        m_id = id;
    }

    /**
     * Directly defines a DSS command, whether it be a preprocessor
     * or an ordinary command. Definer functions exist with the sole
     * purpose of using this particular method.
     */
    void define_command(
        utils::DSSFunc func,
        std::string name,
        std::string description,
        int minimum_args = -1,
        int maximum_args = -1
    )
    {
        if (this == nullptr) {return;}

        Command res = Command(func, name, description, minimum_args, maximum_args, this);
        m_loaded_commands.push_back(res);
    }

    /**
     * Execute a DSS script. This is the
     * intended solution for giving a DSS environment
     * execution tasks.
     * 
     * When given input, the environment will create a task, then execute the task
     * after it is finished with all previous (older) tasks.
     * 
     * @param script A string containing DSS script to execute
     */
    void exec(std::string script)
    {
        if (this == nullptr) {return;}

        Task task = Task(script);
        m_tasks.push_back(task); // Append a task to the task list
        exec_all_tasks(key::FLAG_RECURSIVE_EXECUTION); // Invoke the executor
    }

    RunID get_id()
    {
        return m_id;
    }
};

class Environment
{
private:
    RunID m_id_max;
    std::vector<Executor> m_executors;
    DefinerDelegate m_additional_preprocessors;
    DefinerDelegate m_additional_commands;

public:
    Environment()
    {
        m_id_max = 0;
    }

    /**
     * Connect a preprocessor definer to the environment. Crucial
     * if you intend to graft preprocessors onto DSS.
     * 
     * @param func A pointer to the definer function
     * 
     * @see Executor::connect_command_definer
     */
    void connect_preprocessor_definer(const Definer func)
    {
        if (this == nullptr) {return;}
        if (func == nullptr) {return;}
        m_additional_preprocessors.connect(func);
    }

    /**
     * Connect a command definer to the environment. Crucial
     * if you intend to graft commands onto DSS.
     * 
     * @param fnuc A pointer to the definer function
     * 
     * @see Executor::connect_preprocessor_definer
     */
    void connect_command_definer(const Definer func)
    {
        if (this == nullptr) {return;}
        m_additional_commands.connect(func);
    }

    void spawn_executor()
    {
        Executor new_executor = Executor(unique_runid(), m_additional_preprocessors, m_additional_commands);
        m_executors.push_back(new_executor);
    }

    void init()
    {
        spawn_executor();
    }

    auto main_executor() -> std::optional<Executor>
    {
        if (m_executors.size() == 0)
        {
            return std::nullopt;
        }

        return m_executors[0];
    }

    auto unique_runid() -> RunID
    {
        return m_id_max;
        m_id_max++;
    }

    auto executor_by_id(RunID id) -> std::optional<Executor>
    {
        for (auto executor : m_executors)
        {
            if (executor.get_id() != id) {continue;}

            return executor;
        }

        return std::nullopt;
    }
};

#endif
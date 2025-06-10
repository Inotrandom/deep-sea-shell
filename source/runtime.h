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

class Executor;

typedef int64_t RunID;

/**
 * Arguments to functions connected to delegates
 */
typedef std::vector<std::string> DSSFuncArgs;

typedef std::vector<std::string> StrVec;

/**
 * Return type of functions connected to delegates
 */
typedef std::size_t DSSReturnType;

/**
 * Typing of function pointers that should be
 * passed onto delegates
 */
typedef DSSReturnType (*DSSFunc)(Executor*, DSSFuncArgs);

/**
 * Return type of Delegate::call and any other
 * functions that pass this result down the 
 * pipeline
 */
typedef std::vector<DSSReturnType> DSSDelegateReturnType;

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
    Executor* m_parent_ex;

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
        Executor* parent_ex = nullptr
    )
    {
        //m_delegate = Delegate<DSSFunc, DSSFuncArgs, DSSDelegateReturnType>();
        m_name = name;
        m_delegate.connect(func);
        m_description = description;
        m_minimum_args = minimum_args;
        m_maximum_args = maximum_args;
        m_parent_ex = parent_ex;
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
    auto attempt_parse_and_exec(std::string inp, std::string delim) -> DSSDelegateReturnType;
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
    /**
     * The physical DSS script inside
     * the task
     */
    std::string m_script;
public:
    /**
     * Constructs a task.
     * 
     * @param script The script contents of the task
     */
    Task(const std::string script)
    {
        m_script = script;
    }

    /** 
     * @return A reference to the script of the task.
     * Mutability is intentional.
     */
    auto get_script() -> std::string&
    {
        return m_script;
    }
};

typedef std::any (*Definer)(Executor*);
typedef utils::Delegate<Definer, Executor*, std::vector<std::any>> DefinerDelegate;

template<typename T>
class Var
{
private:
    /**
     * The id of the variable.
     */
    std::string m_id;

    /**
     * The data of the variable
     */
    std::vector<T> m_data;
public:
    /**
     * Constructs an environment variable.
     * 
     * @param id The id of the variable.
     * Note that variables will fail to
     * initialize unless they have different
     * ids.
     * 
     * @param data Data to initialize the variable
     * with.
     */
    Var(std::string id, std::vector<T> data)
    {
        m_id = id;
        m_data = data;
    }

    /**
     * @return A reference to the 
     */
    auto get_id() -> std::string&
    {
        return m_id;
    }

    /**
     * @return A reference to a vector 
     * containing the data of the variable.
     */
    auto get_data() -> std::vector<T>&
    {
        return m_data;
    }

    /**
     * @tparam A the type of the variable.
     * This is assumed to be the same as the
     * type of the variable, or else this
     * will throw an ICE!
     */
    template<typename A>
    void append_data(A what)
    {
        bool found = false;
        for (auto element : m_data)
        {
            try
            {
                A comp = std::any_cast<A>(element);
                if (comp != what)
                {
                    continue;
                }
            } catch (std::bad_any_cast &_e) {return;}

            found = true;
        }

        if (found == true) {return;}

        m_data.push_back(what);
        
        return;
    }
};

const std::string AUTO_PREPROCESSOR_VAR = "auto_preprocessor";

class Vars
{
private:
    /**
     * A vector of generic (`std::any`) environment variables
     */
    std::vector<Var<std::any>> m_vars;

public:
    /**
     * Creates an environment variable
     * 
     * @tparam T the type of the data for the variable.
     * IDs are always std::strings
     * 
     * @param id The identifier of the variable.
     * 
     * @param data The data to initialize the variable with.
     * 
     * @see Var
     */
    void init_var(std::string id, std::vector<std::any> data)
    {
        if (has_var(id) == true) {return;}

        Var new_var = Var<std::any>(id, data);

        m_vars.push_back(new_var);
    }

    /**
     * Attempts to retrieve a variable lazily.
     * 
     * @return A pointer to a generic (`std::any`) variable.
     * Will be `nullptr` if the variable is not found!
     */
    auto get_var(std::string id) -> Var<std::any>*
    {
        size_t index = 0;
        for (auto var : m_vars)
        {
            if (var.get_id() != id) {index++; continue;}

            return &(m_vars[index]);
            index++;
        }

        return nullptr;
    }

    /**
     * Attempts to retrieve a variable lazily, and
     * will initialize the variable if the retrieval fails.
     * 
     * @return A pointer to the variable. This should
     * be safe to rely on.
     */
    auto get_or_add_var(std::string id) -> Var<std::any>*
    {
        Var<std::any> *p_res = get_var(id);

        if (p_res == nullptr)
        {
            init_var(id, {});
            return get_var(id);
        }

        return p_res;
    }

    /**
     * Returns a bool describing whether the variable
     * of id `id` exists (it will find this lazily).
     * 
     * @param id The id to compare against variables
     */
    auto has_var(std::string id) -> bool
    {
        if (get_var(id) != nullptr) {return true;}
        return false;
    }
};

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
     * Data stored in the executor
     */
    Vars m_exec_vars;

    /**
     * Whether or not the environment is actively
     * completing tasks.
     */
    bool m_busy;

    /**
     * The unique id of the executor. This is to
     * differentiate them within the `Environment`
     * hierarchy.
     */
    RunID m_id;

    /**
     * A pointer to the task that is currently being
     * processed. This will be null if there are no
     * tasks (duh).
     */
    Task *m_current_task;

    /**
     * Directly executes a script.
     * 
     * This function is intended exclusively
     * for the internals of the interpreter.
     * 
     * @see Executor::exec_task
     */
    void direct_exec(StrVec statements)
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
     * @brief Applies automatic preprocessors.
     * 
     * Automatic preprocessors run even if they
     * aren't explicitly called within the script
     * body. They are handled after the "preprocessor"
     * pass finishes, but before the "command" pass 
     * starts.
     * 
     * For example: `alias`
     */
    void auto_preprocessors()
    {
        Var<std::any> *auto_preprocessor_var = m_exec_vars.get_or_add_var(AUTO_PREPROCESSOR_VAR);
        if (auto_preprocessor_var == nullptr) {return;} // Impossible

        std::vector<std::string> statements = {};
        for (auto element : auto_preprocessor_var->get_data())
        {
            statements.push_back( std::any_cast<std::string>(element) );
        }

        direct_exec(statements);
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
    void command_pass(DefinerDelegate definer, StrVec statements)
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

        m_current_task = &task;

        std::string &script = task.get_script();
        StrVec statements = string_split(script, key::MULTILINE_DELIM);

        command_pass(m_additional_preprocessors, statements);
        auto_preprocessors();

        statements = string_split(script, key::MULTILINE_DELIM); // Update after the preprocessors are finished

        command_pass(m_additional_commands, statements);

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
        m_current_task = nullptr;

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

    /**
     * @return A clone of the executor's RunID
     */
    auto get_id() -> RunID
    {
        return m_id;
    }

    /**
     * @return A reference to this executor's environment `Vars`.
     */
    auto get_vars() -> Vars&
    {
        return m_exec_vars;
    }

    /**
     * @return A pointer to the currently procesing task.
     * This may be null!
     */
    auto get_current_task() -> Task*
    {
        return m_current_task;
    }
};

/**
 * Activates a pipeline which parses and executes
 * a statement.
 * 
 * @param inp The statement to parse
 * 
 * @param delim The delimiter to separate tokens
 */
auto Command::attempt_parse_and_exec(std::string inp, std::string delim) -> DSSDelegateReturnType
{
    DSSDelegateReturnType res = {};
    
    if (this == nullptr) {return res;}

    std::vector<std::string> tokens = string_split(inp, delim);
    
    if (tokens[0] != m_name) {return res;}

    tokens.erase(tokens.begin());

    std::size_t arg_count = tokens.size();

    if (m_parent_ex == nullptr) {return res;}

    res = m_delegate.call(m_parent_ex, tokens);
    return res;
}

/**
 * A DSS environment. This contains
 * multiple executors
 */
class Environment
{
private:
    /**
     * The maximum executor ID.
     * This is incremented every time an executor
     * is created in order to ensure executors have
     * unique ids.
     */
    RunID m_id_max;

    /**
     * A vector of every executor that is
     * currently alive in this environment.
     */
    std::vector<Executor> m_executors;

    /**
     * Definer delegate containing additional preprocessor definers.
     */
    DefinerDelegate m_additional_preprocessors;

    /**
     * Definer delegate containing additional command definers.
     */
    DefinerDelegate m_additional_commands;

    /**
     * Generates a unique RunID. This is
     * useful when spawning executors.
     */
    auto unique_runid() -> RunID
    {
        return m_id_max;
        m_id_max++;
    }

    /**
     * Lazily retrieves the executor with RunID `id`.
     * 
     * @param id The id of the executor to attempt and find
     * 
     * @return An optional containing the executor, if it
     * exists.
     */
    auto executor_by_id(RunID id) -> std::optional<Executor>
    {
        for (auto executor : m_executors)
        {
            if (executor.get_id() != id) {continue;}

            return executor;
        }

        return std::nullopt;
    }

public:
    /**
     * Constructs an environment with a default
     * maximum ID of zero (root id).
     */
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

    /**
     * Spawns an executor, gives it a unique RunID, and appends it to `m_excutors`
     */
    void spawn_executor()
    {
        Executor new_executor = Executor(unique_runid(), m_additional_preprocessors, m_additional_commands);
        m_executors.push_back(new_executor);
    }

    /**
     * Initialize the environment and run default code to make the
     * environment function. This method serves as an exemplary starting
     * point for working with DSS.
     */
    void init()
    {
        spawn_executor();
    }

    /**
     * Retrieves executor `0` (RunID), also known as the root ("main") executor.
     * 
     * @return an optional containing the main executor. 
     * This should have a value if the environment has been initialized properly.
     */
    auto main_executor() -> std::optional<Executor>
    {
        if (m_executors.size() == 0)
        {
            return std::nullopt;
        }

        return m_executors[0];
    }
};

#endif
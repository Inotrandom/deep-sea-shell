#ifndef H_RUNTIME
#define H_RUNTIME

#include <any>
#include <memory>
#include <map>

#include "dss_utils.h"

namespace DSS
{

void push_error(std::string what, int line = -1);

class executor_t;

typedef int64_t run_id_t;

typedef std::map<uint, std::string> err_codes_t;
typedef std::map<std::string, err_codes_t> err_key_t;

/**
 * Arguments to functions connected to delegates
 */
typedef std::vector<std::string> func_args_t;

typedef std::vector<std::string> strvec_t;

/**
 * Return type of functions connected to delegates
 */
typedef std::size_t return_type_t;

/**
 * Typing of function pointers that should be
 * passed onto delegates
 */
typedef return_type_t (*func_t)(executor_t *, func_args_t);

/**
 * Return type of Delegate::call and any other
 * functions that pass this result down the
 * pipeline
 */
typedef std::vector<return_type_t> delegate_return_t;

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
class command_t
{
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
	command_t(
		func_t func, std::string name, std::string description, std::int64_t minimum_args = -1, std::int64_t maximum_args = -1, executor_t *parent_ex = nullptr)
	{
		// m_delegate = Delegate<DSSFunc, DSSFuncArgs, DSSDelegateReturnType>();
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
	auto attempt_parse_and_exec(strvec_t tokens, std::uint64_t line) -> delegate_return_t;

private:
	dss_utils::Delegate<func_t, func_args_t, delegate_return_t> m_delegate = {32};
	std::string m_name = {""};
	std::string m_description = {""};
	int64_t m_minimum_args = {-1};
	int64_t m_maximum_args = {-1};
	executor_t *m_parent_ex = {nullptr};
};

namespace err
{
const std::string NOT_A_COMMAND = "command does not exist or is not defined";
const std::string UNKNOWN = "an unnamed critical exception occurred";
} // namespace err

namespace key
{
const std::string MULTILINE_DELIM = "\n";
const std::string TOKEN_DELIM = " ";

const std::string COMMENT_ID = "//";

const bool FLAG_RECURSIVE_EXECUTION = true;
} // namespace key

/**
 * DSSTasks are scripts that require running
 *
 * They may have metadata in the future
 */
class task_t
{
public:
	/**
	 * Constructs a task.
	 *
	 * @param script The script contents of the task
	 */
	task_t(const std::string script) { m_script = script; }

	/**
	 * @return A reference to the script of the task.
	 * Mutability is intentional.
	 */
	auto get_script() -> std::string & { return m_script; }

private:
	/**
	 * The physical DSS script inside
	 * the task
	 */
	std::string m_script = {""};
};

typedef std::any (*definer_t)(executor_t *);
typedef dss_utils::Delegate<definer_t, executor_t *, std::vector<std::any>> definer_delegate_t;

template <typename T> class var_t
{
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
	var_t(std::string id, std::vector<T> data)
	{
		m_id = id;
		m_data = data;
	}

	/**
	 * @return A reference to the
	 */
	auto get_id() -> std::string & { return m_id; }

	/**
	 * @return A reference to a vector
	 * containing the data of the variable.
	 */
	auto get_data() -> std::vector<T> & { return m_data; }

	/**
	 * @tparam A the type of the variable.
	 * This is assumed to be the same as the
	 * type of the variable, or else this
	 * will throw an ICE!
	 */
	template <typename A> void append_data(A what)
	{
		bool found = false;
		A *found_element;
		for (auto element : m_data)
		{
			try
			{
				A comp = std::any_cast<A>(element);
				if (comp != what)
				{
					continue;
				}
				found_element = &comp;
				found = true;
			}
			catch (std::bad_any_cast &_e)
			{
				return;
			}
		}

		if (found == true)
		{
			return;
		}

		m_data.push_back(what);

		return;
	}

private:
	/**
	 * The id of the variable.
	 */
	std::string m_id;

	/**
	 * The data of the variable
	 */
	std::vector<T> m_data;
};

const std::string AUTO_PREPROCESSOR_VAR = "auto_preprocessor";

class vars_t
{
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
		if (has_var(id) == true)
		{
			return;
		}

		std::shared_ptr<var_t<std::any>> new_var = std::make_shared<var_t<std::any>>(id, data);

		m_vars.push_back(new_var);
	}

	/**
	 * Attempts to retrieve a variable lazily.
	 *
	 * @return A pointer to a generic (`std::any`) variable.
	 * Will be `nullptr` if the variable is not found!
	 */
	auto get_var(std::string id) -> std::shared_ptr<var_t<std::any>>
	{
		/*
		size_t index = 0;
		std::vector<std::shared_ptr<Var<std::any>>>::iterator itr;
		for (itr = m_vars.begin(); itr != m_vars.end(); itr++)
		{
			if (itr->get_id() != id) {index++; continue;}

			return &(*itr);
			index++;
		}
		*/

		for (auto var : m_vars)
		{
			if (var->get_id() != id)
				continue;

			return var;
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
	auto get_or_add_var(std::string id) -> std::shared_ptr<var_t<std::any>>
	{
		std::shared_ptr<var_t<std::any>> p_res = get_var(id);

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
		if (get_var(id) != nullptr)
		{
			return true;
		}
		return false;
	}

private:
	/**
	 * A vector of generic (`std::any`) environment variables
	 */
	std::vector<std::shared_ptr<var_t<std::any>>> m_vars;
};

/**
 * DSS execution environment. Accepts and executes tasks.
 */
class executor_t
{
public:
	/**
	 * Produces a DSS executor.
	 *
	 * @param id The unique RunID of this particular executor.
	 * @param additional_preprocessors Any additional preprocessor definers
	 * @param additional_commands Any additional command definers
	 */
	executor_t(run_id_t id, definer_delegate_t additional_preprocessors = definer_delegate_t(), definer_delegate_t additional_commands = definer_delegate_t(),
		err_key_t lookup_error = err_key_t())
	{
		m_loaded_commands = {};
		m_additional_preprocessors = additional_preprocessors;
		m_additional_commands = additional_commands;
		m_tasks = {};
		m_current_task = nullptr;
		m_lookup_error = lookup_error;

		m_id = id;
	}

	/**
	 * Directly defines a DSS command, whether it be a preprocessor
	 * or an ordinary command. Definer functions exist with the sole
	 * purpose of using this particular method.
	 */
	void define_command(DSS::func_t func, std::string name, std::string description, int minimum_args = -1, int maximum_args = -1)
	{

		command_t res = command_t(func, name, description, minimum_args, maximum_args, this);
		m_loaded_commands.push_back(res);
	}

	/**
	 * Queues a task while the executor is busy.
	 */
	void queue_task(task_t task) { m_task_buffer.push_back(task); }

	/**
	 * Execute a DSS script. This is the
	 * intended solution for beginning task execution
	 *
	 * When given input, the environment will create a task, then execute the task
	 * after it is finished with all previous (older) tasks.
	 *
	 * @param script A string containing DSS script to execute
	 */
	void exec(std::string script);

	/**
	 * @return A clone of the executor's RunID
	 */
	auto get_id() -> run_id_t { return m_id; }

	/**
	 * @return A reference to this executor's environment `Vars`.
	 */
	auto get_vars() -> vars_t & { return m_exec_vars; }

	/**
	 * @return A pointer to the currently procesing task.
	 * This may be null!
	 */
	auto get_current_task() -> task_t * { return m_current_task; }

private:
	/**
	 * Every command currently working for this
	 * particular "pass" of execution
	 */
	std::vector<command_t> m_loaded_commands;

	/**
	 * Any user-defined preprocessor definers
	 */
	definer_delegate_t m_additional_preprocessors;

	/**
	 * Any user-defined command definers
	 */
	definer_delegate_t m_additional_commands;

	/**
	 * Every single task that this environment
	 * has yet to complete or is in the process
	 * of completing
	 */
	std::vector<task_t> m_tasks;

	std::vector<task_t> m_task_buffer;

	/**
	 * Data stored in the executor
	 */
	vars_t m_exec_vars;

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
	run_id_t m_id;

	/**
	 * A pointer to the task that is currently being
	 * processed. This will be null if there are no
	 * tasks (duh).
	 */
	task_t *m_current_task;

	/**
	 * The error keys for every defined command
	 */
	err_key_t m_lookup_error;

	void find_and_push_error(std::string command, int code, int line = -1);

	/**
	 * Directly executes a script.
	 *
	 * This function is intended exclusively
	 * for the internals of the interpreter.
	 *
	 * @see Executor::exec_task
	 */
	void direct_exec(strvec_t statements);

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
	void auto_preprocessors();

	/**
	 * Command passes describe one singular execution of the program.
	 *
	 * @param definer The function pointer that determines which commands
	 * are active during the "pass"
	 *
	 * @param statements A vector of individual command strings. These will
	 * be further split into individual tokens by the command itself.
	 */
	void command_pass(definer_delegate_t definer, strvec_t statements);

	/**
	 * Consider this the actual executor- this will
	 * commence the parsing and execution process
	 * of commands.
	 *
	 * @param task The task to be completed
	 *
	 * @return The result of executing the task.
	 */
	auto exec_task(task_t task) -> DSS::return_type_t;

	/**
	 * Executes every single task in the queue.
	 *
	 * @param recursive Determines whether or not DSS will continue executing
	 * new tasks as they arrive.
	 *
	 * @return A vector containing the result of
	 * execution for every single task.
	 */
	auto exec_all_tasks(bool recursive) -> DSS::delegate_return_t;
};

/**
 * A DSS environment. This contains
 * multiple executors
 */
class environment_t
{
public:
	/**
	 * Constructs an environment with a default
	 * maximum ID of zero (root id).
	 */
	environment_t()
	{
		m_id_max = 0;
		m_additional_commands = definer_delegate_t(32);
		m_additional_preprocessors = definer_delegate_t(32);
	}

	void apply_error_key(err_key_t key);

	/**
	 * Connect a preprocessor definer to the environment. Crucial
	 * if you intend to graft preprocessors onto DSS.
	 *
	 * @param func A pointer to the definer function
	 *
	 * @see Executor::connect_command_definer
	 */
	void connect_preprocessor_definer(const definer_t func) { m_additional_preprocessors.connect(func); }

	/**
	 * Connect a command definer to the environment. Crucial
	 * if you intend to graft commands onto DSS.
	 *
	 * @param func A pointer to the definer function
	 *
	 * @see Executor::connect_preprocessor_definer
	 */
	void connect_command_definer(const definer_t func) { m_additional_commands.connect(func); }

	/**
	 * Spawns an executor, gives it a unique RunID, and appends it to `m_excutors`
	 */
	void spawn_executor()
	{
		std::shared_ptr<executor_t> new_executor =
			std::make_shared<executor_t>(unique_runid(), m_additional_preprocessors, m_additional_commands, m_lookup_error);
		m_executors.emplace_back(new_executor);
	}

	/**
	 * Initialize the environment and run default code to make the
	 * environment function. This method serves as an exemplary starting
	 * point for working with DSS.
	 */
	void init();

	/**
	 * Retrieves executor `0` (RunID), also known as the root ("main") executor.
	 *
	 * @return an optional containing the main executor.
	 * This should have a value if the environment has been initialized properly.
	 */
	auto main_executor() -> std::shared_ptr<executor_t>
	{
		if (m_executors.size() == 0)
		{
			return nullptr;
		}

		return m_executors[0];
	}

private:
	/**
	 * The maximum executor ID.
	 * This is incremented every time an executor
	 * is created in order to ensure executors have
	 * unique ids.
	 */
	run_id_t m_id_max;

	/**
	 * A vector of every executor that is
	 * currently alive in this environment.
	 */
	std::vector<std::shared_ptr<executor_t>> m_executors;

	/**
	 * Definer delegate containing additional preprocessor definers.
	 */
	definer_delegate_t m_additional_preprocessors;

	/**
	 * Definer delegate containing additional command definers.
	 */
	definer_delegate_t m_additional_commands;

	err_key_t m_lookup_error;

	/**
	 * Generates a unique RunID. This is
	 * useful when spawning executors.
	 */
	auto unique_runid() -> run_id_t
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
	auto executor_by_id(run_id_t id) -> std::shared_ptr<executor_t>;
};

}; // namespace DSS

#endif
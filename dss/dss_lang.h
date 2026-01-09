/**
 * This file contains core language functionality
 * charactaristic of ordinary DSS.
 *
 * Every distribution of DSS should have these
 * commands, as they are fundamental language
 * features.
 */

#ifndef H_LANG
#define H_LANG

#include <iostream>
#include <filesystem>

#include "runtime.h"
#include "utils.h"

namespace lang
{

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
struct alias_t
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

	bool operator<=>(const alias_t &) const = default;
};
typedef std::vector<alias_t> AliasVarData;

/**
 * Applies `alias` as an automatic preprocessor.
 * If no aliases are defined in the executor, then
 * this function will never get called. Consequently,
 * the `alias` preprocessor will never become automatic.
 */
inline void use_alias(DSS::executor_t *p_ex)
{
	DSS::vars_t &vars = p_ex->get_vars();
	std::shared_ptr<DSS::var_t<std::any>> auto_preproc_var = vars.get_or_add_var(DSS::AUTO_PREPROCESSOR_VAR);

	if (auto_preproc_var == nullptr)
	{
		return;
	}

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
inline auto create_alias(DSS::executor_t *p_ex, std::string id, std::string data) -> DSS::return_type_t
{
	DSS::vars_t &vars = p_ex->get_vars();
	std::shared_ptr<DSS::var_t<std::any>> alias_var = vars.get_or_add_var(ALIAS_VAR);

	if (alias_var == nullptr)
	{
		return 1;
	}

	alias_t new_alias = alias_t();
	new_alias.id = id;
	new_alias.value = data;

	// Append it or replace it, but don't drop it
	for (auto &element : alias_var->get_data())
	{
		try
		{
			lang::alias_t comp = std::any_cast<lang::alias_t>(element);
			if (comp.id != id)
			{
				continue;
			}

			// Successful replace
			element = new_alias;
			return 0;
		}
		catch (std::bad_any_cast &_e)
		{
			return 0;
		}
	}

	alias_var->append_data(new_alias);

	return 0;
}

const std::string NAME = "lang";

namespace func
{
/**
 * Out will push arguments into the stdout stream.
 */
inline auto out(DSS::executor_t *p_ex, DSS::func_args_t args) -> DSS::return_type_t
{
	(void)p_ex;

	for (auto argument : args)
	{
		std::cout << argument;
		std::cout << " ";
	}

	std::cout << std::endl;

	return 0;
}

inline auto ls(DSS::executor_t *p_ex, DSS::func_args_t args) -> DSS::return_type_t
{
	(void)p_ex;
	(void)args;

	for (const auto &dir_entry : std::filesystem::directory_iterator("."))
	{
		std::string name = dir_entry.path().generic_string();
		std::cout << name << std::endl;
	}

	return 0;
}

/**
 * @brief This will set the filesystem.current_directory using argument 1
 */
inline auto curdir(DSS::executor_t *p_ex, DSS::func_args_t args) -> DSS::return_type_t
{
	(void)p_ex;

	if (std::filesystem::exists(args[0]) == false)
		return 1;
	std::filesystem::current_path(args[0]);

	return 0;
}

/**
 * Alias Define will define an alias and call `use_alias`
 */
inline auto alias_def(DSS::executor_t *p_ex, DSS::func_args_t args) -> DSS::return_type_t
{
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
 * @brief Will not check for bad casts
 *
 * @param a The first alias
 * @param b The second alias
 * @return true a.id.length() > b.id.length()
 * @return false a.id.length() < b.id.length()
 */
auto is_any_alias_id_greater(const std::any &a, const std::any &b) -> bool
{
	const alias_t &aa = std::any_cast<alias_t>(a);
	const alias_t &ab = std::any_cast<alias_t>(b);

	return (aa.id.length() > ab.id.length());
}

/**
 * Alias will apply defined aliases throughout
 * the script lazily.
 */
inline auto alias(DSS::executor_t *p_ex, DSS::func_args_t args) -> DSS::return_type_t
{
	(void)args;

	DSS::task_t *p_current_task = p_ex->get_current_task();
	if (p_current_task == nullptr)
	{
		return 1;
	}

	DSS::vars_t &vars = p_ex->get_vars();
	std::shared_ptr<DSS::var_t<std::any>> alias_var = vars.get_var(ALIAS_VAR);
	if (alias_var == nullptr)
	{
		return 1;
	}

	std::string &script = p_current_task->get_script();

	std::sort(alias_var->get_data().begin(), alias_var->get_data().end(), is_any_alias_id_greater);

	for (auto element : alias_var->get_data())
	{
		alias_t alias = std::any_cast<alias_t>(element);

		std::string deref = ALIAS_DEREF + alias.id;
		utils::string_replace(script, deref, alias.value);
	}

	return 0;
}

inline auto source(DSS::executor_t *p_ex, DSS::func_args_t args) -> DSS::return_type_t
{
	std::optional<std::string> res = utils::file_read(args[0]);

	// std::cout << std::filesystem::current_path();
	// std::cout << res.has_value();

	if (res.has_value() == false)
	{
		return 2;
	}

	DSS::task_t task = DSS::task_t(res.value());
	p_ex->queue_task(task);

	return 0;
}
} // namespace func

/**
 * Definer for `lang` preprocessors
 */
inline std::any preprocessor_definer(DSS::executor_t *exec)
{
	exec->define_command(func::source, "src", "runs a dss script at path <path>", 1, 1);

	exec->define_command(func::alias_def, "alias_def", "creates an alias", 2);

	exec->define_command(func::alias, "alias", "applies aliases");

	return nullptr;
}

/**
 * Definer for `command` preprocessors
 */
inline std::any command_definer(DSS::executor_t *exec)
{
	exec->define_command(func::out, "out", "outputs to console", 1);

	exec->define_command(func::curdir, "cd", "changes the current directory", 1, 1);

	exec->define_command(func::ls, "ls", "lists all files and directories in the current directory (.)", 0, 0);

	return nullptr;
}

const std::string NULL_ENVIRONMENT =
	"internal interpreter error, critical data unexpectedly returned null.\n\nnote: this error requires the attention of a developer";
const DSS::err_codes_t OUT = {{1, NULL_ENVIRONMENT}};

const DSS::err_codes_t SRC = {{1, NULL_ENVIRONMENT}, {2, "failed to queue script, file does not exist"}};

const DSS::err_codes_t ALIAS_DEF = {{1, NULL_ENVIRONMENT}};

const DSS::err_codes_t ALIAS = {
	{1, "internal interpreter error, automatic command failure, critical data unexpectedly returned null. \n\nhelp: did you mean \"alias_def\"?"}};

const DSS::err_codes_t CURDIR = {{1, "file does not exist"}};

const DSS::err_key_t ERR_KEY = {{"out", OUT}, {"src", SRC}, {"alias_def", ALIAS_DEF}, {"alias", ALIAS}, {"cd", CURDIR}};

}; // namespace lang

#endif // H_LANG
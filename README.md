# Deep Sea Shell

## Brief

DSS is a high-level data-oriented operative scripting language. Among others,
its primary purpose is for communication and control of robotic systems. Quite literally,
it is a parsing utility for more-complicated command line interfaces.

Unlike JSon or TOML, it serves as a directly executable format.

DSS is designed not just for human users, but also for systems to universally
send/recieve data packets on a network.

Grafting to DSS is a core language feature; producing your own organized commands
and namespaces is important to ensure the effectiveness of the language for your
specific needs.

## Usage

Default DSS currently has only three (technically four) built-in functions:

`alias_def <id> <value>`
`out <msg...>`
`src <path to script>`
`cd <path>`

These are defined in the `dss_lang.h` file

### Example

Demonstrated below is a program that successfully invokes DSS:

```cpp
#include "DSS.h"

int main()
{
	DSS::environment_t env = DSS::environment_t();

	env.init();
	std::shared_ptr<DSS::executor_t> main_ex = env.main_executor();

	if (main_ex == nullptr)
	{
		return 1;
	}

	DSS::cli_t cli = DSS::cli_t(main_ex);
	cli.init(); // TODO: Execute "src example.dss" in the console to see DSS in action

	return 0;
}
```

**See the example.dss program in the repository for basic DSS syntax**

Important notes:

* Environment::init() should be called *after* any Environment::connect_preprocessor_definer or Environment::connect_command_definer calls, not before. If you intend to create executors manually, it should be done after you have connected all of your command definers.
* Default DSS Lang features are grafted automatically upon the calling of Environment::init()

### Grafting

A "Command" is an object that (put briefly) contains a function pointer, keyword (name) and brief manual.
When defining custom functionality for DSS, you must create a definer function:

```cpp
namespace example
{
static std::any command_definer(runtime::Executor *exec)
{
    if (exec == nullptr) {return NULL;}

    exec->define_command(
        // function pointer,
        // keyword string literal,
        // manual string literal
    );

    return NULL;
}
};
```

After the definer has been declared, it must now be connected to the environment:

```cpp
env.connect_command_definer(example::command_definer);
```

*(Remember: it is imperative that env.init() is not called before env.connect_command_definer())*

## More Information

It is highly recommended to read the source code of `dss_lang.h` for documented examples on the usage of DSS's grafting feature.

Building and running the program will result in the example (shown above in the "Example" section) being run.
This will open an instance of the command line interface and allow the user to directly execute Deep Sea Shell.

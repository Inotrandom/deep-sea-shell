# Deep Sea Shell

## Brief

DSS is a high-level data-oriented operative programming language. Among others,
its primary purpose is for communication and control of robotic systems.
DSS is intended to exist as both an effective means of storing, parsing, and
using data, but also as a simple tool for operating systems.

DSS is designed not just for human users, but also for systems to universally
send/recieve data packets on a network.

Grafting to DSS is a core language feature; producing your own organized commands
and namespaces is important to ensure the effectiveness of the language for your
specific needs.

## Usage

Default DSS currently has only three (technically four) built-in functions:

`alias_def <id> <value>`
`out <msg...>`
`src <relative path to script>`

These are defined in the `lang.h` file

### Example

Demonstrated below is a program that successfully invokes DSS:

```cpp
#include <iostream>
#include "DSS.h"

int main(int argv, char** argc)
{
    DSS::Environment env = DSS::Environment();

    env.init();
    std::optional<DSS::Executor> opt_main_executor = env.main_executor();

    if (opt_main_executor.has_value() == false)
    {
        return 1;
    }

    DSS::Executor main_executor = opt_main_executor.value();

    DSS::CLI cli = DSS::CLI(&main_executor);
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

It is highly recommended to read the source code of `lang.h` for documented examples on the usage of DSS's grafting feature.

Building and running the program will result in the example (shown above in the "Example" section) being run.
This will open an instance of the command line interface and allow the user to directly execute Deep Sea Shell.
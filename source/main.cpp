/**
 * DSS is a high-level data-oriented operative programming language. Among others,
 * its primary purpose is for communication and control of robotic systems.
 * DSS is intended to exist as both an effective means of storing, parsing, and
 * using data, but also as a simple tool for operating systems.
 * 
 * DSS is designed not just for human users, but also for systems to universally
 * send/recieve data packets on a network.
 * 
 * Grafting to DSS is a core language feature; producing your own organized commands
 * and namespaces is important to ensure the effectiveness of the language for your
 * specific needs.
 */

#include <iostream>
#include "runtime.h"
#include "lang.h"

int main(int argv, char** argc)
{
    runtime::Environment env = runtime::Environment();
    env.connect_command_definer(lang::definer);

    env.exec(
        "out Hello, world!"
        "\nout That's not nice..."
    );

    return 0;
}
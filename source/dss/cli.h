#include <string>

#include "runtime.h"

#ifndef CLI_H
#define CLI_H

namespace DSS {

const std::string DEFAULT_CLI_NAME = "dss";

class CLI
{
private:
    DSS::Executor *m_bound_executor;
    bool m_alive;
    std::string m_name;
public:

    CLI(DSS::Executor *bound_executor, std::string name)
    {
        if (bound_executor == nullptr) {return;}
        m_bound_executor = bound_executor;
        m_alive = true;

        m_name = name;
    }

    CLI(DSS::Executor *bound_executor)
    {
        if (bound_executor == nullptr) {return;}
        m_bound_executor = bound_executor;
        m_alive = true;

        m_name = DEFAULT_CLI_NAME;
    }

    void init();

    void execute(std::string what);

    void input_loop();
};

}

#endif
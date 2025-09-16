#include "runtime.h"
#include <string>

#ifndef CLI_H
#define CLI_H

namespace DSS {

class CLI
{
private:
    DSS::Executor *m_bound_executor;
    bool m_alive;
public:
    CLI(DSS::Executor *bound_executor)
    {
        if (bound_executor == nullptr) {return;}
        m_bound_executor = bound_executor;
        m_alive = true;
    }

    void init();

    void execute(std::string what);

    void input_loop();
};

}

#endif
#ifndef H_INIT
#define H_INIT

#include <string>
#include <sstream>

#include "version.h"

inline auto init_script() -> std::string
{
	std::stringstream script;
	script << "alias_def __VERSION__" << str_VERSION;

	return script.str();
}

#endif
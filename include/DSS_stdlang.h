#ifndef DSS_STDLANG_H
#define DSS_STDLANG_H

#include <stdio.h>

#include "DSS_mem.h"
#include "DSS_env.h"

int DSS_stdlang_out(DSS_mem_t *mem, int argc, char **argv)
{
	(void)mem;

	for (char **iter = argv; iter < (argv + argc); ++iter)
	{
		printf("%s ", *iter);
	}
	printf("\n");

	return 0;
}

void DSS_stdlang_definer(DSS_env_t *env) { DSS_env_define_command(env, "out", DSS_stdlang_out); }

#endif // DSS_STDLANG_H

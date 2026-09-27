# Deep Sea Shell

> [!NOTE]
> Expect changes as features are added!

## Overview
Deep Sea Shell (DSS) is a high-level data-oriented procedural scripting language. It is suited for tasks that are too complicated for simple parsing, but too simple for a configuration language like Lua.
Most notably, it excels at networking packets and command-line interfaces.

## Design
The language is intentionally not Turing complete, even if its suite of tools would handle it fine. This is to avoid the pitfalls that Lua fell into.
Unlike JSON or YAML, Deep Sea Shell is procedural and "live." When commands are parsed, they are dumped straight into function pointers. This minimal overhead also makes the language hyper-optimized for scripting.

## v2.x
* C

The new Deep Sea Shell is written in pure C, making it significantly simpler, less-bloated, and portable.

* Simplicity

By retiring features such as executors, ameliorating the complicated definer-delegate memory structure, and shortening the standard commands,
the interface is able to be greatly simplified.

* Organized

Compared to the prototype, the new version is spectacularly better organized.

* Header-Only

The requirement to link to Deep Sea Shell is gone.

## Usage

> [!NOTE]
> Consolidated examples and documentations are work-in-progress while the source code solidifies.

Let's implement an `echo` function in C and connect it to Deep Sea Shell. (This is already included in the Deep Sea Shell standard commands, but we'll do it again here to demonstrate.)
Your workspace should approximately resemble the following for this example:

```
my_project/
├─ src/
│  ├─ main.c
│  ├─ my_definer.h
```
*(Definers are the term for the bridge between C and DSS)* 

As for our Deep Sea Shell command, 
here's our syntax: `echo <args...>`, expecting that it should print the arguments in the console

First, let's implement our command in the definer header file.

```c
#ifndef MY_DEFINER_H
#define MY_DEFINER_H

#include "DSS_mem.h"

int my_echo(DSS_mem_t *mem, int argc, char **argv)
{
	return 0;
}

#endif // MY_DEFINER_H
```
We include `DSS_mem.h` since it allows us to talk to Deep Sea Shell's virtual memory bank.

> [!NOTE]
> The virtual memory is stored in a simple key-pair table. Here is a simple demonstration:
> ```c
> /* (Within the implementation of a command, ideally) */
>
> /* Allocate our pair */
> int *pair = (int *)malloc(sizeof(int));
>	(*pair) = 10;
>
> /* Create the mempair; give it a unique key, send it a pointer to our allocated block,
> and then give it the length of the block. */
> DSS_mempair_t *mempair = DSS_mempair_create("my_key", (void *)pair, 1);
>
> DSS_mem_push_back(mem, mempair);
>
> /* A mutable pointer directly to the pair */
> void *mempair_gotten = DSS_mem_get(mem, "my_key");
> printf("%i\n", *(int *)mempair_gotten);
> ```

For this echo function, `mem` is **not** necessary.
So we will cast it to void.

```c
#ifndef MY_DEFINER_H
#define MY_DEFINER_H

#include "DSS_mem.h"

int my_echo(DSS_mem_t *mem, int argc, char **argv)
{
  (void)mem;
  return 0;
}

#endif // MY_DEFINER_H
```

Next, it's time to actually implement the function. Similarly to a main function (as you've probably noted), `argv` is an array of strings, each having been separated by spaces.

*(The command `echo Hello, world!` turns into the arguments: `Hello,` and `world!`, with a length of two)*

```c
#ifndef MY_DEFINER_H
#define MY_DEFINER_H

#include <stdio.h>

#include "DSS_mem.h"

int my_echo(DSS_mem_t *mem, int argc, char **argv)
{
  (void)mem;

	for (char **iter = argv; iter < (argv + argc); ++iter)
	{
		printf("%s ", *iter);
	}
	printf("\n");

  return 0;
}

#endif // MY_DEFINER_H
```

With the function working, our next goal is to connect it to Deep Sea Shell. This is thankfully simple too.

```c
#ifndef MY_DEFINER_H
#define MY_DEFINER_H

#include <stdio.h>

#include "DSS_mem.h"
#include "DSS_env.h"

/*^^^Implementation...^^^*/

void my_definer(DSS_env_t *env)
{
  DSS_env_define_command(env, "echo", my_echo);
}

#endif // MY_DEFINER_H
```

`DSS_env_define_command` requires an identifier, so we've placed "echo" there. Identifiers are what Deep Sea Shell uses to identify and distinguish commands, so it must be unique.
Syntactically, it always appears as the first token in any DSS statement.

Next, we have to configure the DSS runtime environment. This is as simple as creating it and executing our definer:

```c
/*main.c*/

#include "DSS_env.h"
#include "DSS_stdlang.h"

#include "my_definer.h"

int main(int argc, char **argv)
{
  /*^^^^^any unrelated code^^^^^*/

	DSS_env_t *env = DSS_env_create();
  /* It's a good idea to define the standard language */
	DSS_stdlang_definer(env);

  /* Call your definer. */
  my_definer(env);
}
```

To test it, we can invoke Deep Sea Shell directly:

```c
/* main.c */

  DSS_env_exec(env, "echo Hello, world!");
```

And finally, we destroy the environment.

```c
/* main.c */

  DSS_env_destroy(env);
```

> [!WARNING]
> The freeing of `DSS_mem_t` virtual memory records is delegated to their owners. To prevent memory leaks, it is advisable to connect a function to the environment's `event_destroying` event.
>
> ```c
> /* my_definer.h */
>
> void my_definer_on_env_destroying(void *arg)
> {
> 	/* This particular event will always pass the environment */
>   DSS_env_t *env = (DSS_env_t *)arg;
>
>   /* Free your records in the environment's DSS_mem_t */
> }
> ```
> 
> ```c
> /* main.c */
>
>  DSS_event_connect(env->event_destroying, my_definer_on_env_destroying); 
> ```

## Start to finish, this is the full example:

```c
/* my_definer.h */

#ifndef MY_DEFINER_H
#define MY_DEFINER_H

#include <stdio.h>

#include "DSS_mem.h"
#include "DSS_env.h"

int my_echo(DSS_mem_t *mem, int argc, char **argv)
{
  (void)mem;

	for (char **iter = argv; iter < (argv + argc); ++iter)
	{
		printf("%s ", *iter);
	}
	printf("\n");

  return 0;
}

void my_definer(DSS_env_t *env)
{
  DSS_env_define_command(env, "echo", my_echo);
}

#endif // MY_DEFINER_H
```

```c
/*main.c*/

#include "DSS_env.h"
#include "DSS_stdlang.h"

#include "my_definer.h"

int main(int argc, char **argv)
{
  /* Unused */
  (void)argc;
  (void)argv;

	DSS_env_t *env = DSS_env_create();
  /* It's a good idea to define the standard language */
	DSS_stdlang_definer(env);

  /* Call your definer. */
  my_definer(env);

  /* Test your new command */
  DSS_env_exec(env, "echo Hello, world!");

  /* Destroy the environment */
  DSS_env_destroy(env);

  return 0;
}
```

#include <stdio.h>
#include <assert.h>

#include "DSS_event.h"
#include "DSS_mem.h"
#include "DSS_parser.h"
#include "DSS_stdlang.h"

void do_something_interesting(void *arg)
{
	(void)arg;
	printf("%s", "Something interesting\n");
}

void do_something_else(void *arg)
{
	(void)arg;
	printf("%s", "Something else\n");
}

int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	DSS_event_t *event = DSS_event_create();

	DSS_event_connect(event, do_something_interesting);
	DSS_event_connect(event, do_something_else);

	DSS_event_fire(event, NULL);
	fflush(stdout);

	DSS_event_destroy(event);

	int *mempair_test_value = (int *)malloc(sizeof(int));
	(*mempair_test_value) = 10;
	DSS_mempair_t *mempair = DSS_mempair_create("test_key", (void *)mempair_test_value, 1);
	int *retrieved = (int *)mempair->pair;
	printf("%i\n", *retrieved);

	int *mempair_a_test_value = (int *)malloc(sizeof(int));
	(*mempair_a_test_value) = 11;
	DSS_mempair_t *mempair_a = DSS_mempair_create("Key_a", (void *)mempair_a_test_value, 1);

	int *mempair_b_test_value = (int *)malloc(sizeof(int));
	(*mempair_b_test_value) = 12;
	DSS_mempair_t *mempair_b = DSS_mempair_create("Key_b", (void *)mempair_b_test_value, 1);

	DSS_mem_t *mem = DSS_mem_create();
	DSS_mem_push_back(mem, mempair_a);
	DSS_mem_push_back(mem, mempair_b);

	void *mempair_gotten = DSS_mem_get(mem, "Key_a");

	if (mempair_gotten == NULL)
	{
		perror("main() mempair_gotten is NULL\n");
		abort();
	}

	char *test_trim = strdup("    fresh trim!   ");
	printf("\"%s\"\n", test_trim);
	DSS_string_trim(test_trim);
	printf("\"%s\"\n", test_trim);
	free(test_trim);

	printf("%i\n", *(int *)mempair_gotten);

	char *haystack = "Another Thing {{ Um And So }} Hey No {{Yes}} {{ Again Right Sure }} Oh {{Well Command}}";
	printf("Occurences: %i\n", DSS_string_count_occurences(haystack, " "));

	char **split_res = DSS_string_parse(haystack, " ", true);

	for (char **iter = split_res; *iter != NULL && iter != NULL; ++iter)
	{
		printf("\"%s\"\n", *iter);
	}
	fflush(stdout);

	DSS_string_array_destroy(split_res);

	DSS_env_t *env = DSS_env_create();
	DSS_stdlang_definer(env);
	DSS_env_exec(env, "out {{Hello, world!}}\nout Goodbye, world.");

	return 0;
}

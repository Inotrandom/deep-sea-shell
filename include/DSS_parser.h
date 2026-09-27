#ifndef DSS_PARSER_H
#define DSS_PARSER_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "DSS_defs.h"

unsigned int DSS_string_count_occurences(char *haystack, const char *needle)
{
	unsigned int res = 0;
	unsigned int needle_size = strlen(needle);
	char *last_occurence = strstr(haystack, needle);
	char *this_occurence;

	for (this_occurence = last_occurence; this_occurence != NULL; this_occurence = strstr(last_occurence, needle))
	{
		if (res > DSS_EXHAUST)
		{
			perror("DSS_string_count_occurences() exhaust reached, execution terminated\n");
			return 0;
		}

		last_occurence = this_occurence + needle_size;
		++res;
	}

	return res;
}

unsigned int DSS_count_string_array(char **string_array)
{
	unsigned int res = 0;

	for (char **iter = string_array; *iter != NULL; ++iter)
	{
		if (res > DSS_EXHAUST)
		{
			perror("DSS_count_string_array() exhaust reached, execution terminated\n");
			return 0;
		}
		++res;
	}

	return res;
}

char **DSS_string_split(char *string, const char *delim)
{
	char *copied_string = (char *)malloc(strlen(string) + strlen(delim));

	// Appends a delimeter to the end of the string
	(void)memcpy(copied_string, string, strlen(string));
	(void)memcpy(copied_string + strlen(string), delim, strlen(delim));

	char **res = NULL;
	int res_end_n = 0;
	int res_size = 0;

	res_size = DSS_string_count_occurences(copied_string, delim);

	// Plus one for the NULL last string
	res = (char **)malloc(sizeof(char *) * (res_size + 1));

	char *token = strtok(copied_string, delim);

	while (token)
	{
		// Check for empty strings
		if (token[0] == '\0')
		{
			token = strtok(NULL, delim);
			continue;
		}

		res[res_end_n] = strdup(token);
		++res_end_n;
		token = strtok(NULL, delim);
	}

	// Ensure it always ends on a NULL
	res[res_size + 1] = NULL;

	free(copied_string);

	return res;
}

#endif // DSS_PARSER_H

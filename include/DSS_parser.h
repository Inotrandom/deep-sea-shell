#ifndef DSS_PARSER_H
#define DSS_PARSER_H

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

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

unsigned int DSS_string_array_len(char **string_array)
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

void DSS_string_array_dump(char **string_array)
{
	for (char **iter = string_array; *iter != NULL; ++iter)
	{
		printf("\"%s\"\n", *iter);
	}
}

void DSS_string_array_destroy(char **string_array)
{
	for (char **iter = string_array; *iter != NULL; ++iter)
	{
		free(*iter);
		*iter = NULL;
	}

	free(string_array);
	string_array = NULL;
}

int DSS_string_array_characters_until(char **string_array, const char *needle)
{
	int res = 0;
	char *occurence = NULL;
	for (char **iter = string_array; iter != NULL; ++iter)
	{
		occurence = strstr(*iter, needle);
		if (occurence == NULL)
		{
			res += strlen(*iter) + 1;
			continue;
		}

		int occurence_location = (occurence - *iter);
		res += occurence_location;
		return res;
	}

	return -1;
}

void DSS_string_rm_character(char *string, int idx)
{
	for (unsigned long i = idx; i < strlen(string) - 1; ++i)
	{
		string[i] = string[i + 1];
	}
}

void DSS_string_trim(char *string)
{
	char *iter = string;
	char *iter_back = (string + strlen(string) - 1);
	while (isspace(*iter))
	{
		DSS_string_rm_character(string, 0);
	}

	while (isspace(*iter_back))
	{
		--iter_back;
	}

	++iter_back;
	*iter_back = '\0';
}

char **DSS_string_handle_brackets(char **tokens)
{
	// This awful thing needs fixing

	int bracks_open = 0;
	int bracks_close = 0;
	int bracks = 0;
	char **res = NULL;
	int idx = 0;
	char *word = NULL;
	char *temp = NULL;
	int temp_size = 0;
	int word_n = 0;
	bool in_brack = false;
	int brack_size = 0;

	for (char **iter = tokens; *iter != NULL; ++iter)
	{
		bracks_open += DSS_string_count_occurences(*iter, DSS_LEX_BRACK_OPEN);
		bracks_close += DSS_string_count_occurences(*iter, DSS_LEX_BRACK_CLOSE);
	}
	bracks = fmin(bracks_open, bracks_close);

	if (bracks == 0)
	{
		return NULL;
	}

	unsigned int res_size = sizeof(char *) * ((DSS_string_array_len(tokens) - bracks) + 1);
	res = (char **)malloc(res_size);

	for (char **iter = tokens; *iter != NULL; ++iter)
	{
		if (strstr(*iter, DSS_LEX_BRACK_OPEN) && strstr(*iter, DSS_LEX_BRACK_CLOSE) && in_brack == false)
		{
			int bracklen = strlen(DSS_LEX_BRACK_CLOSE) + strlen(DSS_LEX_BRACK_OPEN);
			int token_len = (strlen(*iter) - (bracklen));
			temp = (char *)malloc(sizeof(char) * (token_len + 1));
			memcpy(temp, *iter + strlen(DSS_LEX_BRACK_OPEN), token_len);
			temp[token_len] = '\0';

			res[idx] = strdup(temp);
			++idx;
			free(temp);
			temp = NULL;
			continue;
		}

		if (strstr(*iter, DSS_LEX_BRACK_CLOSE) && in_brack == true)
		{
			if (strcmp(*iter, DSS_LEX_BRACK_CLOSE) == 0)
			{
				in_brack = false;
				res[idx] = strdup(word);
				++idx;
				free(word);
				word_n = 0;
				word = NULL;

				continue;
			}

			temp = (char *)malloc(sizeof(char) * ((strlen(*iter) - strlen(DSS_LEX_BRACK_CLOSE)) + 1));
			memcpy(temp, *iter, strlen(*iter) - strlen(DSS_LEX_BRACK_CLOSE));

			temp[strlen(*iter) - strlen(DSS_LEX_BRACK_CLOSE)] = '\0';

			memcpy((word + word_n), temp, strlen(temp) + 1);
			free(temp);
			temp = NULL;
			in_brack = false;

			res[idx] = strdup(word);
			++idx;

			free(word);
			word_n = 0;
			word = NULL;

			continue;
		}

		if (in_brack == true)
		{
			// Append a space to the end
			temp_size = strlen(*iter) + 2;
			temp = (char *)malloc(sizeof(char) * temp_size);
			memcpy(temp, *iter, strlen(*iter));
			temp[strlen(*iter)] = ' ';
			temp[strlen(*iter) + 1] = '\0';

			memcpy((word + word_n), temp, strlen(temp));
			word_n += strlen(temp);

			free(temp);
			temp = NULL;

			continue;
		}

		if (strstr(*iter, DSS_LEX_BRACK_OPEN) && in_brack == false)
		{
			in_brack = true;
			brack_size = DSS_string_array_characters_until(iter, DSS_LEX_BRACK_CLOSE);
			if (brack_size == -1)
			{
				DSS_string_array_destroy(res);
				return NULL;
			}

			// Exclude the opening bracket
			word = (char *)malloc(sizeof(char) * brack_size + 1);
			for (char *iter = word; iter < (word + brack_size + 1); ++iter)
				*iter = '\0';

			if (strcmp(*iter, DSS_LEX_BRACK_OPEN) == 0)
			{
				continue;
			}
			memcpy(word, (*iter + strlen(DSS_LEX_BRACK_OPEN)), strlen(*iter) - strlen(DSS_LEX_BRACK_OPEN));
			word_n = strlen(*iter) - strlen(DSS_LEX_BRACK_OPEN);

			memcpy((word + word_n), " ", 1);
			++word_n;

			continue;
		}

		res[idx] = strdup(*iter);
		++idx;
	}

	if (word)
	{
		free(word);
		word = NULL;
	}

	// Ensure it ends on NULL
	res[idx] = NULL;

	return res;
}

char **DSS_string_parse(char *string, const char *delim, bool handle_brackets)
{
	char *copied_string = (char *)malloc(strlen(string) + strlen(delim) + 2);

	// Appends a delimeter to the end of the string
	memcpy(copied_string, string, strlen(string));
	memcpy(copied_string + strlen(string), delim, strlen(delim));
	*(copied_string + strlen(string) + strlen(delim)) = '\0';

	char **split = NULL;
	int split_end_n = 0;
	int split_size = 0;

	split_size = DSS_string_count_occurences(copied_string, delim);

	// Plus one for the NULL last string
	split = (char **)malloc(sizeof(char *) * (split_size + 1));

	char *token = strtok(copied_string, delim);

	while (token)
	{
		// Check for empty strings
		if (token[0] == '\0')
		{
			token = strtok(NULL, delim);
			continue;
		}

		split[split_end_n] = strdup(token);
		++split_end_n;
		token = strtok(NULL, delim);
	}

	// Ensure it always ends on a NULL
	split[split_size] = NULL;

	char **res = NULL;
	if (handle_brackets == true)
	{
		res = DSS_string_handle_brackets(split);
	}

	if (res == NULL)
	{
		free(copied_string);
		copied_string = NULL;
		return split;
	}
	DSS_string_array_destroy(split);
	free(copied_string);

	copied_string = NULL;

	return res;
}

#endif // DSS_PARSER_H

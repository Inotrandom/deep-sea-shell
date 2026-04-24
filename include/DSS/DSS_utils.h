/**
 * This file contains useful functions as well as several important
 * parts that DSS requires to function. If it is simply a small
 * piece of DSS that does not directly contribute to the runtime
 * function of DSS, then it likely belongs here.
 */

#ifndef DSS_UTILS_H
#define DSS_UTILS_H

#include <vector>
#include <string>
#include <optional>
#include <fstream>
#include <sstream>

namespace DSS
{

/**
 * Splits a string into multiple substrings, separated by a delimiter.
 *
 * @param s The input string to be operated over
 * @param delimiter A delimiter to determine how tokens are separated
 *
 * @returns A vector of substrings after having been separated by delimiter.
 */
inline std::vector<std::string> string_split(std::string s, const std::string &delimiter)
{
	std::vector<std::string> tokens;
	std::size_t pos = 0;
	std::string token;

	while ((pos = s.find(delimiter)) != std::string::npos)
	{
		token = s.substr(0, pos);
		tokens.push_back(token);
		s.erase(0, (pos + delimiter.length()));
	}
	tokens.push_back(s);

	return tokens;
}

/**
 * Finds all occurances and replaces them in a string.
 *
 * @param s The string to be operated over
 *
 * @param what The string to match occurences in `s`
 *
 * @param with The string to replace `what` with
 */
inline void string_replace(std::string &s, std::string &what, const std::string &with)
{
	std::size_t pos = 0;

	while ((pos = s.find(what)) != std::string::npos)
	{
		s.replace(pos, what.size(), with);
	}
}

/**
 * Reads the contents of a file.
 *
 * @param path The relative path from the CWD
 *
 * @return std::nullopt if the file failed to read, or std::string
 * if it was successful
 */
inline std::optional<std::string> file_read(std::string &path)
{
	std::ifstream file(path);

	if (file.is_open() == false)
	{
		return std::nullopt;
	}

	std::stringstream buf;
	buf << file.rdbuf();

	return buf.str();
}

}; // namespace DSS

#endif // H_DSS_UTILS
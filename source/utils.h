/**
 * This file contains useful functions as well as several important
 * parts that DSS requires to function. If it is simply a small
 * piece of DSS that does not directly contribute to the runtime
 * function of DSS, then it likely belongs here.
 */

#ifndef utils
#define utils

#include <vector>
#include <string>
#include <algorithm>
#include <iterator>

/**
 * Splits a string into multiple substrings, separated by a delimiter.
 * 
 * @param s The input string to be operated over
 * @param delimiter A delimiter to determine how tokens are separated
 * 
 * @returns A vector of substrings after having been separated by delimiter.
 */
static inline std::vector<std::string> string_split(std::string s, const std::string& delimiter) {
    std::vector<std::string> tokens;
    std::size_t pos = 0;
    std::string token;

    while ((pos = s.find(delimiter)) != std::string::npos) {
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
static inline void string_replace(std::string& s, std::string& what, const std::string& with) {
    std::size_t pos = 0;

    while ((pos = s.find(what)) != std::string::npos) {
        s.replace(pos, what.size(), with);
    }
}

/**
 * Function pointers are connected to the delegate,
 * which can be called with the same arguments.
 * 
 * This is sprinkled throughout the program, and is
 * used to organize the scopes of commands. Furthermore,
 * it is used for the case that a command might require
 * more than one function to be called. 
 */
template<typename F, typename A, typename R>
class Delegate
{
private:
    std::vector<F> m_connected;

public:
    Delegate()
    {
        m_connected = {};
    }

    /**
     * Connect a DSSFunc to the delegate.
     * 
     * This will eventually be used in the `call()` function.
     * Multiple functions can be connected to the same delegate
     * (this is how a delegate works)
     * 
     * @param func The function pointer to connect to the delegate
     */
    void connect(const F func)
    {
        if (this == nullptr) {return;}

        m_connected.push_back(func);
    }

    /**
     * Attempts to disconnect a DSSFunc from the delegate, if it exists.
     * 
     * Will return if `func` does not exist.
     * 
     * @param func The function pointer to attempt to disconnect from the delegate.
     */
    void disconnect(const F func)
    {
        if (this == nullptr) {return;}

        const auto found = std::find(m_connected.begin(), m_connected.end(), func);
        
        if (found == m_connected.end()) {return;}
        m_connected.erase(found);
    }

    /**
     * Calls the delegate.
     * 
     * This will pass a copy of the arguments to every single function connected
     * to the delegate. Additionally, this will execute every single function
     * (in the order that they were connected- the most recently connected
     * will be executed last.)
     * 
     * @param arg An argument to be copied for each function
     * 
     * @return A vector of integers representing status from each function, in corresponding order
     * to the `connected` member. Note: this vector will be empty if this function errors. Undefined
     * behavior will not appear in the vector, and as such, relying on the fact that the number
     * of returns will be consistent is unsafe.
     */
    template<typename... Ts>
    auto call(Ts... arg) -> R
    {
        R res;

        if (this == nullptr) {return res;}

        for (auto func : m_connected)
        {
            res.push_back(func(arg...));
        }

        return res;
    }
};

#endif
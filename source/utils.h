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
static inline std::vector<std::string> string_split(std::string& s, const std::string& delimiter) {
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
 * Arguments to functions connected to delegates
 */
typedef std::vector<std::string> DSSFuncArgs;

/**
 * Return type of functions connected to delegates
 */
typedef std::size_t DSSReturnType;

/**
 * Typing of function pointers that should be
 * passed onto delegates
 */
typedef DSSReturnType (*DSSFunc)(DSSFuncArgs);

/**
 * Return type of Delegate::call and any other
 * functions that pass this result down the 
 * pipeline
 */
typedef std::vector<DSSReturnType> DSSDelegateReturnType;

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
    std::vector<F> connected;

public:
    inline Delegate()
    {
        this->connected = {};
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
    inline void connect(const F func)
    {
        if (this == nullptr) {return;}

        this->connected.push_back(func);
    }

    /**
     * Attempts to disconnect a DSSFunc from the delegate, if it exists.
     * 
     * Will return if `func` does not exist.
     * 
     * @param func The function pointer to attempt to disconnect from the delegate.
     */
    inline void disconnect(const F func)
    {
        if (this == nullptr) {return;}

        const auto found = std::find(this->connected.begin(), this->connected.end(), func);
        
        if (found == this->connected.end()) {return;}
        this->connected.erase(found);
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
    auto call(A arg) -> R
    {
        R res;

        if (this == nullptr) {return res;}

        for (auto func : this->connected)
        {
            res.push_back(func(arg));
        }

        return res;
    }
};

/**
 * Consider that DSS Commands are essentially named DSSDelegates.
 * 
 * A DSS Command has a name and a description. "Name" is the command's
 * keyword, and "description" should contain a basic manual for the command's
 * usage.
 * 
 * @note DSS Commands are fluid- they are defined and deleted at run time.
 * Assume that no value will be constant, as the command may be constructed
 * at arbitrary times.
 */
class Command
{
private:
    Delegate<DSSFunc> delegate;
    std::string name;
    std::string description;
    int64_t minimum_args;
    int64_t maximum_args;

public:
    /**
     * @param name Keyword for the command. This will determine
     * what token invokes this command.
     * 
     * @param description The manual for the command.
     * 
     * @param minimum_args The minimum expected arguments for the command. (optional)
     * 
     * @param maximum_args The maximum of arguments expected for the command. (optional)
     */
    inline Command(std::string name, std::string description, int64_t minimum_args = -1, int64_t maximum_args = -1)
    {
        this->delegate = Delegate<DSSFunc>();
        this->name = name;
        this->description = description;
        this->minimum_args = minimum_args;
        this->maximum_args = maximum_args;
    }

    /**
     * Lazily attempts to run this command if the keyword (first token)
     * matches the command's "name"
     * 
     * @param inp The string command to be executed (should contain arguments)
     * @param delim The separation between individual tokens in a command.
     * 
     * @return The result of calling the delegate associated with this command.
     * Will return an empty vector if an error has occured;
     * 
     * @see DSSDelegate::call
     */
    inline auto attempt_parse_and_exec(std::string inp, std::string delim) -> DSSDelegateReturnType
    {
        DSSDelegateReturnType res = {};
        
        if (this == nullptr) {return res;}

        std::vector<std::string> args = string_split(inp, delim);

        if (args[0] != this->name) {return res;}

        args.erase(args.begin());

        std::size_t arg_count = args.size();

        if (arg_count < this->minimum_args) {return res;}
        if (this->maximum_args != -1 && arg_count > maximum_args) {return res;}

        res = this->delegate.call(args);
        return res;
    }
};

#endif
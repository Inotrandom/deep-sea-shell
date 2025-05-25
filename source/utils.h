/**
 * This file contains useful functions as well as several important
 * parts that DSS requires to function. If it is simply a small
 * piece of DSS that does not directly contribute to the runtime
 * function of DSS, then it likely belongs here.
 */

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
inline std::vector<std::string> string_split(std::string& s, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;

    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        tokens.push_back(token);
        s.erase(0, (pos + delimiter.length()));
    }
    tokens.push_back(s);

    return tokens;
}

typedef std::vector<std::string> DSSFuncArgs;
typedef size_t (*DSSFunc)(DSSFuncArgs);
typedef std::vector<size_t> DSSReturnType;

/**
 * Function pointers are connected to the DSSDelegate,
 * which can be called with the same arguments.
 * 
 * This is sprinkled throughout the program, and is
 * used to organize the scopes of commands. Furthermore,
 * it is used for the case that a command might require
 * more than one function to be called. 
 */
class DSSDelegate
{
    private:
        std::vector<DSSFunc> connected;
    
    public:
        inline DSSDelegate()
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
        inline void connect(const DSSFunc func)
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
        inline void disconnect(const DSSFunc func)
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
         * @param args A vector of string arguments to be passed into every connected function
         * 
         * @return A vector of integers representing status from each function, in corresponding order
         * to the `connected` member. Note: this vector will be empty if this function errors. Undefined
         * behavior will not appear in the vector, and as such, relying on the fact that the number
         * of returns will be consistent is unsafe.
         */
        inline auto call(DSSFuncArgs args) -> DSSReturnType
        {
            DSSReturnType res = {};

            if (this == nullptr) {return res;}

            for (auto func : this->connected)
            {
                res.push_back(func(args));
            }

            return res;
        }
};

/**
 * Consider that DSSCommands are essentially named DSSDelegates.
 * 
 * A DSSCommand has a name and a description. "Name" is the command's
 * keyword, and "description" should contain a basic manual for the command's
 * usage.
 */
class DSSCommand
{
    private:
        DSSDelegate delegate;
        std::string name;
        std::string description;
    
    public:
        inline DSSCommand(std::string name, std::string description)
        {
            this->delegate = DSSDelegate();
            this->name = name;
            this->description = description;
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
        inline auto parse_and_exec(std::string inp, std::string delim) -> DSSReturnType
        {
            DSSReturnType res = {};
            
            if (this == nullptr) {return res;}

            std::vector<std::string> args = string_split(inp, delim);
            args.erase(args.begin());

            res = this->delegate.call(args);
        }
};
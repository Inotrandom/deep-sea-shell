#include <vector>
#include <string>
#include <algorithm>
#include <iterator>

typedef std::vector<std::string> DSSFuncArgs;
typedef int (*DSSFunc)(DSSFuncArgs);

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
        inline auto call(DSSFuncArgs args) -> std::vector<int>
        {
            std::vector<int> res = {};

            if (this == nullptr) {return res;}

            for (auto func : this->connected)
            {
                res.push_back(func(args));
            }

            return res;
        }
};
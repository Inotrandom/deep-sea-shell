#ifndef DSS_DELEGATE_H
#define DSS_DELEGATE_H

#include <cstdint>
#include <vector>
#include <algorithm>

namespace DSS
{

/**
 * Function pointers are connected to the delegate,
 * which can be called with the same arguments.
 *
 * This is sprinkled throughout the program, and is
 * used to organize the scopes of commands. Furthermore,
 * it is used for the case that a command might require
 * more than one function to be called.
 */
template <typename F, typename R> class delegate_t
{
private:
	std::vector<F> m_connected;
	std::uint32_t m_max_connected;

public:
	explicit delegate_t(int max_connected = 1)
	{
		m_connected = {};
		m_max_connected = max_connected;
	}

	~delegate_t() {}

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
		if (m_connected.size() == std::size_t(m_max_connected))
		{
			return;
		}

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
		const auto found = std::find(m_connected.begin(), m_connected.end(), func);

		if (found == m_connected.end())
		{
			return;
		}
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
	template <typename... Ts> auto call(Ts... arg) -> R
	{
		R res;

		for (auto func : m_connected)
		{
			res.push_back(func(arg...));
		}

		return res;
	}
};

} // namespace DSS

#endif // DSS_DELEGATE_H
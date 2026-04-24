#ifndef DSS_EXECUTOR_H
#define DSS_EXECUTOR_H

#include <vector>
#include <memory>

#include "DSS_task.h"
#include "DSS_delegate.h"
#include "DSS_utils.h"
#include "DSS_lexicon.h"

namespace DSS
{
class environment_t;
class executor_t;
typedef int (*command_signature_t)(std::shared_ptr<DSS::executor_t>, std::vector<std::string>);

class executor_t
{
public:
	explicit executor_t() {}
	~executor_t() {}

private:
	void parse_task(DSS::task_t t)
	{
		std::vector<std::string> lines = DSS::string_split(t.get_contents(), std::string(DSS::STATEMENT_DELIM));

		if (lines.size() == 0)
		{
			return;
		}

		for (const auto &line : lines)
		{
			std::vector<std::string> tokens = DSS::string_split(line, std::string(DSS::TOKEN_DELIM));
		}
	}

	std::shared_ptr<DSS::environment_t> m_parent_environment;
	std::shared_ptr<DSS::delegate_t<DSS::command_signature_t, int>> m_command_definers;
	std::shared_ptr<DSS::delegate_t<DSS::command_signature_t, int>> m_preprocessor_definers;
	std::vector<DSS::task_t> m_tasks;
};
} // namespace DSS

#endif // DSS_EXECUTOR_H
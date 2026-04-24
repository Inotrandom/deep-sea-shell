#ifndef DSS_ENVIRONMENT_H
#define DSS_ENVIRONMENT_H

#include <vector>
#include <memory>
#include <map>
#include <any>

#include "DSS_executor.h"

namespace DSS
{
class environment_t
{
public:
	explicit environment_t() {}
	~environment_t() {}

	auto get_mem() -> std::shared_ptr<std::map<std::string, std::any>> { return m_mem; }

private:
	std::shared_ptr<std::map<std::string, std::any>> m_mem;
	std::vector<std::shared_ptr<DSS::executor_t>> m_executors;
};
} // namespace DSS

#endif // DSS_ENVIRONMENT_H
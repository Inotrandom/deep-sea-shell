#ifndef DSS_TASK_H
#define DSS_TASK_H

#include <string>

namespace DSS
{

class task_t
{
public:
	explicit task_t() {}
	~task_t() {}

	auto get_contents() -> std::string & { return m_contents; }

private:
	std::string m_contents;
};

} // namespace DSS

#endif // DSS_TASK_H
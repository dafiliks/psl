#ifndef CMDARGS_HPP
#define CMDARGS_HPP

#include <string>

class CmdArgs
{
public:
	explicit CmdArgs(int argc, char **argv);

	void handle();

	[[nodiscard]] const std::string &get_source_code() const;
	[[nodiscard]] std::string get_source_path() const;
	[[nodiscard]] std::string get_target_output_flag() const;

private:
	void validate_args();
	[[nodiscard]] std::string file_to_string(const char *file_name);

	// private members
	int m_argc{};
	char **m_argv{};

	std::string m_source_code{};
};

#endif

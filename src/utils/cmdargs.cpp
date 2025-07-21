#include <filesystem>
#include <iostream>
#include <fstream>

#include "../frontend/lexer.hpp"
#include "../utils/cmdargs.hpp"

CmdArgs::CmdArgs(int argc, char** argv) : m_argc(argc), m_argv(argv) {}

void CmdArgs::handle()
{
	validate_args();
	m_source_code = file_to_string(m_argv[1]);
}

[[nodiscard]] const std::string& CmdArgs::get_source_code()       const { return m_source_code; }
[[nodiscard]] std::string CmdArgs::get_source_path()              const { return std::string{m_argv[1]}; }
[[nodiscard]] std::string CmdArgs::get_target_output_flag()       const { return std::string{m_argv[2]}; }

void CmdArgs::validate_args()
{
	if (m_argc < 2) {
		printf("wrong number of arguments provided");
	}

	if (!std::filesystem::exists(m_argv[1])) {
		printf("file doesn't exist");
	}

	if (std::filesystem::path(m_argv[1]).extension() != ".pseudo") {
		printf("file lacks '.pseudo' extension");
	}
}

[[nodiscard]] std::string CmdArgs::file_to_string(const char* file_name)
{
	std::ifstream file{};
	file.open(file_name);
	if (!file.is_open()) {
		Error{"file '" + std::string{file_name} + "' could not be opened"};
	}
	std::string source{std::istreambuf_iterator<char>(file),
	                   std::istreambuf_iterator<char>()};
	file.close();
	return source;
}

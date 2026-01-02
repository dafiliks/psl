/* compiler/compilations_targets.cpp by David Filiks */
/* The compilation targets implementation for the PsL compiler */

#include "compilation_targets.hpp"

CompilationTarget::CompilationTarget(const OutputTarget& target)
/* Initialize the output target */
: m_target(target) {}

GCCTarget::GCCTarget(const OutputTarget& target)
/* Initialize the CompilationTarget object */
: CompilationTarget(target) {}

void GCCTarget::compile(const std::string_view cpp_file)
{
    /* Deduce output file name, by removing the ".cpp" from the file name */
    std::filesystem::path output_file{cpp_file};
    output_file.replace_extension("");

    /* Variable that holds the final command that will be executed */
    std::string command_str{};

    /* Switch through all the possible output targets */
    switch (m_target)
    {
        /* No case for the CPP target as it is the default output from the generator and requires no C++ compiler calls */

        /* If the output target is EXE */
        case (OutputTarget::EXE):
            /* Store the command that compiles the ".cpp" file to EXE with GCC */
            command_str = "g++ " + output_file.string() + ".cpp -std=c++20 -o " + output_file.string() + " && ";

            /* If the user is using EITHER 32-bit or 64-bit Windows */
            #if defined(_WIN32)
                /* Add a ".exe" to the end of the executable name for obvious reasons */
                command_str += output_file.string() + ".exe";
            /* If the user is not using Windows */
            #else
                /* Only prepend the output file path on MacOS and Linux */
                command_str += output_file.string();
            #endif

            /* Pipe the output into a file, which is useful for the GUI output box */
            command_str += " > __psl_out.txt";

            /* Break from the switch case */
            break;

        /* If the output target is ASM */
        case (OutputTarget::ASM):
            /* Store the command that compiles the ".cpp" file to ASM with GCC */
            command_str = "g++ -S " + output_file.string() + ".cpp -std=c++20 -o " + output_file.string() + ".s";

            /* Break from the switch case */
            break;

        /* If the output target is OBJ */
        case (OutputTarget::OBJ):
            /* Store the command that compiles the ".cpp" file to OBJ with GCC */
            command_str = "g++ -c " + output_file.string() + ".cpp -std=c++20 -o " + output_file.string() + ".o";

            /* Break from the switch case */
            break;

        /* If no matches are made */
        default:
            /* Error out */
            CompileError
            {
                "output target is unrecognized, re-executing the program might help"
            };

            /* Break from the switch case */
            break;
    }

    /* Execute the final command using the default shell */
    std::system(command_str.c_str());
}

MSVCTarget::MSVCTarget(const OutputTarget& target)
/* Initialize the CompilationTarget object */
: CompilationTarget(target) {}

void MSVCTarget::compile(const std::string_view cpp_file)
{
    /* Deduce output file name, by removing the ".cpp" from the file name */
    std::filesystem::path output_file{cpp_file};
    output_file.replace_extension("");

    /* Variable that holds the final command that will be executed */
    std::string command_str{};

    /* Switch through all the possible output targets */
    switch (m_target)
    {
        /* No case for the CPP target as it is the default output from the generator and requires no C++ compiler calls */

        /* If the output target is EXE */
        case (OutputTarget::EXE):
            /* Store the command that compiles the ".cpp" file to EXE with MSVC */
            command_str = "cl /std:c++20 /EHsc " + output_file.string() + ".cpp /Fe:" + output_file.string() + " && " + output_file.string() + ".exe";

            /* Pipe the output into a file, which is useful for the GUI output box */
            command_str += " > __psl_out.txt";

            /* Break from the switch case */
            break;

        /* If the output target is ASM */
        case (OutputTarget::ASM):
            /* Store the command that compiles the ".cpp" file to ASM with MSVC */
            command_str = "cl /std:c++20 /EHsc /FA " + output_file.string() + ".cpp /Fa" + output_file.string() + ".asm";

            /* Break from the switch case */
            break;

        /* If the output target is OBJ */
        case (OutputTarget::OBJ):
            /* Store the command that compiles the ".cpp" file to OBJ with MSVC */
            command_str = "cl /std:c++20 /EHsc /c " + output_file.string() + ".cpp /Fo" + output_file.string() + ".obj";

            /* Break from the switch case */
            break;

        /* If no matches are made */
        default:
            /* Error out */
            CompileError
            {
                "output target is unrecognized, re-executing the program might help"
            };

            /* Break from the switch case */
            break;
    }

    /* Execute the final command using the default shell */
    std::system(command_str.c_str());
}

ClangTarget::ClangTarget(const OutputTarget& target)
/* Initialize the CompilationTarget object */
: CompilationTarget(target) {}

void ClangTarget::compile(const std::string_view cpp_file)
{
    /* Deduce output file name, by removing the ".cpp" from the file name */
    std::filesystem::path output_file{cpp_file};
    output_file.replace_extension("");

    /* Variable that holds the final command that will be executed */
    std::string command_str{};

    /* Switch through all the possible output targets */
    switch (m_target)
    {
    /* No case for the CPP target as it is the default output from the generator and requires no C++ compiler calls */

        /* If the output target is EXE */
        case (OutputTarget::EXE):
            /* Store the base command which compiles the ".cpp" file to EXE with Clang */
            command_str = "clang++ " + output_file.string() + ".cpp -std=c++20 -o " + output_file.string() + " && ";

            /* If the user is using EITHER 32-bit or 64-bit Windows */
            #if defined(_WIN32)
                /* Add a ".exe" to the end of the executable name for obvious reasons */
                command_str += output_file.string() + ".exe";
            /* If the user is not using Windows */
            #else
                /* Only prepend the output file path on MacOS and Linux */
                command_str += output_file.string();
            #endif

            /* Pipe the output into a file, which is useful for the GUI output box */
            command_str += " > __psl_out.txt";

            /* Break from the switch case */
            break;

        /* If the output target is ASM */
        case (OutputTarget::ASM):
            /* Store the command that compiles the ".cpp" file to ASM with Clang */
            command_str = "clang++ -S " + output_file.string() + ".cpp -std=c++20 -o " + output_file.string() + ".s";

            /* Break from the switch case */
            break;

        /* If the output target is OBJ */
        case (OutputTarget::OBJ):
            /* Store the command that compiles the ".cpp" file to OBJ with Clang */
            command_str = "clang++ -c " + output_file.string() + ".cpp -std=c++20 -o " + output_file.string() + ".o";

            /* Break from the switch case */
            break;

        /* If no matches are made */
        default:
            /* Error out */
            CompileError
            {
                "output target is unrecognized, re-executing the program might help"
            };

            /* Break from the switch case */
            break;
    }

    /* Execute the final command using the default shell */
    std::system(command_str.c_str());
}

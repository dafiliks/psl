/* compiler/compiler.cpp by David Filiks */
/* The compiler interface implementation for the PsL compiler */

#include "compiler.hpp"

Compiler::Compiler(const CLIArgs& args)
/* Initialize member variables */ 
: m_args(args),
  m_lexer(m_args),
  m_parser(m_lexer),
  m_gen(m_parser) {}

void Compiler::compile()
{
    /* Compile source file to a C++20 compliant ".cpp" file */
    compile_to_cpp();

    /* Compile the ".cpp" file into the desired output target */
    compile_to_output_target(m_args.get_source_path());
}

void Compiler::compile_to_cpp()
{
    /* Construct and execute lexing process */
    m_lexer = Lexer{m_args};
    m_lexer.execute();

    /* Construct and execute parsing process */
    m_parser = Parser{m_lexer};
    m_parser.execute();

    /* Construct and execute code generation process */
    m_gen = Generator{m_parser};
    m_gen.execute();
}

void Compiler::compile_to_output_target(const std::string_view cpp_file)
{
    /* Declare a smart pointer to a CompilationTarget */
    std::unique_ptr<CompilationTarget> target{};

    /* If the target output flag is "-exe" */
    if (m_args.get_target_output_flag() == "-exe")
    {
        /* If the user is using the G++ compiler */
        #if defined(__GNUG__)
            /* Set the target compiler to GCC (G++) and the output target to EXE */
            target = std::make_unique<GCCTarget>(OutputTarget::EXE);

        /* If the user is using the MSVC compiler */
        #elif defined(_MSC_VER)
            /* Set the target compiler to MSVC and the output target to EXE */
            target = std::make_unique<MSVCTarget>(OutputTarget::EXE);

        /* If the user is using the Clang compiler */
        #elif defined(__clang__)
            /* Set the target compiler to Clang and the output target to EXE */
            target = std::make_unique<ClangTarget>(OutputTarget::EXE);
        #endif
    }

    /* If the target output flag is "-cpp" */
    else if (m_args.get_target_output_flag() == "-cpp")
    {
        /* If the user is using the G++ compiler */
        #if defined(__GNUG__)
            /* Set the target compiler to GCC (G++) and the output target to CPP */
            target = std::make_unique<GCCTarget>(OutputTarget::CPP);

        /* If the user is using the MSVC compiler */
        #elif defined(_MSC_VER)
            /* Set the target compiler to MSVC and the output target to CPP */
            target = std::make_unique<MSVCTarget>(OutputTarget::CPP);

        /* If the user is using the Clang compiler */
        #elif defined(__clang__)
            /* Set the target compiler to Clang and the output target to CPP */
            target = std::make_unique<ClangTarget>(OutputTarget::CPP);
        #endif
    }

    /* If the target output flag is "-asm" */
    else if (m_args.get_target_output_flag() == "-asm")
    {
        /* If the user is using the G++ compiler */
        #if defined(__GNUG__)
            /* Set the target compiler to GCC (G++) and the output target to ASM */
            target = std::make_unique<GCCTarget>(OutputTarget::ASM);

        /* If the user is using the MSVC compiler */
        #elif defined(_MSC_VER)
            /* Set the target compiler to MSVC and the output target to ASM */
            target = std::make_unique<MSVCTarget>(OutputTarget::ASM);

        /* If the user is using the Clang compiler */
        #elif defined(__clang__)
            /* Set the target compiler to Clang and the output target to ASM */
            target = std::make_unique<ClangTarget>(OutputTarget::ASM);
        #endif
    }


    /* If the target output flag is "-obj" */
    else if (m_args.get_target_output_flag() == "-obj")
    {
        /* If the user is using the G++ compiler */
        #if defined(__GNUG__)
            /* Set the target compiler to GCC (G++) and the output target to OBJ */
            target = std::make_unique<GCCTarget>(OutputTarget::OBJ);

        /* If the user is using the MSVC compiler */
        #elif defined(_MSC_VER)
            /* Set the target compiler to MSVC and the output target to OBJ */
            target = std::make_unique<MSVCTarget>(OutputTarget::OBJ);

        /* If the user is using the Clang compiler */
        #elif defined(__clang__)
            /* Set the target compiler to Clang and the output target to OBJ */
            target = std::make_unique<ClangTarget>(OutputTarget::OBJ);
        #endif
    }

    /* If the target was set */
    if (target)
    {
        /* Compile into the output target using the desired compiler */
        target->compile(cpp_file);
    }

    /* If the target was not set */
    else
    {
        /* Throw compilation error */
        throw CompileError
        {
            "no valid compilation target found for flag \"" +
            m_args.get_target_output_flag() +
            "\""
        };
    }
}
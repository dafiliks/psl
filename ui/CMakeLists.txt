cmake_minimum_required(VERSION 3.19)
project(psl LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets LinguistTools)

qt_standard_project_setup()

qt_add_executable(psl
    WIN32 MACOSX_BUNDLE
    main.cpp
    psl.cpp
    ../src/frontend/lexer.cpp
    ../src/frontend/parser.cpp
    ../src/backend/gen.cpp
    ../src/utils/error.cpp
    ../src/utils/error_types.cpp
    ../src/utils/cliargs.cpp
    ../src/compiler/compiler.cpp
    ../src/compiler/compilation_targets.cpp
    psl.h
    psl.ui
)

qt_add_translations(
    TARGETS psl
    TS_FILES psl_en_150.ts
)

target_link_libraries(psl
    PRIVATE
        Qt::Core
        Qt::Widgets
)

include(GNUInstallDirs)

install(TARGETS psl
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET psl
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})

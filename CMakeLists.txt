cmake_minimum_required (VERSION 3.1)

project (Interpreter)

### setup ANTLR4
LIST (APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
SET (CMAKE_CXX_STANDARD 14)
SET (ANTLR4CPP_JAR_LOCATION ${PROJECT_SOURCE_DIR}/lib/antlr/antlr-4.7.1-complete.jar)

include (ExternalAntlr4Cpp)
link_directories(${ANTLR4CPP_LIBS})
include_directories(${ANTLR4CPP_INCLUDE_DIRS})

### setup main project

# dependencies
find_package(Threads REQUIRED)

# add ANTLR generate method
antlr4cpp_process_grammar(Interpreter antlr
        ${CMAKE_CURRENT_SOURCE_DIR}/grammer/LangLexer.g4
        ${CMAKE_CURRENT_SOURCE_DIR}/grammer/LangParser.g4)

# add include dirs
include_directories(${antlr4cpp_include_dirs_antlr} "src/")

# main executable definition
add_executable(interpreter
        ${antlr4cpp_src_files_antlr}
        "src/Scheduler.cpp"
        "src/Runtime.cpp"
        "src/TestRuntime.cpp"
        "src/main.cpp"
        )

add_dependencies(interpreter antlr4cpp antlr4cpp_generation_antlr)

target_link_libraries(interpreter
        Threads::Threads
        antlr4-runtime
)

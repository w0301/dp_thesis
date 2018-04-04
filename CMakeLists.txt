cmake_minimum_required (VERSION 3.1)

project (Interpreter)

# dependencies
find_package(Threads REQUIRED)
find_package(Boost 1.60 COMPONENTS program_options REQUIRED)

# main executable definition
add_executable(interpreter "")

target_link_libraries(interpreter
  Threads::Threads
  Boost::program_options
)

target_include_directories(interpreter
  PRIVATE
    "include/"
)

target_sources(interpreter
  PRIVATE
    "src/worker.cpp"
    "src/scheduler.cpp"
    "src/runtime.cpp"
    "src/test_runtime.cpp"
    "src/main.cpp"
)

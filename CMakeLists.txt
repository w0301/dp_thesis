cmake_minimum_required (VERSION 3.1)

project (Interpreter)

# dependencies
find_package(Threads REQUIRED)

# main executable definition
add_executable(interpreter "")

target_link_libraries(interpreter
  Threads::Threads
)

target_include_directories(interpreter
  PRIVATE
    "include/"
)

target_sources(interpreter
  PRIVATE
    "src/Worker.cpp"
    "src/Scheduler.cpp"
    "src/Runtime.cpp"
    "src/TestRuntime.cpp"
    "src/main.cpp"
)

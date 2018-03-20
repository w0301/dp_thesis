cmake_minimum_required (VERSION 3.1)

project (Interpreter)

# main executable definition
add_executable(interpreter "")

target_include_directories(interpreter
  PRIVATE
    "include/"
)

target_sources(interpreter
  PRIVATE
    "src/main.cpp"
    "src/worker.cpp"
    "src/scheduler.cpp"
)

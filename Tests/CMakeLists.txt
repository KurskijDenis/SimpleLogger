add_executable(SimpleLoggerTests Main.cpp SimpleLogTests.cpp)
target_link_libraries(SimpleLoggerTests gtest SimpleLogger)
target_compile_options(SimpleLogger PRIVATE -std=c++17 -Wextra -Werror -Wall)

add_test(SimpleLoggerTests SimpleLoggerTests)
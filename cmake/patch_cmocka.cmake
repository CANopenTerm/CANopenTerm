cmake_minimum_required(VERSION 3.10)

if(NOT DEFINED CMocka_PATH)
    message(FATAL_ERROR "CMocka_PATH is not defined")
endif()

file(READ "${CMocka_PATH}/CMakeLists.txt" _contents)
string(REPLACE "                \"\${cmocka_BINARY_DIR}/compile_commands.json\"" "#                \"\${cmocka_BINARY_DIR}/compile_commands.json\"" _contents "${_contents}")
string(REPLACE "                \"\${cmocka_SOURCE_DIR}/compile_commands.json\"" "#                \"\${cmocka_SOURCE_DIR}/compile_commands.json\"" _contents "${_contents}")
string(REPLACE "execute_process(COMMAND cmake -E create_symlink" "# execute_process(COMMAND cmake -E create_symlink" _contents "${_contents}")
file(WRITE "${CMocka_PATH}/CMakeLists.txt" "${_contents}")

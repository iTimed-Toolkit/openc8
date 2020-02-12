cmake_minimum_required(VERSION 3.10)
project(checkm8_tool)
enable_language(C ASM)

include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

include_directories(c8_remote/include)
include_directories(include)

ExternalProject_Add(checkm8_arduino
                        PREFIX      c8_arduino/
                        SOURCE_DIR  ${CMAKE_CURRENT_LIST_DIR}/c8_arduino/
                        BINARY_DIR  ${CMAKE_CURRENT_BINARY_DIR}/checkm8_arduino
                        INSTALL_COMMAND "")

ExternalProject_Add_Step(checkm8_arduino upload
                            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/checkm8_arduino/
                            COMMAND cmake --build . --target upload
                            DEPENDEES build
                            EXCLUDE_FROM_MAIN true)

ExternalProject_add_StepTargets(checkm8_arduino upload build)

add_subdirectory(c8_remote/lib)
add_subdirectory(c8_remote)

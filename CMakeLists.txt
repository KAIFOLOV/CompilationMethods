cmake_minimum_required(VERSION 3.14)

project(CompilationMethods LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

foreach(_target SyntacticAnalyze LexicalAnalysis)
    add_executable(${_target} "${_target}.cpp")

    if (_target STREQUAL "SyntacticAnalyze")
        target_sources(${_target} PRIVATE
            Parser.h Parser.cpp
        )
    endif()

    if (_target STREQUAL "LexicalAnalysis")
        target_sources(${_target} PRIVATE
        )
    endif()

    target_link_libraries(${_target}
        Qt${QT_VERSION_MAJOR}::Core
    )
endforeach()

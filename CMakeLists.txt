cmake_minimum_required(VERSION 3.24)

project(ExpenseBot VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Pdf)

qt_standard_project_setup()

qt_add_executable(expensebot
    WIN32
    src/main.cpp
    src/mainwindow.h
    src/mainwindow.cpp
)

target_link_libraries(expensebot
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Pdf
)
cmake_minimum_required(VERSION 3.24)

project(ExpenseBot VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Pdf Test)

qt_standard_project_setup()

add_executable(expensebot
    WIN32
    src/app/main.cpp
    src/app/mainwindow.h
    src/app/mainwindow.cpp
    src/domain/budgetformat.h
    src/domain/budgetformat.cpp
    src/domain/reportsession.h
    src/domain/reportsession.cpp
    src/domain/transaction.h
    src/io/pdftransactionreader.h
    src/io/pdftransactionreader.cpp
    src/io/reportexporter.h
    src/io/reportexporter.cpp
)

target_include_directories(expensebot
    PRIVATE
        src/app
        src/domain
        src/io
)

target_link_libraries(expensebot
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Pdf
)

enable_testing()

add_executable(expensebot_tests
    tests/expensebot_tests.cpp
    src/domain/budgetformat.h
    src/domain/budgetformat.cpp
    src/domain/reportsession.h
    src/domain/reportsession.cpp
    src/domain/transaction.h
    src/io/pdftransactionreader.h
    src/io/pdftransactionreader.cpp
    src/io/reportexporter.h
    src/io/reportexporter.cpp
)

target_include_directories(expensebot_tests
    PRIVATE
        src/domain
        src/io
)

target_link_libraries(expensebot_tests
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Pdf
        Qt6::Widgets
        Qt6::Test
)

add_test(NAME expensebot_tests COMMAND expensebot_tests)

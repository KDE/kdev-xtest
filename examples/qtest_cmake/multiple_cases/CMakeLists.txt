project(QTestCmakeExample)
cmake_minimum_required(VERSION 2.6)

find_package(Qt4 REQUIRED)
enable_testing()

include_directories(${QT_INCLUDES} ${CMAKE_BINARY_DIR})
set(moneytest_SRCS moneytest.cpp)
qt4_automoc(${moneytest_SRCS})
add_test(moneytest moneytest)
add_executable(moneytest ${moneytest_SRCS})
target_link_libraries(moneytest ${QT_QTTEST_LIBRARY})

set(banktest_SRCS banktest.cpp)
qt4_automoc(${banktest_SRCS})
add_test(banktest ${CMAKE_BINARY_DIR}banktest)
add_executable(banktest ${banktest_SRCS})
target_link_libraries(banktest ${QT_QTTEST_LIBRARY})

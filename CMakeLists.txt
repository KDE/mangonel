project(Mangonel)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(Mangonel_SRCS Mangonel.cpp Label.cpp Applications.cpp Paths.cpp Shell.cpp Calculator.cpp main.cpp)
qt4_automoc(${Mangonel_SRCS})
add_executable(mangonel ${Mangonel_SRCS})
target_link_libraries(mangonel plasma ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

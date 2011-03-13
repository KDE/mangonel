project(Mangonel)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)
find_package(KDE4 4.3 REQUIRED)
include (KDE4Defaults)


include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES})

set(Mangonel_SRCS Mangonel.cpp Label.cpp Config.cpp main.cpp
providers/Applications.cpp
providers/Paths.cpp
providers/Shell.cpp
providers/Calculator.cpp
providers/Units.cpp
providers/units/units.c
)
qt4_automoc(${Mangonel_SRCS})
add_executable(mangonel ${Mangonel_SRCS})
target_link_libraries(mangonel ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS mangonel
        DESTINATION "bin"
        PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
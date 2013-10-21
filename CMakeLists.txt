project(Mangonel)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)
find_package(KDE4 4.3 REQUIRED)
include(KDE4Defaults)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES})

set(Mangonel_SRCS
  Mangonel.cpp Label.cpp
  Config.cpp
  main.cpp
  providers/Applications.cpp
  providers/Paths.cpp
  providers/Shell.cpp
  providers/Calculator.cpp
  providers/Units.cpp
)

kde4_add_executable(mangonel ${Mangonel_SRCS})

target_link_libraries(mangonel
  ${KDE4_KDECORE_LIBS}
  ${KDE4_PLASMA_LIBS}
  ${KDE4_KIO_LIBS}
  ${KDE4_KUNITCONVERSION_LIBS}
  ${KDE4_KNOTIFYCONFIG_LIBRARY}
)

install(TARGETS mangonel ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES mangonel.notifyrc DESTINATION ${DATA_INSTALL_DIR}/mangonel)

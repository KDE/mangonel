cmake_minimum_required(VERSION 2.8.12)

project(Mangonel)

find_package (ECM REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include (KDEInstallDirs)
include (ECMInstallIcons)
include (FeatureSummary)

find_package (Qt5 CONFIG REQUIRED Core Widgets Quick)

find_package (KF5 REQUIRED COMPONENTS
    I18n
    GlobalAccel
    NotifyConfig
    Notifications
    DBusAddons
    UnitConversion
    Service
)

set(Mangonel_SRCS
  Mangonel.cpp
  Config.cpp
  main.cpp
  Provider.cpp
  IconProvider.cpp
  providers/Applications.cpp
  providers/Paths.cpp
  providers/Shell.cpp
  providers/Calculator.cpp
  providers/Units.cpp

  providers/calculator/constants.cpp
  providers/calculator/constants.h
  providers/calculator/evaluator.cpp
  providers/calculator/functions.cpp
  providers/calculator/numberformatter.cpp
  providers/calculator/opcode.cpp
  providers/calculator/settings.cpp

  providers/calculator/floatcommon.c
  providers/calculator/floatconst.c
  providers/calculator/floatconvert.c
  providers/calculator/floaterf.c
  providers/calculator/floatexp.c
  providers/calculator/floatgamma.c
  providers/calculator/floathmath.c
  providers/calculator/floatincgamma.c
  providers/calculator/floatio.c
  providers/calculator/floatipower.c
  providers/calculator/floatlog.c
  providers/calculator/floatlogic.c
  providers/calculator/floatlong.c
  providers/calculator/floatnum.c
  providers/calculator/floatpower.c
  providers/calculator/floatseries.c
  providers/calculator/floattrig.c
  providers/calculator/number.c
  providers/calculator/cmath.cpp
  providers/calculator/cnumberparser.cpp
  providers/calculator/hmath.cpp
  providers/calculator/quantity.cpp
  providers/calculator/rational.cpp
  providers/calculator/units.cpp
)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Wall")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")

set(CMAKE_AUTOMOC On)
qt5_add_resources(Mangonel_SRCS resources.qrc)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(mangonel ${Mangonel_SRCS})

target_link_libraries(mangonel
  Qt5::Core
  Qt5::Widgets
  Qt5::Quick
  KF5::GlobalAccel
  KF5::I18n
  KF5::DBusAddons
  KF5::NotifyConfig
  KF5::Notifications
  KF5::UnitConversion
  KF5::Service
)

set(DO_CLANG_TIDY "clang-tidy" "-checks=*,-clang-analyzer-alpha.*,-modernize-use-auto,-fuchsia-default-arguments,-hicpp-use-auto")

#set_target_properties(mangonel PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}")

install(TARGETS mangonel ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES mangonel.notifyrc DESTINATION ${KNOTIFYRC_INSTALL_DIR})
install(PROGRAMS mangonel.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

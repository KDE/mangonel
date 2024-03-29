cmake_minimum_required(VERSION 2.8.12)

project(Mangonel)

find_package (ECM REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include (KDEInstallDirs)
include (ECMInstallIcons)
include (FeatureSummary)

find_package (Qt5 CONFIG REQUIRED Core Widgets Quick)

find_package (KF5
    REQUIRED COMPONENTS
        I18n
        NotifyConfig
        Notifications
        DBusAddons
        UnitConversion
        Service
    OPTIONAL_COMPONENTS
        GlobalAccel

)
set(EXTRA_LIBRARIES)

if (KF5GlobalAccel_FOUND)
  set(EXTRA_LIBRARIES ${EXTRA_LIBRARIES} KF5::GlobalAccel)
  add_definitions(-DKGLOBALACCEL_FOUND)
endif()

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

  globalshortcut/qglobalshortcut.cpp
)

if(APPLE)
  set(Mangonel_SRCS ${Mangonel_SRCS}
    globalshortcut/qglobalshortcut_macx.cpp
  )
elseif(WIN32)
  set(Mangonel_SRCS ${Mangonel_SRCS}
    globalshortcut/qglobalshortcut_win.cpp
  )
else() # Just assume Linux
  set(Mangonel_SRCS ${Mangonel_SRCS}
    globalshortcut/qglobalshortcut_x11.cpp
  )
  find_package(Qt5X11Extras REQUIRED)
  find_package(X11 REQUIRED)
  find_package(XCB REQUIRED)
  set(EXTRA_LIBRARIES ${EXTRA_LIBRARIES} Qt5::X11Extras X11::X11 XCB::XCB)
endif()

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Wall")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")

set(CMAKE_AUTOMOC On)
qt5_add_resources(Mangonel_SRCS resources.qrc)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(mangonel ${Mangonel_SRCS})

target_link_libraries(mangonel
  ${EXTRA_LIBRARIES}
  Qt5::Core
  Qt5::Widgets
  Qt5::Quick
  KF5::I18n
  KF5::DBusAddons
  KF5::NotifyConfig
  KF5::Notifications
  KF5::UnitConversion
  KF5::Service
)

set(DO_CLANG_TIDY "clang-tidy" "-checks=*,-clang-analyzer-alpha.*,-modernize-use-auto,-fuchsia-default-arguments,-hicpp-use-auto")

#set_target_properties(mangonel PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}")

install(TARGETS mangonel ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES mangonel.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFY5RCDIR})
install(PROGRAMS mangonel.desktop DESTINATION ${KDE_INSTALL_AUTOSTARTDIR})

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

cmake_minimum_required(VERSION 2.8.12)

project(Mangonel)

find_package (ECM REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include (KDEInstallDirs)
include (KDECMakeSettings)
include (KDECompilerSettings)
include (ECMInstallIcons)
include (FeatureSummary)

find_package (Qt5 CONFIG REQUIRED Core Widgets)

include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

find_package (KF5 REQUIRED COMPONENTS
    I18n
    GlobalAccel
    KIO
    NotifyConfig
    Notifications
    DBusAddons
    WindowSystem
    XmlGui
    UnitConversion
)

set(Mangonel_SRCS
  Mangonel.cpp Label.cpp
  Config.cpp
  main.cpp
  Provider.cpp
  providers/Applications.cpp
  providers/Paths.cpp
  providers/Shell.cpp
  providers/Calculator.cpp
  providers/Units.cpp
)

add_executable(mangonel ${Mangonel_SRCS})

target_link_libraries(mangonel
  Qt5::Core
  Qt5::Widgets
  KF5::GlobalAccel
  KF5::KIOCore
  KF5::KIOWidgets
  KF5::I18n
  KF5::DBusAddons
  KF5::NotifyConfig
  KF5::Notifications
  KF5::WindowSystem
  KF5::XmlGui
  KF5::UnitConversion
)

install(TARGETS mangonel ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES mangonel.notifyrc DESTINATION ${KNOTIFYRC_INSTALL_DIR})
install(PROGRAMS mangonel.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})


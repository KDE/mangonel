cmake_minimum_required(VERSION 2.8.12)

project(Mangonel)

find_package (ECM REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include (KDEInstallDirs)
include (KDECMakeSettings)
include (KDECompilerSettings)
include (ECMInstallIcons)
include (FeatureSummary)

find_package (Qt5 CONFIG REQUIRED Core Widgets Quick)

find_package(FLEX REQUIRED)
set_package_properties(FLEX PROPERTIES DESCRIPTION "Flex is a tool for generating scanners, which recognizes lexical patterns"
                       URL "http://flex.sourceforge.net"
                       TYPE REQUIRED
                      )

find_package(BISON REQUIRED)
set_package_properties(BISON PROPERTIES DESCRIPTION "Bison is a general-purpose parser generator"
                       URL "http://www.gnu.org/software/bison"
                       TYPE REQUIRED
                      )



include_directories( ${CMAKE_CURRENT_BINARY_DIR} calc/ )

find_package (KF5 REQUIRED COMPONENTS
    I18n
    GlobalAccel
    KIO
    NotifyConfig
    Notifications
    DBusAddons
    WindowSystem
    UnitConversion
)

flex_target(CalcLexer calc/lexer.ll ${CMAKE_CURRENT_BINARY_DIR}/lexer_lex.cpp)
bison_target(CalcParser calc/parser.yy ${CMAKE_CURRENT_BINARY_DIR}/parser_yacc.cpp)

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
  calc/evaluator.cpp
  calc/node.cpp
  calc/numerictypes.cpp
  calc/hmath.cpp
  calc/number.c
  calc/result.cpp
  calc/function.cpp
  calc/valuemanager.cpp
  ${BISON_CalcParser_OUTPUTS}
  ${FLEX_CalcLexer_OUTPUTS}
)
qt5_add_resources(Mangonel_SRCS resources.qrc)

add_executable(mangonel ${Mangonel_SRCS})

target_link_libraries(mangonel
  Qt5::Core
  Qt5::Widgets
  Qt5::Quick
  KF5::GlobalAccel
  KF5::KIOCore
  KF5::KIOWidgets
  KF5::I18n
  KF5::DBusAddons
  KF5::NotifyConfig
  KF5::Notifications
  KF5::WindowSystem
  KF5::UnitConversion
)

install(TARGETS mangonel ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES mangonel.notifyrc DESTINATION ${KNOTIFYRC_INSTALL_DIR})
install(PROGRAMS mangonel.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})


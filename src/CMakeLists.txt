cmake_minimum_required(VERSION 3.26)

project(DataObjectEditorApp)

# Qt
find_package(
  Qt6
  COMPONENTS
  REQUIRED Core Gui Widgets Svg
)
set(QT_LIBRARIES Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Svg)

# Add definitions for command framework
add_definitions(-DUSE_COMMAND_FRAMEWORK)

set(PROJECT_FILES
    main.cpp
    MainWindow.cpp
    MainWindow.h
)

qt_add_executable(
  ${PROJECT_NAME} ${PROJECT_FILES}
  $<TARGET_OBJECTS:cafCommandFeatures> # Needed for cmake version < 3.12
)

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE
    cafUserInterface
    cafCommand
    custom-opm-common
    ${QT_LIBRARIES}
    ${EXTERNAL_LINK_LIBRARIES}
    Boost::filesystem
    Boost::regex
    Boost::system
)

# Copy Qt DLLs on Windows
foreach(qtlib ${QT_LIBRARIES})
  add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${qtlib}>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
  )
endforeach(qtlib)

# Install
install(
  TARGETS ${PROJECT_NAME}
  BUNDLE DESTINATION .
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
  TARGET ${PROJECT_NAME} OUTPUT_SCRIPT deploy_script
  NO_UNSUPPORTED_PLATFORM_ERROR NO_TRANSLATIONS
)
install(SCRIPT ${deploy_script})

cmake_minimum_required(VERSION 3.19)
project(HyperBotUploader LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets LinguistTools)

qt_standard_project_setup()

qt_add_executable(HyperBotUploader
    WIN32
    MACOSX_BUNDLE

    src/main.cpp
    src/MainWindow.cpp
    include/MainWindow.h

    src/ProjectManager.cpp
    include/ProjectManager.h

    include/FieldScene.h
    src/FieldScene.cpp

    include/FieldView.h
    src/FieldView.cpp

    include/FieldWindow.h
    src/FieldWindow.cpp

    include/RobotItem.h
    src/RobotItem.cpp

    include/UsbManager.h
    src/UsbManager.cpp
    
)
target_include_directories(HyperBotUploader
    PRIVATE
        include
)
qt_add_resources(HyperBotUploader "AppResources"
    PREFIX "/"
    FILES
        resources/overrideField.png
)
qt_add_translations(
    TARGETS HyperBotUploader
    TS_FILES HyperBotUploader_en_GB.ts
)

target_link_libraries(HyperBotUploader
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS HyperBotUploader
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET HyperBotUploader
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})

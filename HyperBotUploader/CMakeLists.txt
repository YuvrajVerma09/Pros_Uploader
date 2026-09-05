cmake_minimum_required(VERSION 3.19)
project(HyperBotUploader LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Gui
    Network
)

qt_standard_project_setup()

qt_add_executable(HyperBotUploader
    
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

    include/UploadButton.h
    src/UploadButton.cpp

    include/ProsUploader.h
    src/ProsUploader.cpp

    include/DependencyManager.h
    src/DependencyManager.cpp
    
)
target_include_directories(HyperBotUploader
    PRIVATE
        include
)
qt_add_resources(HyperBotUploader AppResources
    PREFIX "/"

    BASE resources

    FILES
        resources/overrideField.png
)
qt_add_translations(
    TARGETS HyperBotUploader
    TS_FILES HyperBotUploader_en_GB.ts
)

target_link_libraries(
    HyperBotUploader
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
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

set_target_properties(
    HyperBotUploader
    PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER
            "com.hyperbot.uploader"

        MACOSX_BUNDLE_BUNDLE_NAME
            "HyperBotUploader"

        MACOSX_BUNDLE_BUNDLE_VERSION
            "${PROJECT_VERSION}"

        MACOSX_BUNDLE_SHORT_VERSION_STRING
            "${PROJECT_VERSION}"
)

cmake_minimum_required(VERSION 3.21)

project(
    HyperBotUploader
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(
    Qt6 6.4 REQUIRED
    COMPONENTS
        Core
        Gui
        Widgets
        
)

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

    include/UploadButton.h
    src/UploadButton.cpp

    include/ProsUploader.h
    src/ProsUploader.cpp
    
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


target_link_libraries(HyperBotUploader PRIVATE
    Qt::Core
    Qt::Widgets
    
    Qt6::Gui
)

install(TARGETS HyperBotUploader
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET HyperBotUploader
    FILENAME_VARIABLE deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${deploy_script})
if(WIN32)
    target_sources(
        HyperBotUploader
        PRIVATE
            resources/windows.rc
    )
endif()

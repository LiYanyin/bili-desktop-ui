cmake_minimum_required(VERSION 3.22)
project(bili-desktop-ui VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Qt 6
set(CMAKE_PREFIX_PATH "$ENV{Qt6_DIR}" CACHE PATH "Qt6 install prefix")
find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(SOURCES
    src/main.cpp
    src/widgets/TitleBar.h
    src/widgets/TitleBar.cpp
    src/widgets/MainWindow.h
    src/widgets/MainWindow.cpp
    src/widgets/Sidebar.h
    src/widgets/Sidebar.cpp
    src/widgets/FlowLayout.h
    src/widgets/FlowLayout.cpp
    src/widgets/VideoData.h
    src/widgets/VideoCard.h
    src/widgets/VideoCard.cpp
    src/widgets/VideoDataLoader.h
    src/widgets/VideoDataLoader.cpp
)

add_executable(bili-desktop-ui ${SOURCES})

# Copy resources to build directory
add_custom_command(TARGET bili-desktop-ui POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/resources
    $<TARGET_FILE_DIR:bili-desktop-ui>/resources
)

target_include_directories(bili-desktop-ui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(bili-desktop-ui PRIVATE Qt6::Widgets)

# Windows-specific: link dwmapi for Mica effect
if(WIN32)
    target_link_libraries(bili-desktop-ui PRIVATE dwmapi)
endif()

cmake_minimum_required(VERSION 3.5)

project(Image_Processing VERSION 0.1 LANGUAGES CXX)
# add the eigen3 source path
set(EIGEN3_DIR F:/Coding/Libs/Eigen/src/eigen/)


set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
    # 或者只禁用这个警告（不推荐，但快速）
    # add_compile_options(/wd4819)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(QT_VERSION 6)

# find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Charts)
find_package(Qt${QT_VERSION} REQUIRED COMPONENTS Widgets Charts)

# my append
find_package(OpenCV REQUIRED)

# debug info
message(STATUS "OpenCV found: ${OpenCV_FOUND}")
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")



file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/src/*.ui"
)

message(STATUS "=== Collected cpp files ===")
foreach(file ${SOURCES})
    if(file MATCHES "\\.cpp$")
        message(STATUS "  ${file}")
    endif()
endforeach()
file(GLOB_RECURSE HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/inc/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/inc/*.hpp"
)
# set(RESOURCES resources/resource.qrc)

set(PROJECT_SOURCES
    ${HEADERS}
    ${SOURCES}
)

# set(PROJECT_SOURCES
#     ./Source/inc/*.h
#     ./Source/src/*.cpp
#     ./Resource/*.ui
#         # main.cpp
#         # mainwindow.cpp
#         # mainwindow.h
#         # mainwindow.ui

#         # sys.h  # 总头文件

#         # # 图像处理中间件
#         # imageprocesser.cpp
#         # imageprocesser.h

#         # # 页面UI
#         # pagebase.h
#         # pagebase.cpp
#         # pageimagefilter.h
#         # pageimagefilter.cpp
#         # pagehistogramandthreshold.h
#         # pagehistogramandthreshold.cpp
#         # pagebinaryoperation.h pagebinaryoperation.cpp
#         # pagebinaryfunction.cpp pagebinaryfunction.h
#         # pagegrayfunction.cpp pagegrayoperation.cpp
#         # pagegrayoperation.h pagegrayfunction.h

# )


if(${QT_VERSION} GREATER_EQUAL 6)
    qt_add_executable(Image_Processing
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        # ${SOURCES}
        # ${HEADERS}

    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Image_Processing APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Image_Processing SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Image_Processing
            ${PROJECT_SOURCES}
            imageprocesser.h imageprocesser.cpp
            pagebinaryoperation.h pagebinaryoperation.cpp
            pagebinaryfunction.h pagebinaryfunction.cpp
            pagegrayoperation.h pagegrayoperation.cpp
            pagegrayfunction.h pagegrayfunction.cpp
        )
    endif()
endif()
message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")
# include_directories(${EIGEN3_DIR})

target_include_directories(Image_Processing PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/inc
    ${EIGEN3_DIR}

)

target_link_libraries(Image_Processing PRIVATE
    ${OpenCV_LIBS}

    Qt${QT_VERSION}::Widgets
    Qt${QT_VERSION}::Charts   # 添加上这一行
    # opencv_core
    # opencv_imgcodecs
    # opencv_imgproc
    # ${OpenCV_LIBS}
)

set_target_properties(Image_Processing PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS Image_Processing
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION EQUAL 6)
    qt_finalize_executable(Image_Processing)
endif()

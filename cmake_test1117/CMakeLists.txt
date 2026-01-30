cmake_minimum_required(VERSION 3.5)

project(cmake_test1117 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # zhw:1120

# 添加 MSVC 特定的编译器选项
# if(MSVC)
#     add_compile_options(/Zc:__cplusplus)
# endif()
find_package(Threads REQUIRED)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt6  COMPONENTS  SerialPort   REQUIRED)
find_package(Qt6  COMPONENTS  PrintSupport  REQUIRED)
find_package(Qt6  COMPONENTS  pdf  REQUIRED)
find_package(Qt6  COMPONENTS  PdfWidgets   REQUIRED)
find_package(Qt6  COMPONENTS  Concurrent     REQUIRED)

#set(OpenCV_DIR "D:/Program Files (x86)/opencv/opencv/build/x64/vc16/lib") #HC
set(OpenCV_DIR "D:/Qt/opencv/build/x64/vc15/lib") #HC

#set(OpenCV_DIR "C:/opencv/build/x64/vc16/lib") # zhw
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

set(PROJECT_SOURCES

    mainwindow.ui


    block.cpp
    datafilterworker.cpp
    globaldata.cpp
    globalfunctions.cpp
    gradientpalette.cpp
    heartmap.cpp
    main.cpp
    mainwindow.cpp
    mapper_40.cpp
    myserialport.cpp
    preprocess.cpp
    utils.cpp
    process.cpp

    block.h
    commondata.h
    datafilterworker.h
    filesaveworker.h
    global.h
    globaldata.h
    globalfunctions.h
    gradientpalette.h
    heartmap.h
    mainwindow.h
    mapper_40.h
    myserialport.h
    preprocess.h
    utils.h
    process.h
    form_blockdata.cpp
    form_blockdata.h
    form_blockdata.ui

    form_processdata.cpp
    form_processdata.h
    form_processdata.ui

    ellipsewidget.cpp
    ellipsewidget.h
    serial_port_thread.cpp
    serial_port_thread.h
    log.cpp
    log.h
    showreport.cpp
    showreport.h
    showreport.ui

    coordinatewidget.cpp
    coordinatewidget.h
    include.qrc
    jingtai.cpp
    jingtai.h
    tuoyuan.cpp
    tuoyuan.h
    data_structer.h
    dongtai.cpp
    dongtai.h
    frame.cpp
    frame.h
zugong.cpp
zugong.h



)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(cmake_test1117
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET cmake_test1117 APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(cmake_test1117 SHARED
            ${PROJECT_SOURCES}
        )
    # Define properties for Android with Qt 5 after find_package() calls as:
    #    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
else()
    add_executable(cmake_test1117
        ${PROJECT_SOURCES}
    )
endif()
endif()

target_link_libraries(cmake_test1117 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${OpenCV_LIBS})
target_link_libraries(cmake_test1117 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(cmake_test1117 PRIVATE Qt6::SerialPort)
target_link_libraries(cmake_test1117 PRIVATE Qt6::PrintSupport)
target_link_libraries(cmake_test1117 PRIVATE Qt6::Pdf)
target_link_libraries(cmake_test1117 PUBLIC Qt6::PdfWidgets)
target_link_libraries(cmake_test1117 PRIVATE Qt6::Concurrent )

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.cmake_test1117)
endif()
set_target_properties(cmake_test1117 PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS cmake_test1117
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}    # 静态库路径
)
# 安装额外资源文件
install(FILES
    mainwindow.ui
    form_blockdata.ui
    form_processdata.ui
    DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake_test1117/ui
)
# 设置默认安装路径
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Install path" FORCE)
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(cmake_test1117)
endif()

cmake_minimum_required(VERSION 3.1)

#set project name and language
project(vf-customerdata LANGUAGES CXX)


#Automoc and RCC ON. Needed in QT projects
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
#C++ 11 Flag# spawn out some info on configuration
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Set project Version
set(VfCustomerdata_VERSION_MAJOR "0")
set(VfCustomerdata_VERSION_MINOR "0")
set(VfCustomerdata_VERSION_PATCH "1")

set(PROJECT_VERSION "${VfCustomerdata_VERSION_MAJOR}.${VfCustomerdata_VERSION_MINOR}.${VfCustomerdata_VERSION_PATCH}")


#Adding cmake functions like feature_summary()
include(FeatureSummary)
include(GNUInstallDirs)

#Find dependecies
find_package(Qt5 COMPONENTS Core Gui Concurrent  CONFIG REQUIRED)
find_package(VfComponent REQUIRED)
find_package(VfHelpers REQUIRED)
find_package(VfEvent REQUIRED)
find_package(VfCpp REQUIRED)


#sum up project Files 
file(GLOB SOURCES 
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    )

file(GLOB HEADER
    ${PROJECT_SOURCE_DIR}/src/*.h
    )

file(GLOB RESOURCES 
    ${PROJECT_SOURCE_DIR}/*.qrc
    )

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
#create library 
add_library(VfCustomerdata SHARED
    ${SOURCES}
    ${HEADER}
    ${RESOURCES}
    )




#link dependencies
target_link_libraries(VfCustomerdata 
    PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Concurrent
    VeinMeta::VfHelpers
    VeinMeta::VfEvent
    VeinMeta::VfComponent
    VeinMeta::VfCpp
    )


#set target Version
set_target_properties(VfCustomerdata PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(VfCustomerdata PROPERTIES SOVERSION ${VfCustomerdata_VERSION_MAJOR})

#generate export header
include(GenerateExportHeader)
generate_export_header(VfCustomerdata)


#install export header
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/vfcustomerdata_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/VfCustomerdata
    )

# announce headers - target perspective
target_include_directories(VfCustomerdata
    PUBLIC
    $<INSTALL_INTERFACE:include/VfCustomerdata>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

# install library
install(TARGETS VfCustomerdata
    EXPORT VfCustomerdataExport
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

# configure *.cmake File
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    VfCustomerdataConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY SameMinorVersion
    )

# configure *.cmake File 
configure_file(VfCustomerdataConfig.cmake.in VfCustomerdataConfig.cmake @ONLY)


# install *(Version).cmake
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/VfCustomerdataConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/VfCustomerdataConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VfCustomerdata
    )

# install targets cmake-files
install(EXPORT VfCustomerdataExport
    FILE VfCustomerdataTargets.cmake
    NAMESPACE VeinMeta::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VfCustomerdata
    )

# install public headers
install(
    FILES ${HEADER}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/VfCustomerdata
    )

# spawn out some info on configuration
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)



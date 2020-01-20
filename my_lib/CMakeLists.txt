file(GLOB_RECURSE SrcFiles *.cpp)

if (BUILD_SHARED_LIBS)
    add_library(my_lib SHARED ${SrcFiles})
    target_compile_definitions(my_lib PUBLIC -DMY_LIB_SHARED_BUILD)
    target_compile_definitions(my_lib PRIVATE -DMY_LIB_EXPORTS)
else()
    add_library(my_lib STATIC ${SrcFiles})
endif()
#Add an alias so that library can be used inside the build tree, e.g. when testing
add_library(my_lib::my_lib ALIAS my_lib)
target_include_directories(my_lib
    PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
set_target_properties(my_lib PROPERTIES 
    CXX_STANDARD 14
    CMAKE_CXX_STANDARD_REQUIRED True
)

#install settings
install(TARGETS my_lib
EXPORT my_libTargets
RUNTIME DESTINATION "bin"
LIBRARY DESTINATION "lib"
ARCHIVE DESTINATION "lib"
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION "include"
        FILES_MATCHING PATTERN "*.h")
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION "include"
        FILES_MATCHING PATTERN "*.hpp")

install(EXPORT my_libTargets
    FILE my_libTargets.cmake
    NAMESPACE my_lib::
    DESTINATION lib/cmake/my_lib
)

include(CMakePackageConfigHelpers)
# generate the config file that is includes the exports
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/my_libConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/my_libConfig.cmake"
    INSTALL_DESTINATION "lib/cmake/my_lib"
)
# generate the version file for the config file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/my_libConfigVersion.cmake"
    VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/my_libConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/my_libConfigVersion.cmake
    DESTINATION lib/cmake/my_lib
)

export(EXPORT my_libTargets FILE ${CMAKE_CURRENT_BINARY_DIR}/my_libTargets.cmake NAMESPACE my_lib::)

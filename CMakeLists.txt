cmake_minimum_required(VERSION 4.1)

include(FetchContent)
FetchContent_Declare(my-vcpkg-triplets
		GIT_REPOSITORY "https://github.com/Zinn-o-Matics/my-vcpkg-triplets.git"
		GIT_TAG "master"
)
FetchContent_MakeAvailable(my-vcpkg-triplets)

list(APPEND VCPKG_OVERLAY_TRIPLETS "${my-vcpkg-triplets_SOURCE_DIR}")

project(modernIni CXX)

option(BUILD_TESTS "Build the GTest executable" OFF)
option(MAGIC_ENUM "Use magic-enum for enum names" OFF)

add_library(${PROJECT_NAME} STATIC)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# Use -MT / -MTd runtime library
set_property(TARGET ${PROJECT_NAME} PROPERTY
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_include_directories(
		${PROJECT_NAME}
		INTERFACE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
		$<INSTALL_INTERFACE:include>
)

target_compile_definitions(
		${PROJECT_NAME}
		PUBLIC
		NOMINMAX
)

# add general sources
target_sources(${PROJECT_NAME}
		PUBLIC
		FILE_SET HEADERS
		FILES
		modernIni.h
		modernIniMacros.h
)

target_sources(${PROJECT_NAME}
		PRIVATE
		modernIni.cpp
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

if (MAGIC_ENUM)
	find_package(magic_enum CONFIG REQUIRED)
	target_compile_definitions(${PROJECT_NAME} PRIVATE MAGIC_ENUM)
	target_link_libraries(${PROJECT_NAME} PUBLIC magic_enum::magic_enum)
endif ()

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
		EXPORT modernIniTargets
		FILE_SET HEADERS
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)

include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
		${CMAKE_CURRENT_BINARY_DIR}/modernIniConfig.cmake
		INSTALL_DESTINATION share/${PROJECT_NAME}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/modernIniConfig.cmake
		DESTINATION share/${PROJECT_NAME}
)

install(EXPORT modernIniTargets
		NAMESPACE ${PROJECT_NAME}::
		FILE modernIniTargets.cmake
		DESTINATION share/${PROJECT_NAME}
)

if (BUILD_TESTS)
	target_compile_definitions(${PROJECT_NAME} PRIVATE TESTS)
	target_link_libraries(${PROJECT_NAME} PRIVATE GTest::gtest)

	find_package(GTest CONFIG REQUIRED)
	include(GoogleTest)
	add_executable(
			${PROJECT_NAME}Tests
			modernIniTestAccessor.h
			modernIniParsingTests.cpp
			modernIniSerializationTests.cpp
			modernIniGetTests.cpp
			modernIniSetTests.cpp
			modernIniTypeTests.cpp
	)
	target_compile_features(${PROJECT_NAME}Tests PUBLIC cxx_std_23)
	target_compile_definitions(${PROJECT_NAME}Tests PRIVATE TESTS)

	# Use -MT / -MTd runtime library
	set_property(TARGET ${PROJECT_NAME}Tests PROPERTY
			MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

	target_link_libraries(${PROJECT_NAME}Tests PRIVATE modernIni::modernIni GTest::gtest GTest::gtest_main)

	gtest_discover_tests(${PROJECT_NAME}Tests)

	target_compile_definitions(${PROJECT_NAME}Tests PUBLIC TEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/test/\")

endif ()

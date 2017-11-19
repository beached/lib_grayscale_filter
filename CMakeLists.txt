cmake_minimum_required( VERSION 3.2 )

project( lib_grayscale_filter_prj )

include( ExternalProject )
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR} )

set( Boost_USE_STATIC_LIBS OFF )
set( Boost_USE_MULTITHREADED ON )
set( Boost_USE_STATIC_RUNTIME OFF )

if(DAWFILTER_USEPYTHON)
	find_package( Boost 1.58.0 REQUIRED COMPONENTS system filesystem regex unit_test_framework program_options iostreams python )
else( )
	find_package( Boost 1.58.0 REQUIRED COMPONENTS system filesystem regex unit_test_framework program_options iostreams )
endif( )

find_package( FreeImage REQUIRED )
enable_testing( )

if( ${CMAKE_CXX_COMPILER_ID} STREQUAL 'MSVC' )
	add_compile_options( -D_WIN32_WINNT=0x0601 /std:c++latest )
else( )
	if( ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang" )
		add_compile_options(-std=c++14 -march=native -Weverything -Wno-c++98-compat -Wno-covered-switch-default -Wno-padded -Wno-exit-time-destructors -Wno-c++98-compat-pedantic -Wno-unused-parameter -Wno-missing-noreturn -Wno-missing-prototypes -Wno-disabled-macro-expansion)
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
	elseif( ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" )
		add_compile_options(-std=c++14 -march=native -Wall -Wno-deprecated-declarations -Wduplicated-cond -Wlogical-op -Wnull-dereference -Wold-style-cast -Wshadow)
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
	endif( )
endif( )

externalproject_add(
	header_libraries_prj
	GIT_REPOSITORY "https://github.com/beached/header_libraries.git"
	GIT_TAG "master"
	SOURCE_DIR "${CMAKE_BINARY_DIR}/dependencies/header_libraries"
	INSTALL_DIR "${CMAKE_BINARY_DIR}/install"
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install
	PATCH_COMMAND ""
	TEST_COMMAND ""
)

include_directories( SYSTEM "${CMAKE_BINARY_DIR}/install/include" )
link_directories( "${CMAKE_BINARY_DIR}/install/lib" )
include_directories( "./include" )

include_directories( SYSTEM ${Boost_INCLUDE_DIRS} )
include_directories( SYSTEM ${FREEIMAGE_INCLUDE_DIRS} )
link_directories( ${FREEIMAGE_LIBRARIES} )

set( HEADER_FOLDER "include" )
set( SOURCE_FOLDER "src" )
set( TEST_FOLDER "tests" )

SET( HEADER_FILES
	${HEADER_FOLDER}/filterdawgscolourize.h
	${HEADER_FOLDER}/filterdawgs.h
	${HEADER_FOLDER}/filterdawgs2.h
	${HEADER_FOLDER}/filterrotate.h
	${HEADER_FOLDER}/fimage.h
	${HEADER_FOLDER}/genericimage.h
	${HEADER_FOLDER}/genericrgb.h
	${HEADER_FOLDER}/helpers.h
)

set( SOURCE_FILES
	${SOURCE_FOLDER}/filterdawgs2.cpp
	${SOURCE_FOLDER}/filterdawgscolourize.cpp
	${SOURCE_FOLDER}/filterdawgs.cpp
	${SOURCE_FOLDER}/filterrotate.cpp
	${SOURCE_FOLDER}/fimage.cpp
	${SOURCE_FOLDER}/genericimage.cpp
)

add_library( grayscale_filter ${HEADER_FILES} ${SOURCE_FILES} )
add_dependencies( grayscale_filter header_libraries_prj )
target_link_libraries( grayscale_filter ${Boost_LIBRARIES} ${FREEIMAGE_LIBRARIES} )

add_custom_target( check COMMAND ${CMAKE_CTEST_COMMAND} )

add_executable( image_in_out_test_bin EXCLUDE_FROM_ALL ${FUNCTION_STREAM_HEADER_FILES} ${TASK_SCHEDULER_HEADER_FILES} ${TEST_FOLDER}/image_in_out_test.cpp )
target_link_libraries( image_in_out_test_bin grayscale_filter ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} )
add_dependencies( image_in_out_test_bin grayscale_filter header_libraries_prj )
add_test( image_in_out_test image_in_out_test_bin "${PROJECT_SOURCE_DIR}/img_in_001.jpg" "${CMAKE_BINARY_DIR}/img_out_001.jpg" )
add_dependencies( check image_in_out_test_bin )

add_executable( filter_speed_test_bin EXCLUDE_FROM_ALL ${FUNCTION_STREAM_HEADER_FILES} ${TASK_SCHEDULER_HEADER_FILES} ${TEST_FOLDER}/filter_speed_test.cpp )
target_link_libraries( filter_speed_test_bin grayscale_filter ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} )
add_dependencies( filter_speed_test_bin grayscale_filter header_libraries_prj )
add_test( filter_speed_test filter_speed_test_bin "${PROJECT_SOURCE_DIR}/img_in_001.jpg" "${CMAKE_BINARY_DIR}/img_out_001.jpg" )
add_dependencies( check filter_speed_test_bin )

install( TARGETS grayscale_filter DESTINATION lib )
install( DIRECTORY ${HEADER_FOLDER}/ DESTINATION include/daw/grayscale_filter )



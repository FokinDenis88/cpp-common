# CMakeList.txt : CMake project for artificial-neural-networks, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.8)

# Can be different with target
set(PROJECT_NAME cpp-utility)
project(${PROJECT_NAME} VERSION 1.0.0)
set(TARGET_NAME ${PROJECT_NAME})


#============================Google Test========================================
# http://google.github.io/googletest/quickstart-cmake.html
# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG        origin/main		# Branch name, tag name or tag hash
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
set(GTEST_INCLUDE_DIR ${gtest_SOURCE_DIR}/include)
#=================================================================================


set(ALL_HEADERS_FILTER
    include/all-headers.hpp

    # https://en.cppreference.com/w/cpp/headers.html
    # algorithms-library

    # concepts-library

    # concurrency-support-library
    include/concurrency-support-library/multithreading.hpp
    include/concurrency-support-library/thread.hpp

    # containers-library
    include/containers-library/generic-container.hpp

    # diagnostics-library
    include/diagnostics-library/custom-exception.hpp
    include/diagnostics-library/error.hpp

    # general-utilities-library
        # functional
        include/general-utilities-library/functional/functional.hpp
        include/general-utilities-library/functional/weak-method-invoker.hpp

    include/general-utilities-library/utility.hpp
    include/general-utilities-library/iaction.hpp
    include/general-utilities-library/ieditor.hpp
    include/general-utilities-library/interface-macros.hpp
    include/general-utilities-library/tuple.hpp


    # input-output-library

    # iterators-library

    # language-support-library

    # memory-management-library
        # weak-ptr
        include/memory-management-library/weak-ptr/weak-ptr.hpp

    include/memory-management-library/generic-smart-ptr.hpp


    # metaprogramming-library
        # type-traits
        include/metaprogramming-library/type-traits/type-traits.hpp


    # multi-purpose-headers

    # numerics-library

    # ranges-library

    # strings-library

    # text-processing-library

    # time-library
    include/time-library/current-date.hpp
    include/time-library/date-time.hpp
	)


set(SOURCES
	src/cpp-utility.cpp
    src/concurrency/multithread-for-loop.cpp
	)


set(DOC_FILTER
    README.md
    CHANGELOG.md
    .gitlab-ci.yml
    doc/issues/to-do.txt
	)


#find_package(OpenMP REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
# RUNTIME_OUTPUT_DIRECTORY and RUNTIME_OUTPUT_NAME

# Include Directories
set(INCLUDES
	include
	${GTEST_INCLUDE_DIR}	# for "gtest/gtest.h"
)

# Include Directories
#target_include_directories(${TARGET_NAME} PUBLIC ${INCLUDES})
include_directories(PUBLIC ${INCLUDES})
#target_link_libraries(${TARGET_NAME} PRIVATE OpenMP::OpenMP_CXX)


# Add source to this project's executable.
add_executable(${TARGET_NAME} ${SOURCES} ${ALL_HEADERS_FILTER} ${DOC_FILTER})

#target_link_libraries(${TARGET_NAME} PRIVATE OpenMP::OpenMP_CXX)
#add_custom_command()


# Filters for Grouping files in project
# Headers
source_group("Headers" FILES ${ALL_HEADERS_FILTER})

# Sources
source_group("Source" FILES ${SOURCES})

# Other groups
source_group("Doc" FILES ${DOC_FILTER})


SET(ECLIPSE_GCC_STATIC_ANALYSIS -g -pedantic -Wall -Wextra -Wconversion -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wswitch-default -Wundef -Weffc++ -Wfloat-equal -c -fmessage-length=0)
set(ALL_STATIC_ANALYSIS_OPTIONS -std=c++2a -Wall -Wextra -pedantic -fanalyzer)
if(LINUX)
	#-std=c++2a -I/media/sf_Projects/IT/Programming/!git-web/private/design-patterns/include -O0 -g -pedantic -Wall -Wextra -Wconversion -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wswitch-default -Wundef -Weffc++ -Wfloat-equal -c -fmessage-length=0
	target_compile_options(${TARGET_NAME} PUBLIC ${ALL_STATIC_ANALYSIS_OPTIONS} ${ECLIPSE_GCC_STATIC_ANALYSIS})
elseif(MSVC)

endif()


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 23)
  set_property(TARGET ${TARGET_NAME} PROPERTY C_STANDARD 17)
endif()
set_property(TARGET ${TARGET_NAME} PROPERTY SUBSYSTEM CONSOLE)



#============================Google Test========================================
# Source code, that will be tested
set(TARGET_TESTABLE_CODE
    # mustn't be file with main function
    src/concurrency/multithread-for-loop.cpp
)
# Source code of tests
set(SOURCES_FILTER_TESTS
	test/utility/utility-test.cpp
)

# Library with code of work project for linking to test project
add_library(testable_code_library OBJECT ${TARGET_TESTABLE_CODE})
source_group("Tests" FILES ${SOURCES_FILTER_TESTS})

# Enable Google Tests
set(TEST_TARGET_NAME ${TARGET_NAME}-test)
enable_testing()

add_executable(
  ${TEST_TARGET_NAME}
  ${SOURCES_FILTER_TESTS})

target_link_libraries(
  ${TEST_TARGET_NAME}
  PUBLIC testable_code_library
  GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(${TEST_TARGET_NAME})
#================================================================================



#=========================Web Links=============================
# GCC Compiler
# https://gcc.gnu.org/onlinedocs/gcc-6.2.0/gcc/Invoking-GCC.html
# https://www.rapidtables.com/code/linux/gcc.html

# Static Analysis Options:
# -std=c++2a -std=c++1z -Wall -Wextra -fanalyzer

# Eclipse gcc Static Analysis with all warnings
#-std=c++2a -I/media/sf_Projects/IT/Programming/!git-web/private/design-patterns/include -O0 -g -pedantic -Wall -Wextra -Wconversion -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wswitch-default -Wundef -Weffc++ -Wfloat-equal -c -fmessage-length=0



# Help Information
# https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html
# https://cmake.org/cmake/help/latest/manual/cmake-language.7.html#cmake-language-variables


#set(srcs a.c b.c c.c) # sets "srcs" to "a.c;b.c;c.c"

#add_executable(main ${SOURCE_EXE})	# Создает исполняемый файл с именем main
#add_library (Hello STATIC hello.cxx)
#target_include_directories (Hello PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
#add_executable (helloDemo demo.cxx demo_b.cxx)
#target_link_libraries (helloDemo LINK_PUBLIC Hello)
#target_compile_options(first-test PRIVATE -fexceptions)
#target_link_options()

#set(SOURCE_EXE main.cpp)   set(SOURCE_LIB foo.cpp)
#add_library(foo STATIC ${SOURCE_LIB})	# Создание статической библиотеки с именем foo
#add_executable(main ${SOURCE_EXE})	# Создает исполняемый файл с именем main
#target_link_libraries(main foo)
#include_directories(foo)			# Расположение заголовочных файлов
#file(GLOB <variable> [LIST_DIRECTORIES true|false] [RELATIVE <path>] [CONFIGURE_DEPENDS] [<globbing-expressions>...])
#target_compile_options(first-test PRIVATE -fexceptions)

#find_package(OpenMP)
#if (OPENMP_FOUND)
    #set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    #set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    #set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
#endif()

#set_property(TARGET artificial-neural-networks PROPERTY SUBSYSTEM WINDOWS)
#CMAKE_CXX_FLAGS    /SUBSYSTEM:CONSOLE

#source_group("Header Files" FILES ${INCLUDES} REGULAR_EXPRESSION *)

#OpenMP
#find_package(OpenMP REQUIRED)
#target_link_libraries(artificial-neural-networks PRIVATE OpenMP::OpenMP_CXX)

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Добавляет директории для дальнейшей обработки. В них нужно добавлять еще одни CMakeLists.txt файлы
# add_subdirectory (Hello)



#message("generator is set to ${CMAKE_GENERATOR}")
#set(CMAKE_GENERATOR "Ninja" CACHE INTERNAL "" FORCE)

#Eclipse CDT4 - Ninja
#Eclipse CDT4 - Unix Makefiles


#if (WIN32)
    # do something
#endif (WIN32)

#if (UNIX)
    # do something
#endif (UNIX)

#if (MSVC)
    # do something
#endif (MSVC)

#if(UNIX AND NOT APPLE)
    #set(LINUX TRUE)
#endif()

# if(NOT LINUX) should work, too, if you need that
#if(LINUX)
    #message(STATUS ">>> Linux")
    # linux stuff here
#else()
    #message(STATUS ">>> Not Linux")
    # stuff that should happen not on Linux
#endif()

# Permanent verbose command output from the generated Makefiles
#cmake -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON .









#=============================================================================
# https://gcc.gnu.org/onlinedocs/gcc-6.2.0/gcc/Invoking-GCC.html
# https://www.rapidtables.com/code/linux/gcc.html
# -c				: compile source files to object files without linking
# -E				: Preprocess only; do not compile, assemble or link
# -glevel			: generate debug information to be used by GDB
# -Idir 			: include directory of header files
# -llib				: link with library file
# -Ldir 			: look in directory for library files
# -o output file 	: write build output to output file
# -Olevel			: optimize for code size and execution tim
# -time 			: Time the execution of each subprocess
# __version			: Display compiler version information
# https://gcc.gnu.org/onlinedocs/gcc-6.2.0/gcc/Warning-Options.html#Warning-Options
# -Wall				:
# -Wextra			:
# -fanalyzer		:
# -std=c++2a		:
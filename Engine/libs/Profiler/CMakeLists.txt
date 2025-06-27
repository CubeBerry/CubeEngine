CMAKE_MINIMUM_REQUIRED(VERSION 3.16 FATAL_ERROR)
PROJECT(Profiler)

# NOTE: On Windows and Cygwin, the dll's are placed in the
# CMAKE_RUNTIME_OUTPUT_DIRECTORY, while their corresponding import
# libraries end up in CMAKE_ARCHIVE_OUTPUT_DIRECTORY. On other
# platforms, such as Linux, the shared libraries are put in
# CMAKE_ARCHIVE_OUTPUT_DIRECTORY instead.
# Static libraries end up in CMAKE_ARCHIVE_OUTPUT_DIRECTORY on all
# platforms.
# SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
# SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
# SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

OPTION(PROFILER_BUILD_SHARED_LIBS "Build Profiler shared library." OFF)
OPTION(PROFILER_BUILD_STATIC_LIBS "Build Profiler static library." OFF)

# if (PROFILER_BUILD_SHARED_LIBS)
#     set(PROFILER_BUILD_STATIC_LIBS OFF)
# endif()

# option to also copy .pdb files to install directory when executing
# INSTALL target
IF(MSVC)
    OPTION(INSTALL_PDB "Also install .pdb files" ON)
ELSE()
    SET(INSTALL_PDB OFF)
ENDIF()

FILE(GLOB PROFILER_SRCS ${PROFILER_SRCS} "src/include/*.h" "src/*.h" "src/*.cpp" "src/*.hpp")

IF(WIN32)
    FILE(GLOB PROFILER_SRCS ${PROFILER_SRCS} "src/mswin/*.h" "src/mswin/*.cpp" "src/mswin/*.hpp" "src/mswin/*.asm")
ELSEIF(ANDROID)
    FILE(GLOB PROFILER_SRCS ${PROFILER_SRCS} "src/android/*.h" "src/android/*.cpp" "src/android/*.hpp" "src/android/*.s")
ELSE()
    FILE(GLOB PROFILER_SRCS ${PROFILER_SRCS} "src/unix/*.h" "src/unix/*.cpp" "src/unix/*.hpp" "src/unix/*.s")
ENDIF()

IF(WIN32)
    FILE(GLOB ASM_SRCS ${ASM_SRCS} "src/*.asm" "*.asm" "src/mswin/*.asm" "src/msvc/*.asm")
ELSEIF(ANDROID)
    FILE(GLOB ASM_SRCS ${ASM_SRCS} "src/*.s" "src/android/*.s")
ELSE()
    FILE(GLOB ASM_SRCS ${ASM_SRCS} "src/*.s" "src/unix/*.s" "src/linux/*.s")
ENDIF()

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/include)
IF(WIN32)
    INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/mswin)
ELSEIF(ANDROID)
    INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/android)
ELSE()
    INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/unix)
ENDIF()

# lib m for math, not needed on windows
IF (NOT WIN32)
    # For compilation:
    LIST(APPEND LIBS m)
    # For CHECK_FUNCTION_EXISTS:
    LIST(APPEND CMAKE_REQUIRED_LIBRARIES m)
ENDIF()

IF(WIN32)
    # hide insecure CRT warnings, common practice
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
    IF(MSVC)
        SET( CMAKE_DEBUG_POSTFIX "d" )
    ENDIF(MSVC)    
ENDIF()

IF(CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
  IF(!ANDROID)
    # not setting -ansi as EGL/KHR headers doesn't support it
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi")
  ENDIF()
ENDIF(CMAKE_COMPILER_IS_GNUCC)

IF(ANDROID)
    # -landroid for ANativeWindow
    # -llog for native Android logging
    LIST(APPEND LIBS android log)
ENDIF()

IF(${CMAKE_SYSTEM_NAME} MATCHES "Android" OR
   ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" OR
   ${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR
   ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
STRING(TOLOWER -${CMAKE_SYSTEM_NAME} SYSTEM_NAME_LOWERCASE)
ELSE()
SET(SYSTEM_NAME_LOWERCASE "-other")
ENDIF()
FILE(WRITE ${CMAKE_BINARY_DIR}/config.h "
#if defined(_MSC_VER) || defined(__WATCOMC__) || defined(__MINGW32__) \\
    || defined(_WIN32) || defined(_WIN32_WCE) || defined(__CYGWIN__)
#    include \"config-windows.h\"
#elif defined(__ANDROID__)
#    include \"config-android.h\"
#elif defined(__APPLE__)
#    include \"config-darwin.h\"
#elif defined(__linux__)
#    include \"config-linux.h\"
#else
#    include \"config-other.h\"
#endif
")

# IF(PROFILER_BUILD_SHARED_LIBS)
#     set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
#     ADD_LIBRARY(profiler SHARED ${PROFILER_SRCS} ${ASM_SRCS} )
#     target_include_directories(profiler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/include)
# ENDIF()
# IF(PROFILER_BUILD_STATIC_LIBS)
    ADD_LIBRARY(profiler_static STATIC ${PROFILER_SRCS} ${ASM_SRCS} ) 
    target_include_directories(profiler_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/include)
# ENDIF()

if(MSVC)
  enable_language(ASM_MASM)
  set_source_files_properties(${ASM_SRCS} PROPERTIES LANGUAGE ASM_MASM)
endif(MSVC)

# if (FORCE_DBGHELP)
if(MSVC)
#   IF(PROFILER_BUILD_SHARED_LIBS)
#     target_link_libraries(profiler dbghelp)
#   ENDIF()
#   IF(PROFILER_BUILD_STATIC_LIBS)
    target_link_libraries(profiler_static dbghelp)
#   ENDIF()
elseif(UNIX)
    find_package(Unwind)
    if(UNWIND_FOUND)
    # IF(PROFILER_BUILD_SHARED_LIBS)
    #     target_link_libraries(profiler ${UNWIND_LIBRARIES})
    # ENDIF()
    # IF(PROFILER_BUILD_STATIC_LIBS)
        target_link_libraries(profiler_static ${UNWIND_LIBRARIES})
    # ENDIF()
    endif(UNWIND_FOUND)
endif(MSVC)
# endif (FORCE_DBGHELP)

IF(WIN32)
    LIST(APPEND LIBS winmm)
    # IF(PROFILER_BUILD_SHARED_LIBS)
    #     SET_TARGET_PROPERTIES(profiler PROPERTIES COMPILE_FLAGS -DPROFILER_EXPORTS)
    # ENDIF()
    # IF(PROFILER_BUILD_STATIC_LIBS)
        SET_TARGET_PROPERTIES(profiler_static PROPERTIES COMPILE_FLAGS -DPROFILER_STATIC)
        # need to set machine:x64 for linker, at least for VC10, and
        # doesn't hurt for older compilers:
        # http://public.kitware.com/Bug/view.php?id=11240#c22768
        IF (CMAKE_CL_64)
            SET_TARGET_PROPERTIES(profiler_static PROPERTIES STATIC_LIBRARY_FLAGS "/machine:x64")
        ENDIF()
    # ENDIF()
ELSE()
    SET(LIBNAME profiler)
    # IF(PROFILER_BUILD_SHARED_LIBS)
    #     SET_TARGET_PROPERTIES(profiler PROPERTIES VERSION 1.0.0 SOVERSION 0 OUTPUT_NAME ${LIBNAME})
    #     SET_TARGET_PROPERTIES(profiler PROPERTIES COMPILE_FLAGS -DPROFILER_EXPORTS)
    # ENDIF()
    # IF(PROFILER_BUILD_STATIC_LIBS)
        SET_TARGET_PROPERTIES(profiler_static PROPERTIES OUTPUT_NAME ${LIBNAME})
        SET_TARGET_PROPERTIES(profiler_static PROPERTIES COMPILE_FLAGS -DPROFILER_STATIC)
    # ENDIF()
    IF(ANDROID)
        # Not in CMake toolchain file, because the toolchain
        # file is called several times and generally doesn't
        # seem to be meant for modifying CFLAGS:
        # '-mandroid' is not mandatory but doesn't hurt
        # '-O0 -gstabs+' helps the currently buggy GDB port
        # Too late to manipulate ENV: SET(ENV{CFLAGS} "$ENV{CFLAGS} -mandroid")
        # Not using _INIT variables, they seem to be used internally only
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mandroid")
        SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -gstabs+")
    ENDIF()
ENDIF()
# IF(PROFILER_BUILD_SHARED_LIBS)
#     TARGET_LINK_LIBRARIES(profiler ${LIBS})
# ENDIF()
# IF(PROFILER_BUILD_STATIC_LIBS)
    TARGET_LINK_LIBRARIES(profiler_static ${LIBS})
# ENDIF()

# IF(PROFILER_BUILD_SHARED_LIBS)
#     INSTALL(TARGETS profiler
#             RUNTIME DESTINATION bin
#             LIBRARY DESTINATION "${LIB_INSTALL_DIR}"
#             ARCHIVE DESTINATION "${LIB_INSTALL_DIR}"
#     )
#     IF(INSTALL_PDB)
#         INSTALL(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/profiler${CMAKE_DEBUG_POSTFIX}.pdb
#             DESTINATION bin
# 			CONFIGURATIONS Debug)
#     ENDIF()
#     add_custom_command(TARGET profiler POST_BUILD
#         COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION> ${CMAKE_BINARY_DIR}/bin/
# )
# ENDIF()
# IF(PROFILER_BUILD_STATIC_LIBS)
    INSTALL(TARGETS profiler_static
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
    )
    IF(INSTALL_PDB)
        INSTALL(FILES ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/Debug/profiler_static${CMAKE_DEBUG_POSTFIX}.pdb
            DESTINATION lib
			CONFIGURATIONS Debug)
    ENDIF()
# ENDIF()


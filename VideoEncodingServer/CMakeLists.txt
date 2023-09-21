cmake_minimum_required(VERSION 3.12)
project(VideoEncodingServer)

set(CMAKE_CXX_STANDARD 17)
    
include(ExternalProject)

if(NOT EXISTS ${FFMPEG_SOURCE_DIR}/ffmpeg/src/FFmpeg)
    set(FFMPEG_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg/src)

    execute_process(
        COMMAND git clone https://github.com/FFmpeg/FFmpeg.git
        WORKING_DIRECTORY ${FFMPEG_SOURCE_DIR}
    )

endif()
 
ExternalProject_Add(
    ffmpeg_external
    PREFIX ffmpeg
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg/src/FFmpeg
    GIT_REPOSITORY "https://github.com/FFmpeg/FFmpeg.git"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
)

ExternalProject_Get_Property(ffmpeg_external SOURCE_DIR)
if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg/build/lib)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E chdir ${SOURCE_DIR} bash -c "${SOURCE_DIR}/configure --enable-shared --enable-gpl --enable-nonfree --enable-libvpx --enable-libsvtav1 --enable-libx265 --enable-libx264 --disable-static --prefix=${CMAKE_CURRENT_BINARY_DIR}/ffmpeg/build && make && make install"
    )
endif()

set(FFMPEG_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg/build/include)
set(FFMPEG_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg/build/lib)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPQXX REQUIRED libpqxx)
pkg_check_modules(CURL REQUIRED libcurl)

include_directories(${LIBPQXX_INCLUDE_DIRS})


set (CROW_INCLUDE_DIR ./3rdParty/crow)
set (PUGIXML_INCLUDE_DIR ./3rdParty/pugixml)
set (COMMON_INCLUDE_DIR ./Common)

file(GLOB_RECURSE SOURCES "*.cpp" "*.h" "*.c" "*.hpp")

add_executable(VideoEncodingServer ${SOURCES})

add_dependencies(VideoEncodingServer ffmpeg_external)

target_link_libraries(VideoEncodingServer PRIVATE
    ${FFMPEG_LIB_DIR}/libavformat.so
    ${FFMPEG_LIB_DIR}/libavcodec.so
    ${FFMPEG_LIB_DIR}/libpostproc.so
    ${FFMPEG_LIB_DIR}/libavutil.so
    ${FFMPEG_LIB_DIR}/libavfilter.so
    ${FFMPEG_LIB_DIR}/libswscale.so
    ${FFMPEG_LIB_DIR}/libswresample.so
    ${LIBPQXX_LIBRARIES}
    ${CURL_LIBRARIES}
)

target_include_directories(VideoEncodingServer PRIVATE
    ${FFMPEG_INCLUDE_DIR}
    ${CROW_INCLUDE_DIR}
    ${PUGIXML_INCLUDE_DIR}
    ${LIBDASH_BINARY_INCLUDE}
    ${COMMON_INCLUDE_DIR}
    /usr/include/postgresql
)

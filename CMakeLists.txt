cmake_minimum_required(VERSION 3.16)
project(ImageViewer VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 寻找SDL2库
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
pkg_check_modules(LIBARCHIVE REQUIRED libarchive)

# 添加可执行文件
add_executable(image_viewer
    src/main.cpp
    src/ImageViewer.cpp
    src/MenuBar.cpp
    src/FontManager.cpp
    src/SimpleFileDialog.cpp
)

# 添加头文件目录
target_include_directories(image_viewer PRIVATE
    include
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
    ${LIBARCHIVE_INCLUDE_DIRS}
)

# 链接库
target_link_libraries(image_viewer
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    ${LIBARCHIVE_LIBRARIES}
)

# 添加编译选项
target_compile_options(image_viewer PRIVATE 
    ${SDL2_CFLAGS_OTHER}
    ${SDL2_TTF_CFLAGS_OTHER}
    # ${GTK3_CFLAGS_OTHER}
)

# 设置输出目录
set_target_properties(image_viewer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 如果是Debug模式，添加调试信息
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(image_viewer PRIVATE -g -O0)
endif()

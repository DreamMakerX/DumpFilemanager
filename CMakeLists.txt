# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。

#CMake版本
cmake_minimum_required (VERSION 3.8)

project ("DumpFileManager")

# 包含子项目。
add_subdirectory ("DumpFileManager")

# 设置编译器宏 _AFXDLL
target_compile_definitions(DumpFileManager PRIVATE _AFXDLL)

# 其他必要的设置（例如，指定 C++ 标准等）
set_target_properties(DumpFileManager PROPERTIES CXX_STANDARD 11)
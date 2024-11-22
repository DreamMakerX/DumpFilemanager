# CMakeList.txt: CMakePrjTest 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#

# 将源代码添加到此项目的可执行文件。
add_executable (DumpFileManager 
	test.cpp
	DumpFileManager.cpp 
	DumpFileManager.h
)
# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.9

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.9.5/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.9.5/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/zhuangyi/zhuangyi/nfv_exam

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/zhuangyi/zhuangyi/nfv_exam/build

# Include any dependencies generated for this target.
include CMakeFiles/zz.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/zz.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/zz.dir/flags.make

CMakeFiles/zz.dir/src/log.cpp.o: CMakeFiles/zz.dir/flags.make
CMakeFiles/zz.dir/src/log.cpp.o: ../src/log.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/zhuangyi/zhuangyi/nfv_exam/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/zz.dir/src/log.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/zz.dir/src/log.cpp.o -c /Users/zhuangyi/zhuangyi/nfv_exam/src/log.cpp

CMakeFiles/zz.dir/src/log.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/zz.dir/src/log.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/zhuangyi/zhuangyi/nfv_exam/src/log.cpp > CMakeFiles/zz.dir/src/log.cpp.i

CMakeFiles/zz.dir/src/log.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/zz.dir/src/log.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/zhuangyi/zhuangyi/nfv_exam/src/log.cpp -o CMakeFiles/zz.dir/src/log.cpp.s

CMakeFiles/zz.dir/src/log.cpp.o.requires:

.PHONY : CMakeFiles/zz.dir/src/log.cpp.o.requires

CMakeFiles/zz.dir/src/log.cpp.o.provides: CMakeFiles/zz.dir/src/log.cpp.o.requires
	$(MAKE) -f CMakeFiles/zz.dir/build.make CMakeFiles/zz.dir/src/log.cpp.o.provides.build
.PHONY : CMakeFiles/zz.dir/src/log.cpp.o.provides

CMakeFiles/zz.dir/src/log.cpp.o.provides.build: CMakeFiles/zz.dir/src/log.cpp.o


CMakeFiles/zz.dir/src/main.cpp.o: CMakeFiles/zz.dir/flags.make
CMakeFiles/zz.dir/src/main.cpp.o: ../src/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/zhuangyi/zhuangyi/nfv_exam/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/zz.dir/src/main.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/zz.dir/src/main.cpp.o -c /Users/zhuangyi/zhuangyi/nfv_exam/src/main.cpp

CMakeFiles/zz.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/zz.dir/src/main.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/zhuangyi/zhuangyi/nfv_exam/src/main.cpp > CMakeFiles/zz.dir/src/main.cpp.i

CMakeFiles/zz.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/zz.dir/src/main.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/zhuangyi/zhuangyi/nfv_exam/src/main.cpp -o CMakeFiles/zz.dir/src/main.cpp.s

CMakeFiles/zz.dir/src/main.cpp.o.requires:

.PHONY : CMakeFiles/zz.dir/src/main.cpp.o.requires

CMakeFiles/zz.dir/src/main.cpp.o.provides: CMakeFiles/zz.dir/src/main.cpp.o.requires
	$(MAKE) -f CMakeFiles/zz.dir/build.make CMakeFiles/zz.dir/src/main.cpp.o.provides.build
.PHONY : CMakeFiles/zz.dir/src/main.cpp.o.provides

CMakeFiles/zz.dir/src/main.cpp.o.provides.build: CMakeFiles/zz.dir/src/main.cpp.o


CMakeFiles/zz.dir/src/physical_node.cpp.o: CMakeFiles/zz.dir/flags.make
CMakeFiles/zz.dir/src/physical_node.cpp.o: ../src/physical_node.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/zhuangyi/zhuangyi/nfv_exam/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/zz.dir/src/physical_node.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/zz.dir/src/physical_node.cpp.o -c /Users/zhuangyi/zhuangyi/nfv_exam/src/physical_node.cpp

CMakeFiles/zz.dir/src/physical_node.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/zz.dir/src/physical_node.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/zhuangyi/zhuangyi/nfv_exam/src/physical_node.cpp > CMakeFiles/zz.dir/src/physical_node.cpp.i

CMakeFiles/zz.dir/src/physical_node.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/zz.dir/src/physical_node.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/zhuangyi/zhuangyi/nfv_exam/src/physical_node.cpp -o CMakeFiles/zz.dir/src/physical_node.cpp.s

CMakeFiles/zz.dir/src/physical_node.cpp.o.requires:

.PHONY : CMakeFiles/zz.dir/src/physical_node.cpp.o.requires

CMakeFiles/zz.dir/src/physical_node.cpp.o.provides: CMakeFiles/zz.dir/src/physical_node.cpp.o.requires
	$(MAKE) -f CMakeFiles/zz.dir/build.make CMakeFiles/zz.dir/src/physical_node.cpp.o.provides.build
.PHONY : CMakeFiles/zz.dir/src/physical_node.cpp.o.provides

CMakeFiles/zz.dir/src/physical_node.cpp.o.provides.build: CMakeFiles/zz.dir/src/physical_node.cpp.o


CMakeFiles/zz.dir/src/physical_node_manager.cpp.o: CMakeFiles/zz.dir/flags.make
CMakeFiles/zz.dir/src/physical_node_manager.cpp.o: ../src/physical_node_manager.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/zhuangyi/zhuangyi/nfv_exam/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/zz.dir/src/physical_node_manager.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/zz.dir/src/physical_node_manager.cpp.o -c /Users/zhuangyi/zhuangyi/nfv_exam/src/physical_node_manager.cpp

CMakeFiles/zz.dir/src/physical_node_manager.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/zz.dir/src/physical_node_manager.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/zhuangyi/zhuangyi/nfv_exam/src/physical_node_manager.cpp > CMakeFiles/zz.dir/src/physical_node_manager.cpp.i

CMakeFiles/zz.dir/src/physical_node_manager.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/zz.dir/src/physical_node_manager.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/zhuangyi/zhuangyi/nfv_exam/src/physical_node_manager.cpp -o CMakeFiles/zz.dir/src/physical_node_manager.cpp.s

CMakeFiles/zz.dir/src/physical_node_manager.cpp.o.requires:

.PHONY : CMakeFiles/zz.dir/src/physical_node_manager.cpp.o.requires

CMakeFiles/zz.dir/src/physical_node_manager.cpp.o.provides: CMakeFiles/zz.dir/src/physical_node_manager.cpp.o.requires
	$(MAKE) -f CMakeFiles/zz.dir/build.make CMakeFiles/zz.dir/src/physical_node_manager.cpp.o.provides.build
.PHONY : CMakeFiles/zz.dir/src/physical_node_manager.cpp.o.provides

CMakeFiles/zz.dir/src/physical_node_manager.cpp.o.provides.build: CMakeFiles/zz.dir/src/physical_node_manager.cpp.o


# Object files for target zz
zz_OBJECTS = \
"CMakeFiles/zz.dir/src/log.cpp.o" \
"CMakeFiles/zz.dir/src/main.cpp.o" \
"CMakeFiles/zz.dir/src/physical_node.cpp.o" \
"CMakeFiles/zz.dir/src/physical_node_manager.cpp.o"

# External object files for target zz
zz_EXTERNAL_OBJECTS =

../bin/zz: CMakeFiles/zz.dir/src/log.cpp.o
../bin/zz: CMakeFiles/zz.dir/src/main.cpp.o
../bin/zz: CMakeFiles/zz.dir/src/physical_node.cpp.o
../bin/zz: CMakeFiles/zz.dir/src/physical_node_manager.cpp.o
../bin/zz: CMakeFiles/zz.dir/build.make
../bin/zz: CMakeFiles/zz.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/zhuangyi/zhuangyi/nfv_exam/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking CXX executable ../bin/zz"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/zz.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/zz.dir/build: ../bin/zz

.PHONY : CMakeFiles/zz.dir/build

CMakeFiles/zz.dir/requires: CMakeFiles/zz.dir/src/log.cpp.o.requires
CMakeFiles/zz.dir/requires: CMakeFiles/zz.dir/src/main.cpp.o.requires
CMakeFiles/zz.dir/requires: CMakeFiles/zz.dir/src/physical_node.cpp.o.requires
CMakeFiles/zz.dir/requires: CMakeFiles/zz.dir/src/physical_node_manager.cpp.o.requires

.PHONY : CMakeFiles/zz.dir/requires

CMakeFiles/zz.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/zz.dir/cmake_clean.cmake
.PHONY : CMakeFiles/zz.dir/clean

CMakeFiles/zz.dir/depend:
	cd /Users/zhuangyi/zhuangyi/nfv_exam/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/zhuangyi/zhuangyi/nfv_exam /Users/zhuangyi/zhuangyi/nfv_exam /Users/zhuangyi/zhuangyi/nfv_exam/build /Users/zhuangyi/zhuangyi/nfv_exam/build /Users/zhuangyi/zhuangyi/nfv_exam/build/CMakeFiles/zz.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/zz.dir/depend

# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/brucelee/CS1951_DataStructures/BPT

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/brucelee/CS1951_DataStructures/BPT/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/BPT.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/BPT.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/BPT.dir/flags.make

CMakeFiles/BPT.dir/main.cpp.o: CMakeFiles/BPT.dir/flags.make
CMakeFiles/BPT.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/brucelee/CS1951_DataStructures/BPT/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/BPT.dir/main.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/BPT.dir/main.cpp.o -c /home/brucelee/CS1951_DataStructures/BPT/main.cpp

CMakeFiles/BPT.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/BPT.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/brucelee/CS1951_DataStructures/BPT/main.cpp > CMakeFiles/BPT.dir/main.cpp.i

CMakeFiles/BPT.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/BPT.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/brucelee/CS1951_DataStructures/BPT/main.cpp -o CMakeFiles/BPT.dir/main.cpp.s

# Object files for target BPT
BPT_OBJECTS = \
"CMakeFiles/BPT.dir/main.cpp.o"

# External object files for target BPT
BPT_EXTERNAL_OBJECTS =

BPT: CMakeFiles/BPT.dir/main.cpp.o
BPT: CMakeFiles/BPT.dir/build.make
BPT: CMakeFiles/BPT.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/brucelee/CS1951_DataStructures/BPT/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable BPT"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/BPT.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/BPT.dir/build: BPT

.PHONY : CMakeFiles/BPT.dir/build

CMakeFiles/BPT.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/BPT.dir/cmake_clean.cmake
.PHONY : CMakeFiles/BPT.dir/clean

CMakeFiles/BPT.dir/depend:
	cd /home/brucelee/CS1951_DataStructures/BPT/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/brucelee/CS1951_DataStructures/BPT /home/brucelee/CS1951_DataStructures/BPT /home/brucelee/CS1951_DataStructures/BPT/cmake-build-debug /home/brucelee/CS1951_DataStructures/BPT/cmake-build-debug /home/brucelee/CS1951_DataStructures/BPT/cmake-build-debug/CMakeFiles/BPT.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/BPT.dir/depend


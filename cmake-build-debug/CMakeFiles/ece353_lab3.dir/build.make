# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.8

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
CMAKE_COMMAND = "/cygdrive/c/Users/Logan Emerson/.CLion2017.2/system/cygwin_cmake/bin/cmake.exe"

# The command to remove a file.
RM = "/cygdrive/c/Users/Logan Emerson/.CLion2017.2/system/cygwin_cmake/bin/cmake.exe" -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "/cygdrive/c/Users/Logan Emerson/Documents/GitHub/ece353-lab3"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "/cygdrive/c/Users/Logan Emerson/Documents/GitHub/ece353-lab3/cmake-build-debug"

# Include any dependencies generated for this target.
include CMakeFiles/ece353_lab3.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/ece353_lab3.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/ece353_lab3.dir/flags.make

CMakeFiles/ece353_lab3.dir/P3Skeleton.c.o: CMakeFiles/ece353_lab3.dir/flags.make
CMakeFiles/ece353_lab3.dir/P3Skeleton.c.o: ../P3Skeleton.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/cygdrive/c/Users/Logan Emerson/Documents/GitHub/ece353-lab3/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/ece353_lab3.dir/P3Skeleton.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/ece353_lab3.dir/P3Skeleton.c.o   -c "/cygdrive/c/Users/Logan Emerson/Documents/GitHub/ece353-lab3/P3Skeleton.c"

CMakeFiles/ece353_lab3.dir/P3Skeleton.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/ece353_lab3.dir/P3Skeleton.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "/cygdrive/c/Users/Logan Emerson/Documents/GitHub/ece353-lab3/P3Skeleton.c" > CMakeFiles/ece353_lab3.dir/P3Skeleton.c.i

CMakeFiles/ece353_lab3.dir/P3Skeleton.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/ece353_lab3.dir/P3Skeleton.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "/cygdrive/c/Users/Logan Emerson/Documents/GitHub/ece353-lab3/P3Skeleton.c" -o CMakeFiles/ece353_lab3.dir/P3Skeleton.c.s

CMakeFiles/ece353_lab3.dir/P3Skeleton.c.o.requires:

.PHONY : CMakeFiles/ece353_lab3.dir/P3Skeleton.c.o.requires

CMakeFiles/ece353_lab3.dir/P3Skeleton.c.o.provides: CMakeFiles/ece353_lab3.dir/P3Skeleton.c.o.requires
	$(MAKE) -f CMakeFiles/ece353_lab3.dir/build.make CMakeFiles/ece353_lab3.dir/P3Skeleton.c.o.provides.build
.PHONY : CMakeFiles/ece353_lab3.dir/P3Skeleton.c.o.provides

CMakeFiles/ece353_lab3.dir/P3Skeleton.c.o.provides.build: CMakeFiles/ece353_lab3.dir/P3Skeleton.c.o


# Object files for target ece353_lab3
ece353_lab3_OBJECTS = \
"CMakeFiles/ece353_lab3.dir/P3Skeleton.c.o"

# External object files for target ece353_lab3
ece353_lab3_EXTERNAL_OBJECTS =

ece353_lab3.exe: CMakeFiles/ece353_lab3.dir/P3Skeleton.c.o
ece353_lab3.exe: CMakeFiles/ece353_lab3.dir/build.make
ece353_lab3.exe: CMakeFiles/ece353_lab3.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir="/cygdrive/c/Users/Logan Emerson/Documents/GitHub/ece353-lab3/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable ece353_lab3.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ece353_lab3.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/ece353_lab3.dir/build: ece353_lab3.exe

.PHONY : CMakeFiles/ece353_lab3.dir/build

CMakeFiles/ece353_lab3.dir/requires: CMakeFiles/ece353_lab3.dir/P3Skeleton.c.o.requires

.PHONY : CMakeFiles/ece353_lab3.dir/requires

CMakeFiles/ece353_lab3.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/ece353_lab3.dir/cmake_clean.cmake
.PHONY : CMakeFiles/ece353_lab3.dir/clean

CMakeFiles/ece353_lab3.dir/depend:
	cd "/cygdrive/c/Users/Logan Emerson/Documents/GitHub/ece353-lab3/cmake-build-debug" && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" "/cygdrive/c/Users/Logan Emerson/Documents/GitHub/ece353-lab3" "/cygdrive/c/Users/Logan Emerson/Documents/GitHub/ece353-lab3" "/cygdrive/c/Users/Logan Emerson/Documents/GitHub/ece353-lab3/cmake-build-debug" "/cygdrive/c/Users/Logan Emerson/Documents/GitHub/ece353-lab3/cmake-build-debug" "/cygdrive/c/Users/Logan Emerson/Documents/GitHub/ece353-lab3/cmake-build-debug/CMakeFiles/ece353_lab3.dir/DependInfo.cmake" --color=$(COLOR)
.PHONY : CMakeFiles/ece353_lab3.dir/depend


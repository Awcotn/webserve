# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.20

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Produce verbose output by default.
VERBOSE = 1

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /root/workspace/webserve

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /root/workspace/webserve

# Include any dependencies generated for this target.
include CMakeFiles/awcotn.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/awcotn.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/awcotn.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/awcotn.dir/flags.make

CMakeFiles/awcotn.dir/awcotn/config.cc.o: CMakeFiles/awcotn.dir/flags.make
CMakeFiles/awcotn.dir/awcotn/config.cc.o: awcotn/config.cc
CMakeFiles/awcotn.dir/awcotn/config.cc.o: CMakeFiles/awcotn.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/workspace/webserve/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/awcotn.dir/awcotn/config.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/config.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/awcotn.dir/awcotn/config.cc.o -MF CMakeFiles/awcotn.dir/awcotn/config.cc.o.d -o CMakeFiles/awcotn.dir/awcotn/config.cc.o -c /root/workspace/webserve/awcotn/config.cc

CMakeFiles/awcotn.dir/awcotn/config.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/awcotn.dir/awcotn/config.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/config.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/workspace/webserve/awcotn/config.cc > CMakeFiles/awcotn.dir/awcotn/config.cc.i

CMakeFiles/awcotn.dir/awcotn/config.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/awcotn.dir/awcotn/config.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/config.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/workspace/webserve/awcotn/config.cc -o CMakeFiles/awcotn.dir/awcotn/config.cc.s

CMakeFiles/awcotn.dir/awcotn/fd_manager.cc.o: CMakeFiles/awcotn.dir/flags.make
CMakeFiles/awcotn.dir/awcotn/fd_manager.cc.o: awcotn/fd_manager.cc
CMakeFiles/awcotn.dir/awcotn/fd_manager.cc.o: CMakeFiles/awcotn.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/workspace/webserve/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/awcotn.dir/awcotn/fd_manager.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/fd_manager.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/awcotn.dir/awcotn/fd_manager.cc.o -MF CMakeFiles/awcotn.dir/awcotn/fd_manager.cc.o.d -o CMakeFiles/awcotn.dir/awcotn/fd_manager.cc.o -c /root/workspace/webserve/awcotn/fd_manager.cc

CMakeFiles/awcotn.dir/awcotn/fd_manager.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/awcotn.dir/awcotn/fd_manager.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/fd_manager.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/workspace/webserve/awcotn/fd_manager.cc > CMakeFiles/awcotn.dir/awcotn/fd_manager.cc.i

CMakeFiles/awcotn.dir/awcotn/fd_manager.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/awcotn.dir/awcotn/fd_manager.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/fd_manager.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/workspace/webserve/awcotn/fd_manager.cc -o CMakeFiles/awcotn.dir/awcotn/fd_manager.cc.s

CMakeFiles/awcotn.dir/awcotn/fiber.cc.o: CMakeFiles/awcotn.dir/flags.make
CMakeFiles/awcotn.dir/awcotn/fiber.cc.o: awcotn/fiber.cc
CMakeFiles/awcotn.dir/awcotn/fiber.cc.o: CMakeFiles/awcotn.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/workspace/webserve/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/awcotn.dir/awcotn/fiber.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/fiber.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/awcotn.dir/awcotn/fiber.cc.o -MF CMakeFiles/awcotn.dir/awcotn/fiber.cc.o.d -o CMakeFiles/awcotn.dir/awcotn/fiber.cc.o -c /root/workspace/webserve/awcotn/fiber.cc

CMakeFiles/awcotn.dir/awcotn/fiber.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/awcotn.dir/awcotn/fiber.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/fiber.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/workspace/webserve/awcotn/fiber.cc > CMakeFiles/awcotn.dir/awcotn/fiber.cc.i

CMakeFiles/awcotn.dir/awcotn/fiber.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/awcotn.dir/awcotn/fiber.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/fiber.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/workspace/webserve/awcotn/fiber.cc -o CMakeFiles/awcotn.dir/awcotn/fiber.cc.s

CMakeFiles/awcotn.dir/awcotn/hook.cc.o: CMakeFiles/awcotn.dir/flags.make
CMakeFiles/awcotn.dir/awcotn/hook.cc.o: awcotn/hook.cc
CMakeFiles/awcotn.dir/awcotn/hook.cc.o: CMakeFiles/awcotn.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/workspace/webserve/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/awcotn.dir/awcotn/hook.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/hook.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/awcotn.dir/awcotn/hook.cc.o -MF CMakeFiles/awcotn.dir/awcotn/hook.cc.o.d -o CMakeFiles/awcotn.dir/awcotn/hook.cc.o -c /root/workspace/webserve/awcotn/hook.cc

CMakeFiles/awcotn.dir/awcotn/hook.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/awcotn.dir/awcotn/hook.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/hook.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/workspace/webserve/awcotn/hook.cc > CMakeFiles/awcotn.dir/awcotn/hook.cc.i

CMakeFiles/awcotn.dir/awcotn/hook.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/awcotn.dir/awcotn/hook.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/hook.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/workspace/webserve/awcotn/hook.cc -o CMakeFiles/awcotn.dir/awcotn/hook.cc.s

CMakeFiles/awcotn.dir/awcotn/iomanager.cc.o: CMakeFiles/awcotn.dir/flags.make
CMakeFiles/awcotn.dir/awcotn/iomanager.cc.o: awcotn/iomanager.cc
CMakeFiles/awcotn.dir/awcotn/iomanager.cc.o: CMakeFiles/awcotn.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/workspace/webserve/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/awcotn.dir/awcotn/iomanager.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/iomanager.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/awcotn.dir/awcotn/iomanager.cc.o -MF CMakeFiles/awcotn.dir/awcotn/iomanager.cc.o.d -o CMakeFiles/awcotn.dir/awcotn/iomanager.cc.o -c /root/workspace/webserve/awcotn/iomanager.cc

CMakeFiles/awcotn.dir/awcotn/iomanager.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/awcotn.dir/awcotn/iomanager.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/iomanager.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/workspace/webserve/awcotn/iomanager.cc > CMakeFiles/awcotn.dir/awcotn/iomanager.cc.i

CMakeFiles/awcotn.dir/awcotn/iomanager.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/awcotn.dir/awcotn/iomanager.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/iomanager.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/workspace/webserve/awcotn/iomanager.cc -o CMakeFiles/awcotn.dir/awcotn/iomanager.cc.s

CMakeFiles/awcotn.dir/awcotn/log.cc.o: CMakeFiles/awcotn.dir/flags.make
CMakeFiles/awcotn.dir/awcotn/log.cc.o: awcotn/log.cc
CMakeFiles/awcotn.dir/awcotn/log.cc.o: CMakeFiles/awcotn.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/workspace/webserve/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/awcotn.dir/awcotn/log.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/log.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/awcotn.dir/awcotn/log.cc.o -MF CMakeFiles/awcotn.dir/awcotn/log.cc.o.d -o CMakeFiles/awcotn.dir/awcotn/log.cc.o -c /root/workspace/webserve/awcotn/log.cc

CMakeFiles/awcotn.dir/awcotn/log.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/awcotn.dir/awcotn/log.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/log.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/workspace/webserve/awcotn/log.cc > CMakeFiles/awcotn.dir/awcotn/log.cc.i

CMakeFiles/awcotn.dir/awcotn/log.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/awcotn.dir/awcotn/log.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/log.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/workspace/webserve/awcotn/log.cc -o CMakeFiles/awcotn.dir/awcotn/log.cc.s

CMakeFiles/awcotn.dir/awcotn/scheduler.cc.o: CMakeFiles/awcotn.dir/flags.make
CMakeFiles/awcotn.dir/awcotn/scheduler.cc.o: awcotn/scheduler.cc
CMakeFiles/awcotn.dir/awcotn/scheduler.cc.o: CMakeFiles/awcotn.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/workspace/webserve/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object CMakeFiles/awcotn.dir/awcotn/scheduler.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/scheduler.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/awcotn.dir/awcotn/scheduler.cc.o -MF CMakeFiles/awcotn.dir/awcotn/scheduler.cc.o.d -o CMakeFiles/awcotn.dir/awcotn/scheduler.cc.o -c /root/workspace/webserve/awcotn/scheduler.cc

CMakeFiles/awcotn.dir/awcotn/scheduler.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/awcotn.dir/awcotn/scheduler.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/scheduler.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/workspace/webserve/awcotn/scheduler.cc > CMakeFiles/awcotn.dir/awcotn/scheduler.cc.i

CMakeFiles/awcotn.dir/awcotn/scheduler.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/awcotn.dir/awcotn/scheduler.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/scheduler.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/workspace/webserve/awcotn/scheduler.cc -o CMakeFiles/awcotn.dir/awcotn/scheduler.cc.s

CMakeFiles/awcotn.dir/awcotn/mutex.cc.o: CMakeFiles/awcotn.dir/flags.make
CMakeFiles/awcotn.dir/awcotn/mutex.cc.o: awcotn/mutex.cc
CMakeFiles/awcotn.dir/awcotn/mutex.cc.o: CMakeFiles/awcotn.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/workspace/webserve/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object CMakeFiles/awcotn.dir/awcotn/mutex.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/mutex.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/awcotn.dir/awcotn/mutex.cc.o -MF CMakeFiles/awcotn.dir/awcotn/mutex.cc.o.d -o CMakeFiles/awcotn.dir/awcotn/mutex.cc.o -c /root/workspace/webserve/awcotn/mutex.cc

CMakeFiles/awcotn.dir/awcotn/mutex.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/awcotn.dir/awcotn/mutex.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/mutex.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/workspace/webserve/awcotn/mutex.cc > CMakeFiles/awcotn.dir/awcotn/mutex.cc.i

CMakeFiles/awcotn.dir/awcotn/mutex.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/awcotn.dir/awcotn/mutex.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/mutex.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/workspace/webserve/awcotn/mutex.cc -o CMakeFiles/awcotn.dir/awcotn/mutex.cc.s

CMakeFiles/awcotn.dir/awcotn/timer.cc.o: CMakeFiles/awcotn.dir/flags.make
CMakeFiles/awcotn.dir/awcotn/timer.cc.o: awcotn/timer.cc
CMakeFiles/awcotn.dir/awcotn/timer.cc.o: CMakeFiles/awcotn.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/workspace/webserve/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building CXX object CMakeFiles/awcotn.dir/awcotn/timer.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/timer.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/awcotn.dir/awcotn/timer.cc.o -MF CMakeFiles/awcotn.dir/awcotn/timer.cc.o.d -o CMakeFiles/awcotn.dir/awcotn/timer.cc.o -c /root/workspace/webserve/awcotn/timer.cc

CMakeFiles/awcotn.dir/awcotn/timer.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/awcotn.dir/awcotn/timer.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/timer.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/workspace/webserve/awcotn/timer.cc > CMakeFiles/awcotn.dir/awcotn/timer.cc.i

CMakeFiles/awcotn.dir/awcotn/timer.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/awcotn.dir/awcotn/timer.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/timer.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/workspace/webserve/awcotn/timer.cc -o CMakeFiles/awcotn.dir/awcotn/timer.cc.s

CMakeFiles/awcotn.dir/awcotn/thread.cc.o: CMakeFiles/awcotn.dir/flags.make
CMakeFiles/awcotn.dir/awcotn/thread.cc.o: awcotn/thread.cc
CMakeFiles/awcotn.dir/awcotn/thread.cc.o: CMakeFiles/awcotn.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/workspace/webserve/CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Building CXX object CMakeFiles/awcotn.dir/awcotn/thread.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/thread.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/awcotn.dir/awcotn/thread.cc.o -MF CMakeFiles/awcotn.dir/awcotn/thread.cc.o.d -o CMakeFiles/awcotn.dir/awcotn/thread.cc.o -c /root/workspace/webserve/awcotn/thread.cc

CMakeFiles/awcotn.dir/awcotn/thread.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/awcotn.dir/awcotn/thread.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/thread.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/workspace/webserve/awcotn/thread.cc > CMakeFiles/awcotn.dir/awcotn/thread.cc.i

CMakeFiles/awcotn.dir/awcotn/thread.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/awcotn.dir/awcotn/thread.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/thread.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/workspace/webserve/awcotn/thread.cc -o CMakeFiles/awcotn.dir/awcotn/thread.cc.s

CMakeFiles/awcotn.dir/awcotn/util.cc.o: CMakeFiles/awcotn.dir/flags.make
CMakeFiles/awcotn.dir/awcotn/util.cc.o: awcotn/util.cc
CMakeFiles/awcotn.dir/awcotn/util.cc.o: CMakeFiles/awcotn.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/workspace/webserve/CMakeFiles --progress-num=$(CMAKE_PROGRESS_11) "Building CXX object CMakeFiles/awcotn.dir/awcotn/util.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/util.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/awcotn.dir/awcotn/util.cc.o -MF CMakeFiles/awcotn.dir/awcotn/util.cc.o.d -o CMakeFiles/awcotn.dir/awcotn/util.cc.o -c /root/workspace/webserve/awcotn/util.cc

CMakeFiles/awcotn.dir/awcotn/util.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/awcotn.dir/awcotn/util.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/util.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/workspace/webserve/awcotn/util.cc > CMakeFiles/awcotn.dir/awcotn/util.cc.i

CMakeFiles/awcotn.dir/awcotn/util.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/awcotn.dir/awcotn/util.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"awcotn/util.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/workspace/webserve/awcotn/util.cc -o CMakeFiles/awcotn.dir/awcotn/util.cc.s

# Object files for target awcotn
awcotn_OBJECTS = \
"CMakeFiles/awcotn.dir/awcotn/config.cc.o" \
"CMakeFiles/awcotn.dir/awcotn/fd_manager.cc.o" \
"CMakeFiles/awcotn.dir/awcotn/fiber.cc.o" \
"CMakeFiles/awcotn.dir/awcotn/hook.cc.o" \
"CMakeFiles/awcotn.dir/awcotn/iomanager.cc.o" \
"CMakeFiles/awcotn.dir/awcotn/log.cc.o" \
"CMakeFiles/awcotn.dir/awcotn/scheduler.cc.o" \
"CMakeFiles/awcotn.dir/awcotn/mutex.cc.o" \
"CMakeFiles/awcotn.dir/awcotn/timer.cc.o" \
"CMakeFiles/awcotn.dir/awcotn/thread.cc.o" \
"CMakeFiles/awcotn.dir/awcotn/util.cc.o"

# External object files for target awcotn
awcotn_EXTERNAL_OBJECTS =

lib/libawcotn.so: CMakeFiles/awcotn.dir/awcotn/config.cc.o
lib/libawcotn.so: CMakeFiles/awcotn.dir/awcotn/fd_manager.cc.o
lib/libawcotn.so: CMakeFiles/awcotn.dir/awcotn/fiber.cc.o
lib/libawcotn.so: CMakeFiles/awcotn.dir/awcotn/hook.cc.o
lib/libawcotn.so: CMakeFiles/awcotn.dir/awcotn/iomanager.cc.o
lib/libawcotn.so: CMakeFiles/awcotn.dir/awcotn/log.cc.o
lib/libawcotn.so: CMakeFiles/awcotn.dir/awcotn/scheduler.cc.o
lib/libawcotn.so: CMakeFiles/awcotn.dir/awcotn/mutex.cc.o
lib/libawcotn.so: CMakeFiles/awcotn.dir/awcotn/timer.cc.o
lib/libawcotn.so: CMakeFiles/awcotn.dir/awcotn/thread.cc.o
lib/libawcotn.so: CMakeFiles/awcotn.dir/awcotn/util.cc.o
lib/libawcotn.so: CMakeFiles/awcotn.dir/build.make
lib/libawcotn.so: CMakeFiles/awcotn.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/root/workspace/webserve/CMakeFiles --progress-num=$(CMAKE_PROGRESS_12) "Linking CXX shared library lib/libawcotn.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/awcotn.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/awcotn.dir/build: lib/libawcotn.so
.PHONY : CMakeFiles/awcotn.dir/build

CMakeFiles/awcotn.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/awcotn.dir/cmake_clean.cmake
.PHONY : CMakeFiles/awcotn.dir/clean

CMakeFiles/awcotn.dir/depend:
	cd /root/workspace/webserve && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /root/workspace/webserve /root/workspace/webserve /root/workspace/webserve /root/workspace/webserve /root/workspace/webserve/CMakeFiles/awcotn.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/awcotn.dir/depend


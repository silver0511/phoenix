# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.6

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
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/shiyunjie/Documents/work/server/qbao/qbim

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/shiyunjie/Documents/work/server/qbao/qbim/cmake-build-debug

# Include any dependencies generated for this target.
include kafka/linux_prj/CMakeFiles/kafka.dir/depend.make

# Include the progress variables for this target.
include kafka/linux_prj/CMakeFiles/kafka.dir/progress.make

# Include the compile flags for this target's objects.
include kafka/linux_prj/CMakeFiles/kafka.dir/flags.make

kafka/linux_prj/CMakeFiles/kafka.dir/__/src/KafkaMgr.cpp.o: kafka/linux_prj/CMakeFiles/kafka.dir/flags.make
kafka/linux_prj/CMakeFiles/kafka.dir/__/src/KafkaMgr.cpp.o: ../kafka/src/KafkaMgr.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/shiyunjie/Documents/work/server/qbao/qbim/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object kafka/linux_prj/CMakeFiles/kafka.dir/__/src/KafkaMgr.cpp.o"
	cd /Users/shiyunjie/Documents/work/server/qbao/qbim/cmake-build-debug/kafka/linux_prj && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/kafka.dir/__/src/KafkaMgr.cpp.o -c /Users/shiyunjie/Documents/work/server/qbao/qbim/kafka/src/KafkaMgr.cpp

kafka/linux_prj/CMakeFiles/kafka.dir/__/src/KafkaMgr.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/kafka.dir/__/src/KafkaMgr.cpp.i"
	cd /Users/shiyunjie/Documents/work/server/qbao/qbim/cmake-build-debug/kafka/linux_prj && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/shiyunjie/Documents/work/server/qbao/qbim/kafka/src/KafkaMgr.cpp > CMakeFiles/kafka.dir/__/src/KafkaMgr.cpp.i

kafka/linux_prj/CMakeFiles/kafka.dir/__/src/KafkaMgr.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/kafka.dir/__/src/KafkaMgr.cpp.s"
	cd /Users/shiyunjie/Documents/work/server/qbao/qbim/cmake-build-debug/kafka/linux_prj && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/shiyunjie/Documents/work/server/qbao/qbim/kafka/src/KafkaMgr.cpp -o CMakeFiles/kafka.dir/__/src/KafkaMgr.cpp.s

kafka/linux_prj/CMakeFiles/kafka.dir/__/src/KafkaMgr.cpp.o.requires:

.PHONY : kafka/linux_prj/CMakeFiles/kafka.dir/__/src/KafkaMgr.cpp.o.requires

kafka/linux_prj/CMakeFiles/kafka.dir/__/src/KafkaMgr.cpp.o.provides: kafka/linux_prj/CMakeFiles/kafka.dir/__/src/KafkaMgr.cpp.o.requires
	$(MAKE) -f kafka/linux_prj/CMakeFiles/kafka.dir/build.make kafka/linux_prj/CMakeFiles/kafka.dir/__/src/KafkaMgr.cpp.o.provides.build
.PHONY : kafka/linux_prj/CMakeFiles/kafka.dir/__/src/KafkaMgr.cpp.o.provides

kafka/linux_prj/CMakeFiles/kafka.dir/__/src/KafkaMgr.cpp.o.provides.build: kafka/linux_prj/CMakeFiles/kafka.dir/__/src/KafkaMgr.cpp.o


kafka/linux_prj/CMakeFiles/kafka.dir/__/src/KafkaProducer.cpp.o: kafka/linux_prj/CMakeFiles/kafka.dir/flags.make
kafka/linux_prj/CMakeFiles/kafka.dir/__/src/KafkaProducer.cpp.o: ../kafka/src/KafkaProducer.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/shiyunjie/Documents/work/server/qbao/qbim/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object kafka/linux_prj/CMakeFiles/kafka.dir/__/src/KafkaProducer.cpp.o"
	cd /Users/shiyunjie/Documents/work/server/qbao/qbim/cmake-build-debug/kafka/linux_prj && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/kafka.dir/__/src/KafkaProducer.cpp.o -c /Users/shiyunjie/Documents/work/server/qbao/qbim/kafka/src/KafkaProducer.cpp

kafka/linux_prj/CMakeFiles/kafka.dir/__/src/KafkaProducer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/kafka.dir/__/src/KafkaProducer.cpp.i"
	cd /Users/shiyunjie/Documents/work/server/qbao/qbim/cmake-build-debug/kafka/linux_prj && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/shiyunjie/Documents/work/server/qbao/qbim/kafka/src/KafkaProducer.cpp > CMakeFiles/kafka.dir/__/src/KafkaProducer.cpp.i

kafka/linux_prj/CMakeFiles/kafka.dir/__/src/KafkaProducer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/kafka.dir/__/src/KafkaProducer.cpp.s"
	cd /Users/shiyunjie/Documents/work/server/qbao/qbim/cmake-build-debug/kafka/linux_prj && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/shiyunjie/Documents/work/server/qbao/qbim/kafka/src/KafkaProducer.cpp -o CMakeFiles/kafka.dir/__/src/KafkaProducer.cpp.s

kafka/linux_prj/CMakeFiles/kafka.dir/__/src/KafkaProducer.cpp.o.requires:

.PHONY : kafka/linux_prj/CMakeFiles/kafka.dir/__/src/KafkaProducer.cpp.o.requires

kafka/linux_prj/CMakeFiles/kafka.dir/__/src/KafkaProducer.cpp.o.provides: kafka/linux_prj/CMakeFiles/kafka.dir/__/src/KafkaProducer.cpp.o.requires
	$(MAKE) -f kafka/linux_prj/CMakeFiles/kafka.dir/build.make kafka/linux_prj/CMakeFiles/kafka.dir/__/src/KafkaProducer.cpp.o.provides.build
.PHONY : kafka/linux_prj/CMakeFiles/kafka.dir/__/src/KafkaProducer.cpp.o.provides

kafka/linux_prj/CMakeFiles/kafka.dir/__/src/KafkaProducer.cpp.o.provides.build: kafka/linux_prj/CMakeFiles/kafka.dir/__/src/KafkaProducer.cpp.o


# Object files for target kafka
kafka_OBJECTS = \
"CMakeFiles/kafka.dir/__/src/KafkaMgr.cpp.o" \
"CMakeFiles/kafka.dir/__/src/KafkaProducer.cpp.o"

# External object files for target kafka
kafka_EXTERNAL_OBJECTS =

kafka/linux_prj/libkafka.a: kafka/linux_prj/CMakeFiles/kafka.dir/__/src/KafkaMgr.cpp.o
kafka/linux_prj/libkafka.a: kafka/linux_prj/CMakeFiles/kafka.dir/__/src/KafkaProducer.cpp.o
kafka/linux_prj/libkafka.a: kafka/linux_prj/CMakeFiles/kafka.dir/build.make
kafka/linux_prj/libkafka.a: kafka/linux_prj/CMakeFiles/kafka.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/shiyunjie/Documents/work/server/qbao/qbim/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX static library libkafka.a"
	cd /Users/shiyunjie/Documents/work/server/qbao/qbim/cmake-build-debug/kafka/linux_prj && $(CMAKE_COMMAND) -P CMakeFiles/kafka.dir/cmake_clean_target.cmake
	cd /Users/shiyunjie/Documents/work/server/qbao/qbim/cmake-build-debug/kafka/linux_prj && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/kafka.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
kafka/linux_prj/CMakeFiles/kafka.dir/build: kafka/linux_prj/libkafka.a

.PHONY : kafka/linux_prj/CMakeFiles/kafka.dir/build

kafka/linux_prj/CMakeFiles/kafka.dir/requires: kafka/linux_prj/CMakeFiles/kafka.dir/__/src/KafkaMgr.cpp.o.requires
kafka/linux_prj/CMakeFiles/kafka.dir/requires: kafka/linux_prj/CMakeFiles/kafka.dir/__/src/KafkaProducer.cpp.o.requires

.PHONY : kafka/linux_prj/CMakeFiles/kafka.dir/requires

kafka/linux_prj/CMakeFiles/kafka.dir/clean:
	cd /Users/shiyunjie/Documents/work/server/qbao/qbim/cmake-build-debug/kafka/linux_prj && $(CMAKE_COMMAND) -P CMakeFiles/kafka.dir/cmake_clean.cmake
.PHONY : kafka/linux_prj/CMakeFiles/kafka.dir/clean

kafka/linux_prj/CMakeFiles/kafka.dir/depend:
	cd /Users/shiyunjie/Documents/work/server/qbao/qbim/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/shiyunjie/Documents/work/server/qbao/qbim /Users/shiyunjie/Documents/work/server/qbao/qbim/kafka/linux_prj /Users/shiyunjie/Documents/work/server/qbao/qbim/cmake-build-debug /Users/shiyunjie/Documents/work/server/qbao/qbim/cmake-build-debug/kafka/linux_prj /Users/shiyunjie/Documents/work/server/qbao/qbim/cmake-build-debug/kafka/linux_prj/CMakeFiles/kafka.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : kafka/linux_prj/CMakeFiles/kafka.dir/depend

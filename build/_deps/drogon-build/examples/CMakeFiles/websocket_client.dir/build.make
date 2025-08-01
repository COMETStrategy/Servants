# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 4.0

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
CMAKE_COMMAND = /opt/homebrew/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Volumes/CaseSensitiveDisk/Development/Servants/Console

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Volumes/CaseSensitiveDisk/Development/Servants/build

# Include any dependencies generated for this target.
include _deps/drogon-build/examples/CMakeFiles/websocket_client.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include _deps/drogon-build/examples/CMakeFiles/websocket_client.dir/compiler_depend.make

# Include the progress variables for this target.
include _deps/drogon-build/examples/CMakeFiles/websocket_client.dir/progress.make

# Include the compile flags for this target's objects.
include _deps/drogon-build/examples/CMakeFiles/websocket_client.dir/flags.make

_deps/drogon-build/examples/CMakeFiles/websocket_client.dir/codegen:
.PHONY : _deps/drogon-build/examples/CMakeFiles/websocket_client.dir/codegen

_deps/drogon-build/examples/CMakeFiles/websocket_client.dir/websocket_client/WebSocketClient.cc.o: _deps/drogon-build/examples/CMakeFiles/websocket_client.dir/flags.make
_deps/drogon-build/examples/CMakeFiles/websocket_client.dir/websocket_client/WebSocketClient.cc.o: /Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/examples/websocket_client/WebSocketClient.cc
_deps/drogon-build/examples/CMakeFiles/websocket_client.dir/websocket_client/WebSocketClient.cc.o: _deps/drogon-build/examples/CMakeFiles/websocket_client.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Volumes/CaseSensitiveDisk/Development/Servants/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object _deps/drogon-build/examples/CMakeFiles/websocket_client.dir/websocket_client/WebSocketClient.cc.o"
	cd /Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-build/examples && /opt/homebrew/bin/g++-13 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT _deps/drogon-build/examples/CMakeFiles/websocket_client.dir/websocket_client/WebSocketClient.cc.o -MF CMakeFiles/websocket_client.dir/websocket_client/WebSocketClient.cc.o.d -o CMakeFiles/websocket_client.dir/websocket_client/WebSocketClient.cc.o -c /Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/examples/websocket_client/WebSocketClient.cc

_deps/drogon-build/examples/CMakeFiles/websocket_client.dir/websocket_client/WebSocketClient.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/websocket_client.dir/websocket_client/WebSocketClient.cc.i"
	cd /Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-build/examples && /opt/homebrew/bin/g++-13 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/examples/websocket_client/WebSocketClient.cc > CMakeFiles/websocket_client.dir/websocket_client/WebSocketClient.cc.i

_deps/drogon-build/examples/CMakeFiles/websocket_client.dir/websocket_client/WebSocketClient.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/websocket_client.dir/websocket_client/WebSocketClient.cc.s"
	cd /Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-build/examples && /opt/homebrew/bin/g++-13 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/examples/websocket_client/WebSocketClient.cc -o CMakeFiles/websocket_client.dir/websocket_client/WebSocketClient.cc.s

# Object files for target websocket_client
websocket_client_OBJECTS = \
"CMakeFiles/websocket_client.dir/websocket_client/WebSocketClient.cc.o"

# External object files for target websocket_client
websocket_client_EXTERNAL_OBJECTS =

_deps/drogon-build/examples/websocket_client: _deps/drogon-build/examples/CMakeFiles/websocket_client.dir/websocket_client/WebSocketClient.cc.o
_deps/drogon-build/examples/websocket_client: _deps/drogon-build/examples/CMakeFiles/websocket_client.dir/build.make
_deps/drogon-build/examples/websocket_client: _deps/drogon-build/libdrogon.a
_deps/drogon-build/examples/websocket_client: _deps/drogon-build/trantor/libtrantor.a
_deps/drogon-build/examples/websocket_client: /opt/homebrew/Cellar/openssl@3/3.5.0/lib/libssl.dylib
_deps/drogon-build/examples/websocket_client: /opt/homebrew/Cellar/openssl@3/3.5.0/lib/libcrypto.dylib
_deps/drogon-build/examples/websocket_client: /opt/homebrew/lib/libcares.dylib
_deps/drogon-build/examples/websocket_client: /opt/homebrew/lib/libjsoncpp.dylib
_deps/drogon-build/examples/websocket_client: /opt/homebrew/lib/libbrotlidec.dylib
_deps/drogon-build/examples/websocket_client: /opt/homebrew/lib/libbrotlienc.dylib
_deps/drogon-build/examples/websocket_client: /opt/homebrew/lib/libbrotlicommon.dylib
_deps/drogon-build/examples/websocket_client: /opt/homebrew/lib/postgresql@14/libpq.dylib
_deps/drogon-build/examples/websocket_client: /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/lib/libsqlite3.tbd
_deps/drogon-build/examples/websocket_client: /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/lib/libz.tbd
_deps/drogon-build/examples/websocket_client: _deps/drogon-build/examples/CMakeFiles/websocket_client.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Volumes/CaseSensitiveDisk/Development/Servants/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable websocket_client"
	cd /Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-build/examples && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/websocket_client.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
_deps/drogon-build/examples/CMakeFiles/websocket_client.dir/build: _deps/drogon-build/examples/websocket_client
.PHONY : _deps/drogon-build/examples/CMakeFiles/websocket_client.dir/build

_deps/drogon-build/examples/CMakeFiles/websocket_client.dir/clean:
	cd /Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-build/examples && $(CMAKE_COMMAND) -P CMakeFiles/websocket_client.dir/cmake_clean.cmake
.PHONY : _deps/drogon-build/examples/CMakeFiles/websocket_client.dir/clean

_deps/drogon-build/examples/CMakeFiles/websocket_client.dir/depend:
	cd /Volumes/CaseSensitiveDisk/Development/Servants/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Volumes/CaseSensitiveDisk/Development/Servants/Console /Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/examples /Volumes/CaseSensitiveDisk/Development/Servants/build /Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-build/examples /Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-build/examples/CMakeFiles/websocket_client.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : _deps/drogon-build/examples/CMakeFiles/websocket_client.dir/depend


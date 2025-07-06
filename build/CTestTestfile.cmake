# CMake generated Testfile for 
# Source directory: /Volumes/CaseSensitiveDisk/Development/Servants/Console
# Build directory: /Volumes/CaseSensitiveDisk/Development/Servants/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[servant-console-tests]=] "/Volumes/CaseSensitiveDisk/Development/Servants/Console/output/macOS/servant-console-tests")
set_tests_properties([=[servant-console-tests]=] PROPERTIES  _BACKTRACE_TRIPLES "/Volumes/CaseSensitiveDisk/Development/Servants/Console/CMakeLists.txt;104;add_test;/Volumes/CaseSensitiveDisk/Development/Servants/Console/CMakeLists.txt;0;")
subdirs("_deps/drogon-build")
subdirs("_deps/googletest-build")

# Install script for directory: /Users/danaklamut/ClionProjects/iondb

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "RelWithDebInfo")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/Users/danaklamut/ClionProjects/iondb/cmake-build-relwithdebinfo/src/tests/cmake_install.cmake")
  include("/Users/danaklamut/ClionProjects/iondb/cmake-build-relwithdebinfo/src/tests/unit/dictionary/cmake_install.cmake")
  include("/Users/danaklamut/ClionProjects/iondb/cmake-build-relwithdebinfo/examples/cmake_install.cmake")
  include("/Users/danaklamut/ClionProjects/iondb/cmake-build-relwithdebinfo/src/util/lfsr/cmake_install.cmake")
  include("/Users/danaklamut/ClionProjects/iondb/cmake-build-relwithdebinfo/src/iinq/cmake_install.cmake")
  include("/Users/danaklamut/ClionProjects/iondb/cmake-build-relwithdebinfo/src/dictionary/bpp_tree/cmake_install.cmake")
  include("/Users/danaklamut/ClionProjects/iondb/cmake-build-relwithdebinfo/src/dictionary/flat_file/cmake_install.cmake")
  include("/Users/danaklamut/ClionProjects/iondb/cmake-build-relwithdebinfo/src/dictionary/open_address_file_hash/cmake_install.cmake")
  include("/Users/danaklamut/ClionProjects/iondb/cmake-build-relwithdebinfo/src/dictionary/open_address_hash/cmake_install.cmake")
  include("/Users/danaklamut/ClionProjects/iondb/cmake-build-relwithdebinfo/src/dictionary/skip_list/cmake_install.cmake")
  include("/Users/danaklamut/ClionProjects/iondb/cmake-build-relwithdebinfo/src/tests/unit/iinq/cmake_install.cmake")
  include("/Users/danaklamut/ClionProjects/iondb/cmake-build-relwithdebinfo/src/tests/unit/dictionary/bpp_tree/cmake_install.cmake")
  include("/Users/danaklamut/ClionProjects/iondb/cmake-build-relwithdebinfo/src/tests/unit/dictionary/flat_file/cmake_install.cmake")
  include("/Users/danaklamut/ClionProjects/iondb/cmake-build-relwithdebinfo/src/tests/unit/dictionary/open_address_file_hash/cmake_install.cmake")
  include("/Users/danaklamut/ClionProjects/iondb/cmake-build-relwithdebinfo/src/tests/unit/dictionary/open_address_hash/cmake_install.cmake")
  include("/Users/danaklamut/ClionProjects/iondb/cmake-build-relwithdebinfo/src/tests/unit/dictionary/skip_list/cmake_install.cmake")
  include("/Users/danaklamut/ClionProjects/iondb/cmake-build-relwithdebinfo/src/tests/behaviour/dictionary/cmake_install.cmake")
  include("/Users/danaklamut/ClionProjects/iondb/cmake-build-relwithdebinfo/src/tests/behaviour/dictionary/flat_file/cmake_install.cmake")
  include("/Users/danaklamut/ClionProjects/iondb/cmake-build-relwithdebinfo/src/tests/behaviour/dictionary/skip_list/cmake_install.cmake")
  include("/Users/danaklamut/ClionProjects/iondb/cmake-build-relwithdebinfo/src/tests/behaviour/dictionary/bpp_tree/cmake_install.cmake")
  include("/Users/danaklamut/ClionProjects/iondb/cmake-build-relwithdebinfo/src/tests/behaviour/dictionary/open_address_hash/cmake_install.cmake")
  include("/Users/danaklamut/ClionProjects/iondb/cmake-build-relwithdebinfo/src/tests/behaviour/dictionary/open_address_file_hash/cmake_install.cmake")
  include("/Users/danaklamut/ClionProjects/iondb/cmake-build-relwithdebinfo/src/cpp_wrapper/cmake_install.cmake")
  include("/Users/danaklamut/ClionProjects/iondb/cmake-build-relwithdebinfo/src/tests/unit/cpp_wrapper/cmake_install.cmake")
  include("/Users/danaklamut/ClionProjects/iondb/cmake-build-relwithdebinfo/src/tests/integration/cpp_wrapper/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/Users/danaklamut/ClionProjects/iondb/cmake-build-relwithdebinfo/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")

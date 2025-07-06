# Install script for directory: /Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src

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
    set(CMAKE_INSTALL_CONFIG_NAME "")
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

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-build/trantor/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-build/examples/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-build/drogon_ctl/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "lib" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-build/libdrogon.a")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libdrogon.a" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libdrogon.a")
    execute_process(COMMAND "/usr/bin/ranlib" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libdrogon.a")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/drogon" TYPE FILE FILES
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/Attribute.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/CacheMap.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/Cookie.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/DrClassMap.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/DrObject.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/DrTemplate.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/DrTemplateBase.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/HttpAppFramework.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/HttpBinder.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/HttpClient.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/HttpController.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/HttpFilter.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/HttpMiddleware.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/HttpRequest.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/RequestStream.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/HttpResponse.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/HttpSimpleController.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/HttpTypes.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/HttpViewData.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/IntranetIpFilter.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/IOThreadStorage.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/LocalHostFilter.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/MultiPart.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/NotFound.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/Session.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/UploadFile.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/WebSocketClient.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/WebSocketConnection.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/WebSocketController.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/drogon.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-build/lib/inc/drogon/version.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/drogon_callbacks.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/PubSubService.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/drogon_test.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/RateLimiter.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-build/exports/drogon/exports.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/drogon/orm" TYPE FILE FILES
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/orm_lib/inc/drogon/orm/ArrayParser.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/orm_lib/inc/drogon/orm/BaseBuilder.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/orm_lib/inc/drogon/orm/Criteria.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/orm_lib/inc/drogon/orm/DbClient.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/orm_lib/inc/drogon/orm/DbConfig.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/orm_lib/inc/drogon/orm/DbListener.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/orm_lib/inc/drogon/orm/DbTypes.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/orm_lib/inc/drogon/orm/Exception.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/orm_lib/inc/drogon/orm/Field.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/orm_lib/inc/drogon/orm/FunctionTraits.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/orm_lib/inc/drogon/orm/Mapper.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/orm_lib/inc/drogon/orm/CoroMapper.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/orm_lib/inc/drogon/orm/Result.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/orm_lib/inc/drogon/orm/ResultIterator.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/orm_lib/inc/drogon/orm/Row.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/orm_lib/inc/drogon/orm/RowIterator.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/orm_lib/inc/drogon/orm/SqlBinder.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/orm_lib/inc/drogon/orm/RestfulController.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/drogon/nosql" TYPE FILE FILES
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/nosql_lib/redis/inc/drogon/nosql/RedisClient.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/nosql_lib/redis/inc/drogon/nosql/RedisResult.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/nosql_lib/redis/inc/drogon/nosql/RedisSubscriber.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/nosql_lib/redis/inc/drogon/nosql/RedisException.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/drogon/utils" TYPE FILE FILES
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/utils/coroutine.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/utils/FunctionTraits.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/utils/HttpConstraint.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/utils/OStringStream.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/utils/Utilities.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/utils/monitoring.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/drogon/utils/monitoring" TYPE FILE FILES
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/utils/monitoring/Counter.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/utils/monitoring/Metric.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/utils/monitoring/Registry.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/utils/monitoring/Collector.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/utils/monitoring/Sample.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/utils/monitoring/Gauge.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/utils/monitoring/Histogram.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/drogon/plugins" TYPE FILE FILES
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/plugins/Plugin.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/plugins/Redirector.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/plugins/SecureSSLRedirector.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/plugins/AccessLogger.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/plugins/RealIpResolver.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/plugins/Hodor.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/plugins/SlashRemover.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/plugins/GlobalFilters.h"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/lib/inc/drogon/plugins/PromExporter.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Drogon" TYPE FILE FILES
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-build/CMakeFiles/DrogonConfig.cmake"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-build/DrogonConfigVersion.cmake"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/cmake_modules/FindUUID.cmake"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/cmake_modules/FindJsoncpp.cmake"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/cmake_modules/FindSQLite3.cmake"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/cmake_modules/FindMySQL.cmake"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/cmake_modules/Findpg.cmake"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/cmake_modules/FindBrotli.cmake"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/cmake_modules/Findcoz-profiler.cmake"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/cmake_modules/FindHiredis.cmake"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/cmake_modules/FindFilesystem.cmake"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/cmake/DrogonUtilities.cmake"
    "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-src/cmake/ParseAndAddDrogonTests.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Drogon/DrogonTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Drogon/DrogonTargets.cmake"
         "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-build/CMakeFiles/Export/9fe51f2b716a6bd37518a903e3e9a4cf/DrogonTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Drogon/DrogonTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Drogon/DrogonTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Drogon" TYPE FILE FILES "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-build/CMakeFiles/Export/9fe51f2b716a6bd37518a903e3e9a4cf/DrogonTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^()$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Drogon" TYPE FILE FILES "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-build/CMakeFiles/Export/9fe51f2b716a6bd37518a903e3e9a4cf/DrogonTargets-noconfig.cmake")
  endif()
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/Volumes/CaseSensitiveDisk/Development/Servants/build/_deps/drogon-build/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()

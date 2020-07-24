# list of source files
set(messaging-src MapWrap.cc CppWrapper.cc messaging-client.c messaging-server.c)


# load package helper for generating cmake CONFIG packages
include (CMakePackageConfigHelpers)

# where to install files for "find_package"
set (messaging-pkg "share/cmake/messaging")

#
# library version set here (e.g. for shared libs).
#
set (MESSAGING_VERSION_MAJOR 0)
set (MESSAGING_VERSION_MINOR 1)
set (MESSAGING_VERSION_PATCH 0)
set (messaging-vers "${MESSAGING_VERSION_MAJOR}.${MESSAGING_VERSION_MINOR}")
set (MESSAGING_VERSION "${messaging-vers}.${MESSAGING_VERSION_PATCH}")

add_library(messaging ${messaging-src})
target_link_libraries (messaging margo)
target_include_directories (messaging PUBLIC $<INSTALL_INTERFACE:include>)

# local include's BEFORE, in case old incompatable .h files in prefix/include
target_include_directories (messaging BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)

# for shared libs, establish the lib version
set_target_properties (messaging
    PROPERTIES VERSION ${MESSAGING_VERSION}
    SOVERSION ${MESSAGING_VERSION_MAJOR})

#
# installation stuff (packaging and install commands)
#
write_basic_package_version_file(
    "messaging-config-version.cmake"
    VERSION ${MESSAGING_VERSION}
    COMPATIBILITY AnyNewerVersion)

# generate our config file for find_package()
configure_file (messaging-config.cmake.in messaging-config.cmake @ONLY)

# some bits for the pkg-config file
set (DEST_DIR "${CMAKE_INSTALL_PREFIX}")
set (PRIVATE_LIBS "-lmessaging -libstdc++")
configure_file ("messaging.pc.in" "messaging.pc" @ONLY)

#
# "make install" rules
#
install (TARGETS messaging EXPORT messaging-targets
         ARCHIVE DESTINATION lib
         LIBRARY DESTINATION lib)
install (EXPORT messaging-targets
         DESTINATION ${messaging-pkg}
         FILE "messaging-targets.cmake")
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/messaging-config.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/messaging-config-version.cmake"
               "../cmake/xpkg-import.cmake"
         DESTINATION ${messaging-pkg} )
install (DIRECTORY ../include
         DESTINATION ${DEST_DIR}
         FILES_MATCHING PATTERN "*messaging-*.h")
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/messaging.pc"
		DESTINATION "lib/pkgconfig/")

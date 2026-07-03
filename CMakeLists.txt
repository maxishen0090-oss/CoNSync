 cmake_minimum_required(VERSION 3.10)
 project(CoNSync)
 
include_directories( playfair llhttp )
 include_directories( lib )
add_subdirectory(lib/playfair)
add_subdirectory(lib/llhttp)
 add_subdirectory(lib/dns_sd)

message( STATUS "*** CFLAGS \"" ${CMAKE_C_FLAGS} "\" from build environment will be postpended to CMAKE_CFLAGS" )

# Common x86/x86_64 cflags
if( NOT NO_MARCH_NATIVE AND CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)" )
  set( CMAKE_C_FLAGS "-O3 -mtune=generic ${CMAKE_C_FLAGS}" )
  message( STATUS "Using CFLAGS with  -march=native" )
  message( STATUS "*** ONLY USE THIS WHEN COMPILING ON THE MACHINE THAT WILL RUN CoNSync" )
  message( STATUS "  run \"cmake -DNO_MARCH_NATIVE=ON\" to switch off this compiler option" )
else()
  message( STATUS "Not using -march=native" )
  set( CMAKE_C_FLAGS "-O2 ${CMAKE_C_FLAGS}" )
endif()

# Common Linux cflags
if ( UNIX  AND NOT APPLE )
  set( CMAKE_C_FLAGS "-DSTANDALONE -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -DTARGET_POSIX -D_LINUX -fPIC -DPIC -D_REENTRANT -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64  -Wall ${CMAKE_C_FLAGS}" )
endif()

if ( WIN32 )
  message( STATUS "Building for Windows " )
  set( CMAKE_C_FLAGS "-DSTANDALONE -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -DTARGET_POSIX -D_WIN32 -fPIC -DPIC -D_REENTRANT -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -Wall ${CMAKE_C_FLAGS}" )
endif()

message( STATUS "using CMAKE_CFLAGS: " ${CMAKE_C_FLAGS} )

#activate the NOHOLD feature to drop existing connections if a third connection is made 
add_definitions( -DNOHOLD )

INCLUDE (CheckIncludeFiles)
if( WIN32 )
CHECK_INCLUDE_FILES ("winsock2.h" WINSOCK2 )
else()
# for BSD Unix (e.g. FreeBSD)
CHECK_INCLUDE_FILES ("sys/endian.h" BSD )
if ( BSD )
  add_definitions( -DSYS_ENDIAN_H )
endif ( BSD )
endif()

if( APPLE )
  set( ENV{PKG_CONFIG_PATH} "/usr/local/lib/pkgconfig" )
  set( ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:/opt/homebrew/lib/pkgconfig" )
  set( ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:$ENV{HOMEBREW_PREFIX}/lib/pkgconfig" )
  set( ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:/opt/local/lib/pkgconfig/" )
  set( ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:/usr/local/opt/openssl@3/lib/pkgconfig" )
  set( ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:/opt/homebrew/opt/openssl@3/lib/pkgconfig" )
  set( ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:$ENV{HOMEBREW_PREFIX}/opt/openssl@3/lib/pkgconfig" )
  message( "PKG_CONFIG_PATH (Apple, lib) = " $ENV{PKG_CONFIG_PATH} )
  find_program( PKG_CONFIG_EXECUTABLE  pkg-config  PATHS /Library/FrameWorks/GStreamer.framework/Commands )
  message( "PKG_CONFIG_EXECUTABLE " ${PKG_CONFIG_EXECUTABLE} )
endif()
find_package(PkgConfig REQUIRED)
 pkg_check_modules(GLIB2 REQUIRED glib-2.0)

aux_source_directory(. play_src)
 aux_source_directory(lib lib_src)
 list(APPEND play_src ${lib_src})
list(REMOVE_ITEM play_src ${CMAKE_CURRENT_SOURCE_DIR}/CoNSync.cpp)
set(DIR_SRCS ${play_src})

add_library( airplay  STATIC
             ${DIR_SRCS}
           )

if ( APPLE )
  target_link_libraries( airplay 
                         pthread
                         playfair
                         dnssd
                         llhttp )
elseif( WIN32 )
  target_link_libraries( airplay 
                         pthread
                         playfair
                         dnssd
                         llhttp
                         wsock32
                         iphlpapi
                         ws2_32 )
else()
  target_link_libraries( airplay PUBLIC
          pthread
          playfair
          dnssd
          llhttp )
endif()

# libplist

if( APPLE )
   pkg_search_module(PLIST REQUIRED libplist-2.0)
   find_library( LIBPLIST libplist-2.0.a REQUIRED )
   message( STATUS "(Static linking) LIBPLIST " ${LIBPLIST} )
   target_link_libraries ( airplay  ${LIBPLIST} )
elseif( WIN32)
pkg_search_module(PLIST REQUIRED libplist-2.0)
  find_library( LIBPLIST ${PLIST_LIBRARIES} PATH ${PLIST_LIBDIR} )
  target_link_libraries ( airplay ${LIBPLIST} )
else ()
  pkg_search_module(PLIST libplist>=2.0)
  if(NOT PLIST_FOUND)
    pkg_search_module(PLIST REQUIRED libplist-2.0)
  endif()
  find_library( LIBPLIST ${PLIST_LIBRARIES} PATH ${PLIST_LIBDIR} )
  target_link_libraries ( airplay PUBLIC ${LIBPLIST} )
endif()
if ( PLIST_FOUND )
   message( STATUS "found libplist-${PLIST_VERSION}")
   pkg_check_modules ( PLIST_21 libplist-2.0>=2.1.0 )
   if ( PLIST_21_FOUND )
     add_definitions( -DPLIST_210 )
   endif()
   pkg_check_modules ( PLIST_23 libplist-2.0>=2.3.0 )
   if ( PLIST_23_FOUND )
     add_definitions( -DPLIST_230 )
   endif()
endif()
target_include_directories( airplay PRIVATE ${PLIST_INCLUDE_DIRS} )
 target_include_directories( airplay PRIVATE ${GLIB2_INCLUDE_DIRS} )

#libcrypto
if( APPLE )
  pkg_check_modules( OPENSSL REQUIRED openssl>=1.1.1)
  message( "OPENSSL_LIBRARY_DIRS " ${OPENSSL_LIBRARY_DIRS} )
  message( "OPENSSL_INCLUDE_DIRS " ${OPENSSL_INCLUDE_DIRS} )
  find_library( LIBCRYPTO libcrypto.a PATHS ${OPENSSL_LIBRARY_DIRS} REQUIRED )
  message( "(Static linking) LIBCRYPTO "  ${LIBCRYPTO}  )
  target_link_libraries( airplay ${LIBCRYPTO} )
  if( LIBCRYPTO MATCHES "/opt/local/lib/libcrypto.a" )
    find_library( LIBZ libz.a)
    message("(MacPorts) LIBZ= " ${LIBZ} )
    target_link_libraries( airplay  ${LIBZ} )
  endif()
  target_include_directories( airplay PRIVATE ${OPENSSL_INCLUDE_DIRS} )
elseif( WIN32 )
  find_package(OpenSSL 1.1.1 REQUIRED)
  target_compile_definitions( airplay PUBLIC OPENSSL_API_COMPAT=0x10101000L )
  target_link_libraries( airplay  OpenSSL::Crypto )
else()
  find_package(OpenSSL 1.1.1 REQUIRED)
  target_compile_definitions( airplay PUBLIC OPENSSL_API_COMPAT=0x10101000L )
  target_link_libraries( airplay PUBLIC OpenSSL::Crypto )
endif()

 # Build the CoNSync executable
 add_subdirectory(renderers)
 if(WIN32)
  add_executable(CoNSync CoNSync.cpp)
  target_link_libraries(CoNSync PUBLIC airplay renderers)
else()
  add_executable(CoNSync CoNSync.cpp)
  target_link_libraries(CoNSync PUBLIC airplay renderers)
endif()
 
# Icon (Windows only)
if(WIN32)
  find_program(WINDRES_EXECUTABLE windres)
  if(WINDRES_EXECUTABLE)
    add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/CoNSync_res.o
      COMMAND ${WINDRES_EXECUTABLE}
      ARGS -i ${CMAKE_CURRENT_SOURCE_DIR}/CoNSync.rc -o ${CMAKE_CURRENT_BINARY_DIR}/CoNSync_res.o
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/CoNSync.rc ${CMAKE_CURRENT_SOURCE_DIR}/CoNSync.ico
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "Compiling Windows resource file (icon)"
    )
    target_sources(CoNSync PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/CoNSync_res.o)
  endif()
 endif()

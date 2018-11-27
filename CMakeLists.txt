cmake_minimum_required(VERSION 3.10)

project(graft_server)

option(OPT_BUILD_TESTS "Build tests." OFF)
option(ENABLE_SYSLOG "SYSLOG support. It can be compiled for UNIX-like platforms only." OFF)

set(EXTERNAL_CRYPTONODE OFF CACHE BINARY "Disables building the embedded GraftNetwork project; advanced usage only.  include/library directories to an existing build must be properly specified.")

configure_file(
    "${PROJECT_SOURCE_DIR}/CMakeConfig.h.in"
    "${PROJECT_BINARY_DIR}/CMakeConfig.h"
    )

find_package(Threads REQUIRED)

add_subdirectory(modules/libr3 EXCLUDE_FROM_ALL)

if (NOT EXTERNAL_CRYPTONODE)
  set(GRAFT_BINARY_DIR "${PROJECT_BINARY_DIR}")
  add_subdirectory(modules/cryptonode)
endif()

set(CMAKE_CXX_STANDARD 14)

add_definitions(-DGN_ENABLE_EVENTFD=1 -DMG_USE_READ_WRITE)

set(GS_SOURCES
    ${PROJECT_SOURCE_DIR}/src/task.cpp
    ${PROJECT_SOURCE_DIR}/src/connection.cpp
    ${PROJECT_SOURCE_DIR}/src/inout.cpp
    ${PROJECT_SOURCE_DIR}/src/router.cpp
    ${PROJECT_SOURCE_DIR}/src/requesttools.cpp
    ${PROJECT_SOURCE_DIR}/src/requestdefines.cpp
    ${PROJECT_SOURCE_DIR}/src/requests.cpp
    ${PROJECT_SOURCE_DIR}/src/requests/salerequest.cpp
    ${PROJECT_SOURCE_DIR}/src/requests/salestatusrequest.cpp
    ${PROJECT_SOURCE_DIR}/src/requests/rejectsalerequest.cpp
    ${PROJECT_SOURCE_DIR}/src/requests/saledetailsrequest.cpp
    ${PROJECT_SOURCE_DIR}/src/requests/payrequest.cpp
    ${PROJECT_SOURCE_DIR}/src/requests/paystatusrequest.cpp
    ${PROJECT_SOURCE_DIR}/src/requests/rejectpayrequest.cpp
    ${PROJECT_SOURCE_DIR}/src/requests/getinforequest.cpp
    ${PROJECT_SOURCE_DIR}/src/requests/sendrawtxrequest.cpp
    ${PROJECT_SOURCE_DIR}/src/requests/authorizertatxrequest.cpp
    ${PROJECT_SOURCE_DIR}/src/requests/sendsupernodeannouncerequest.cpp
    ${PROJECT_SOURCE_DIR}/src/requests/forwardrequest.cpp
    ${PROJECT_SOURCE_DIR}/src/requests/healthcheckrequest.cpp
    ${PROJECT_SOURCE_DIR}/src/requests/debug.cpp
    ${PROJECT_SOURCE_DIR}/src/mongoosex.cpp
    ${PROJECT_SOURCE_DIR}/src/rta/supernode.cpp
    ${PROJECT_SOURCE_DIR}/src/rta/fullsupernodelist.cpp
    ${PROJECT_SOURCE_DIR}/src/rta/DaemonRpcClient.cpp
    ${PROJECT_SOURCE_DIR}/src/common/utils.cpp
    ${PROJECT_SOURCE_DIR}/modules/mongoose/mongoose.c
    ${PROJECT_SOURCE_DIR}/src/backtrace.cpp
    ${PROJECT_SOURCE_DIR}/src/log.cpp
    )

set(GS_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/modules/mongoose
    ${r3_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/requests
    ${PROJECT_SOURCE_DIR}/include/rta
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}
    )

set(GS_LIBS
    r3
    lib3rdparty
    pcre
    dl
    )


if(NOT EXTERNAL_CRYPTONODE)
  set(CRYPTONODE_INCLUDE_DIRS
    ${graftnetwork_SOURCE_DIR}/contrib/epee/include
    ${graftnetwork_SOURCE_DIR}/external/easylogging++
    ${graftnetwork_SOURCE_DIR}/external
    ${graftnetwork_SOURCE_DIR}/src
    ${graftnetwork_BINARY_DIR}/version
    )
endif()

set(CRYPTONODE_LIBS
    easylogging
    wallet
    utils
    mnemonics
    ringct
    epee
    cryptonote_core
    cryptonote_basic
    common
    unbound
    cncrypto
    )

set(COMMON_LIBS
    ssl
    crypto
    readline
    )

#set(Boost_USE_STATIC_LIBS ON)
#set(Boost_USE_MULTITHREADED ON)
#set(Boost_USE_STATIC_RUNTIME ON)

find_package(Boost 1.65 QUIET REQUIRED COMPONENTS system chrono filesystem program_options serialization thread regex)

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find Boost libraries, please make sure you have installed Boost or libboost-all-dev (1.65) or the equivalent")
elseif(Boost_FOUND)
    message(STATUS "Found Boost Version: ${Boost_VERSION}")
endif()

include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
include_directories(${CRYPTONODE_INCLUDE_DIRS})

add_library(graftserver STATIC ${GS_SOURCES})
target_link_libraries(graftserver PRIVATE
    ${CRYPTONODE_LIBS}
    ${GS_LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_CHRONO_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_SERIALIZATION_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    ${Boost_REGEX_LIBRARY}
    ${COMMON_LIBS}
    )

target_include_directories(graftserver PRIVATE
    ${GS_INCLUDE_DIRS}
    )

target_compile_definitions(graftserver PRIVATE MG_ENABLE_COAP=1 -DMONERO_DEFAULT_LOG_CATEGORY="supernode")
if(ENABLE_SYSLOG)
    target_compile_definitions(graftserver PRIVATE -DELPP_SYSLOG)
endif()

add_executable(graft_server
    ${PROJECT_SOURCE_DIR}/src/server.cpp
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    )
install(TARGETS graft_server DESTINATION bin)
target_link_libraries(graft_server PRIVATE graftserver)
target_include_directories(graft_server PRIVATE ${GS_INCLUDE_DIRS})
set_target_properties(graft_server PROPERTIES LINK_FLAGS "-rdynamic")


##################
### tests section
if (OPT_BUILD_TESTS)
        message("==> Build tests section included")
        add_subdirectory(modules/googletest EXCLUDE_FROM_ALL)

        add_executable(graft_server_test
                ${PROJECT_SOURCE_DIR}/test/graft_server_test.cpp
                ${PROJECT_SOURCE_DIR}/test/json_rpc_test.cpp
                ${PROJECT_SOURCE_DIR}/test/json_test.cpp
                ${PROJECT_SOURCE_DIR}/test/cryptonode_handlers_test.cpp
                ${PROJECT_SOURCE_DIR}/test/rta_classes_test.cpp
                ${PROJECT_SOURCE_DIR}/test/main.cpp
        )

        target_include_directories(graft_server_test PRIVATE
                ${GS_INCLUDE_DIRS}
                ${googletest_SOURCE_DIR}/include
                ${CRYPTONODE_INCLUDES}
        )

        target_link_libraries(graft_server_test
            PRIVATE
            graftserver
            gtest
            )


        get_target_property(GRAFT_SERVER_DEFINITIONS graft_server COMPILE_DEFINITIONS)
        target_compile_definitions(graft_server_test PRIVATE ${GRAFT_SERVER_DEFINITIONS})

        file(COPY data/ DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

endif (OPT_BUILD_TESTS)

# copy config file to build directory
file(INSTALL data/config.ini DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

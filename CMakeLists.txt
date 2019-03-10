cmake_minimum_required(VERSION 3.10)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

if(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    message("==> The configuration is ${CMAKE_BUILD_TYPE}. Debug info will be extracted into separate files.")

    function (add_executable _name)
        _add_executable(${ARGV})

        if (TARGET ${_name})
            add_custom_command(TARGET ${_name} POST_BUILD
                COMMAND echo "$<TARGET_FILE_NAME:${_name}>: extracting debug info"
                COMMAND ${CMAKE_COMMAND} -E chdir $<TARGET_FILE_DIR:${_name}> objcopy --only-keep-debug "$<TARGET_FILE_NAME:${_name}>" "$<TARGET_FILE_NAME:${_name}>.debug"
                COMMAND ${CMAKE_COMMAND} -E chdir $<TARGET_FILE_DIR:${_name}> strip --strip-debug --strip-unneeded "$<TARGET_FILE_NAME:${_name}>"
                COMMAND ${CMAKE_COMMAND} -E chdir $<TARGET_FILE_DIR:${_name}> objcopy --add-gnu-debuglink="$<TARGET_FILE_NAME:${_name}>.debug" "$<TARGET_FILE_NAME:${_name}>"
            )
        endif()
    endfunction()

    function (add_library _name _type)
        _add_library(${ARGV})

        if (TARGET ${_name} AND ${_type} STREQUAL SHARED)
            add_custom_command(TARGET ${_name} POST_BUILD
                COMMAND echo "$<TARGET_FILE_NAME:${_name}>: extracting debug info"
                COMMAND ${CMAKE_COMMAND} -E chdir $<TARGET_FILE_DIR:${_name}> objcopy --only-keep-debug "$<TARGET_FILE_NAME:${_name}>" "$<TARGET_FILE_NAME:${_name}>.debug"
                COMMAND ${CMAKE_COMMAND} -E chdir $<TARGET_FILE_DIR:${_name}> strip --strip-debug --strip-unneeded "$<TARGET_FILE_NAME:${_name}>"
                COMMAND ${CMAKE_COMMAND} -E chdir $<TARGET_FILE_DIR:${_name}> objcopy --add-gnu-debuglink="$<TARGET_FILE_NAME:${_name}>.debug" "$<TARGET_FILE_NAME:${_name}>"
            )
        endif()
    endfunction()

endif(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)

project(graft_server)

option(OPT_BUILD_TESTS "Build tests." OFF)
option(ENABLE_SYSLOG "SYSLOG support. It can be compiled for UNIX-like platforms only." OFF)

set(EXTERNAL_CRYPTONODE OFF CACHE BINARY "Disables building the embedded GraftNetwork project; advanced usage only.  include/library directories to an existing build must be properly specified.")

# Current GraftNetwork produces invalid results when compiled with AVX512 instructions; disable them
add_compile_options(-mno-avx512bw -mno-avx512cd -mno-avx512dq -mno-avx512f -mno-avx512vl)

set_property(GLOBAL PROPERTY ALLOW_DUPLICATE_CUSTOM_TARGETS ON)
include(version.cmake)

find_package(Threads REQUIRED)

add_subdirectory(modules/libr3 EXCLUDE_FROM_ALL)

if (NOT EXTERNAL_CRYPTONODE)
  set(GRAFT_BINARY_DIR "${PROJECT_BINARY_DIR}")
  add_subdirectory(modules/cryptonode)
endif()

set(CMAKE_CXX_STANDARD 17)

add_definitions(-DGN_ENABLE_EVENTFD=1 -DMG_USE_READ_WRITE)

set(GRAFT_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/modules/mongoose
    ${r3_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/requests
    ${PROJECT_SOURCE_DIR}/include/rta
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}
    )


if(NOT EXTERNAL_CRYPTONODE)
  set(CRYPTONODE_INCLUDE_DIRS
    ${graftnetwork_SOURCE_DIR}/contrib/epee/include
    ${graftnetwork_SOURCE_DIR}/external/easylogging++
    ${graftnetwork_SOURCE_DIR}/external
    ${graftnetwork_SOURCE_DIR}/src
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
include_directories(${CRYPTONODE_INCLUDE_DIRS} "${CMAKE_BINARY_DIR}/version")


### graftlet_lib
add_library(graftlet_lib
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/graft/GraftletLoader.cpp
    )

target_include_directories(graftlet_lib PRIVATE
    ${GRAFT_INCLUDE_DIRS}
    )

if(UNIX)
    target_link_libraries(graftlet_lib PRIVATE
        dl
        )
endif(UNIX)

set(GRAFTLETS_OUTPUT_DIR ${PROJECT_BINARY_DIR}/graftlets)
add_subdirectory(graftlets ${PROJECT_BINARY_DIR}/graftlets)

### graft library
add_library(graft
    ${PROJECT_SOURCE_DIR}/src/lib/graft/common/utils.cpp
    ${PROJECT_SOURCE_DIR}/src/lib/graft/backtrace.cpp
    ${PROJECT_SOURCE_DIR}/src/lib/graft/blacklist.cpp
    ${PROJECT_SOURCE_DIR}/src/lib/graft/connection.cpp
    ${PROJECT_SOURCE_DIR}/src/lib/graft/context.cpp
    ${PROJECT_SOURCE_DIR}/src/lib/graft/inout.cpp
    ${PROJECT_SOURCE_DIR}/src/lib/graft/log.cpp
    ${PROJECT_SOURCE_DIR}/src/lib/graft/mongoosex.cpp
    ${PROJECT_SOURCE_DIR}/src/lib/graft/router.cpp
    ${PROJECT_SOURCE_DIR}/src/lib/graft/task.cpp
    ${PROJECT_SOURCE_DIR}/modules/mongoose/mongoose.c
    ${PROJECT_SOURCE_DIR}/src/supernode/server.cpp
    ${PROJECT_SOURCE_DIR}/src/supernode/supernode.cpp
    ${PROJECT_SOURCE_DIR}/src/lib/graft/sys_info.cpp
    ${PROJECT_SOURCE_DIR}/src/lib/graft/sys_info_request.cpp
    )

add_dependencies(graft supernode_version)

target_include_directories(graft PRIVATE
    ${GRAFT_INCLUDE_DIRS}
    )

target_link_libraries(graft PRIVATE
    r3
    lib3rdparty
    graftlet_lib
    pcre
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    )

target_compile_definitions(graft PRIVATE MG_ENABLE_COAP=1 -DMONERO_DEFAULT_LOG_CATEGORY="supernode")
if(ENABLE_SYSLOG)
    target_compile_definitions(graft PRIVATE -DELPP_SYSLOG)
endif()

set_target_properties(graft PROPERTIES LINK_FLAGS "-Wl,-E -rdynamic")

### requests_common library
add_library(requests_common
    ${PROJECT_SOURCE_DIR}/src/lib/graft/requests.cpp
    ${PROJECT_SOURCE_DIR}/src/lib/graft/requesttools.cpp
    ${PROJECT_SOURCE_DIR}/src/lib/graft/requests/requestdefines.cpp
    ${PROJECT_SOURCE_DIR}/src/lib/graft/requests/health_check.cpp
    )

target_include_directories(requests_common PRIVATE
    ${GRAFT_INCLUDE_DIRS}
    )

target_link_libraries(requests_common PRIVATE
    ${CRYPTONODE_LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_CHRONO_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_SERIALIZATION_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    ${Boost_REGEX_LIBRARY}
    ${COMMON_LIBS}
    graft
    )

if(ENABLE_SYSLOG)
    target_compile_definitions(requests_common PRIVATE -DELPP_SYSLOG)
endif()

add_dependencies(requests_common graft)

### supernode_common library
add_library(supernode_common
    ${PROJECT_SOURCE_DIR}/src/supernode/requestdefines.cpp
    ${PROJECT_SOURCE_DIR}/src/supernode/requests.cpp
    ${PROJECT_SOURCE_DIR}/src/supernode/requests/authorize_rta_tx.cpp
    ${PROJECT_SOURCE_DIR}/src/supernode/requests/debug.cpp
    ${PROJECT_SOURCE_DIR}/src/supernode/requests/forward.cpp
    ${PROJECT_SOURCE_DIR}/src/supernode/requests/get_info.cpp
    ${PROJECT_SOURCE_DIR}/src/supernode/requests/pay.cpp
    ${PROJECT_SOURCE_DIR}/src/supernode/requests/pay_status.cpp
    ${PROJECT_SOURCE_DIR}/src/supernode/requests/reject_pay.cpp
    ${PROJECT_SOURCE_DIR}/src/supernode/requests/reject_sale.cpp
    ${PROJECT_SOURCE_DIR}/src/supernode/requests/sale_details.cpp
    ${PROJECT_SOURCE_DIR}/src/supernode/requests/sale.cpp
    ${PROJECT_SOURCE_DIR}/src/supernode/requests/sale_status.cpp
    ${PROJECT_SOURCE_DIR}/src/supernode/requests/send_raw_tx.cpp
    ${PROJECT_SOURCE_DIR}/src/supernode/requests/send_supernode_announce.cpp
    ${PROJECT_SOURCE_DIR}/src/supernode/requests/send_supernode_stakes.cpp
    ${PROJECT_SOURCE_DIR}/src/supernode/requests/send_transfer.cpp
    ${PROJECT_SOURCE_DIR}/src/supernode/requests/blockchain_based_list.cpp
    ${PROJECT_SOURCE_DIR}/src/rta/DaemonRpcClient.cpp
    ${PROJECT_SOURCE_DIR}/src/rta/fullsupernodelist.cpp
    ${PROJECT_SOURCE_DIR}/src/rta/supernode.cpp
    )

target_include_directories(supernode_common PRIVATE
    ${GRAFT_INCLUDE_DIRS}
    )

target_link_libraries(supernode_common PRIVATE
    ${CRYPTONODE_LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_CHRONO_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_SERIALIZATION_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    ${Boost_REGEX_LIBRARY}
    ${COMMON_LIBS}
    requests_common
    graft
    )

target_compile_definitions(supernode_common PRIVATE MG_ENABLE_COAP=1 -DMONERO_DEFAULT_LOG_CATEGORY="supernode")
if(ENABLE_SYSLOG)
    target_compile_definitions(supernode_common PRIVATE -DELPP_SYSLOG)
endif()

add_dependencies(supernode_common requests_common graft)

### supernode
add_executable(supernode_bin
    ${PROJECT_SOURCE_DIR}/src/supernode/main.cpp
    )

set_target_properties(supernode_bin PROPERTIES OUTPUT_NAME supernode)
set_target_properties(supernode_bin PROPERTIES LINK_FLAGS "-Wl,-E")

target_include_directories(supernode_bin PRIVATE
    ${GRAFT_INCLUDE_DIRS}
    )

target_link_libraries(supernode_bin PRIVATE
    graft
    supernode_common
    )

add_dependencies(supernode_bin graft supernode_common)



### wallet_server
add_executable(wallet_server
    ${PROJECT_SOURCE_DIR}/src/walletnode/main.cpp
    ${PROJECT_SOURCE_DIR}/src/walletnode/requests/wallet_requests.cpp
    ${PROJECT_SOURCE_DIR}/src/walletnode/server.cpp
    ${PROJECT_SOURCE_DIR}/src/walletnode/wallet_manager.cpp
    )

target_include_directories(wallet_server PRIVATE
    ${GRAFT_INCLUDE_DIRS}
    )

target_link_libraries(wallet_server PRIVATE
    ${Boost_SYSTEM_LIBRARY}
    graft
    requests_common
    )

add_dependencies(wallet_server graft requests_common)

install(TARGETS supernode_bin wallet_server DESTINATION bin)

##################
### tests section
if (OPT_BUILD_TESTS)
        message("==> Build tests section included")
        add_subdirectory(modules/googletest EXCLUDE_FROM_ALL)

        add_executable(supernode_test
            ${PROJECT_SOURCE_DIR}/test/upstream_test.cpp
            ${PROJECT_SOURCE_DIR}/test/blacklist_test.cpp
            ${PROJECT_SOURCE_DIR}/test/graft_server_test.cpp
            ${PROJECT_SOURCE_DIR}/test/graftlets_test.cpp
            ${PROJECT_SOURCE_DIR}/test/thread_pool_test.cpp
            ${PROJECT_SOURCE_DIR}/test/json_rpc_test.cpp
            ${PROJECT_SOURCE_DIR}/test/json_test.cpp
            ${PROJECT_SOURCE_DIR}/test/cryptonode_handlers_test.cpp
            ${PROJECT_SOURCE_DIR}/test/rta_classes_test.cpp
            ${PROJECT_SOURCE_DIR}/test/sys_info.cpp
            ${PROJECT_SOURCE_DIR}/test/strand_test.cpp
            ${PROJECT_SOURCE_DIR}/test/main.cpp
        )

        target_include_directories(supernode_test PRIVATE
            ${GRAFT_INCLUDE_DIRS}
            ${googletest_SOURCE_DIR}/include
            ${CRYPTONODE_INCLUDES}
        )

        target_link_libraries(supernode_test PRIVATE
            ${GT_LIBS}
            graft
            supernode_common
            )


        target_compile_definitions(supernode_test PRIVATE MG_ENABLE_COAP=1)
        add_dependencies(supernode_test graft supernode_common googletest)
        set_target_properties(supernode_test PROPERTIES LINK_FLAGS "-Wl,-E")
        if(ENABLE_SYSLOG)
            target_compile_definitions(supernode_test PRIVATE -DELPP_SYSLOG)
        endif()

        if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/test_wallet.keys)
            add_custom_command(
                TARGET supernode_test POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/data/test_wallet.keys
                ${CMAKE_CURRENT_BINARY_DIR}/test_wallet.keys)
        endif()

endif (OPT_BUILD_TESTS)

# copy config file to build directory
file(INSTALL data/config.ini DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")


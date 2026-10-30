cmake_minimum_required(VERSION 3.24)

if(NOT DEFINED TEST_CASE)
    message(FATAL_ERROR "TEST_CASE is required")
endif()

function(assert_strequal actual expected)
    if(NOT "${${actual}}" STREQUAL "${expected}")
        message(FATAL_ERROR "Expected ${actual}='${expected}', got '${${actual}}'")
    endif()
endfunction()

function(assert_false variable)
    if(${variable})
        message(FATAL_ERROR "Expected ${variable}=FALSE, got '${${variable}}'")
    endif()
endfunction()

function(assert_true variable)
    if(NOT ${variable})
        message(FATAL_ERROR "Expected ${variable}=TRUE, got '${${variable}}'")
    endif()
endfunction()

function(seed_module_defaults)
    set(AU_BUILD_CLOUD_AUDIOCOM ON CACHE BOOL "" FORCE)
    set(AU_BUILD_MUSEHUB_MODULE ON CACHE BOOL "" FORCE)
    set(AU_BUILD_USAGEINFO_MODULE ON CACHE BOOL "" FORCE)
    set(MUSE_MODULE_CLOUD ON CACHE BOOL "" FORCE)
    set(MUSE_MODULE_UPDATE ON CACHE BOOL "" FORCE)
    set(MUSE_MODULE_NETWORK ON CACHE BOOL "" FORCE)
    set(MUSE_MODULE_LEARN ON CACHE BOOL "" FORCE)
    set(MUSE_MODULE_DIAGNOSTICS OFF CACHE BOOL "" FORCE)
    set(MUSE_MODULE_LANGUAGES OFF CACHE BOOL "" FORCE)
endfunction()

set(PRODUCT_PROFILE_POLICY "${CMAKE_CURRENT_LIST_DIR}/../SetupProductProfile.cmake")

if(PRODUCT_PROFILE_INVALID_CHILD)
    seed_module_defaults()
    set(AU4_PRODUCT_PROFILE "invalid" CACHE STRING "" FORCE)
    include("${PRODUCT_PROFILE_POLICY}")
    return()
endif()

if(TEST_CASE STREQUAL "invalid_profile")
    execute_process(
        COMMAND "${CMAKE_COMMAND}"
            -DTEST_CASE=invalid_profile
            -DPRODUCT_PROFILE_INVALID_CHILD=ON
            -P "${CMAKE_CURRENT_LIST_FILE}"
        RESULT_VARIABLE child_result
        OUTPUT_VARIABLE child_stdout
        ERROR_VARIABLE child_stderr
    )
    string(CONCAT child_output "${child_stdout}" "${child_stderr}")
    if(child_result EQUAL 0)
        message(FATAL_ERROR "Invalid profile unexpectedly succeeded")
    endif()
    if(NOT child_output MATCHES "AU4_PRODUCT_PROFILE must be audacity or workbench")
        message(FATAL_ERROR "Invalid profile did not report the required validation message:\n${child_output}")
    endif()
    return()
endif()

seed_module_defaults()

if(TEST_CASE STREQUAL "default_profile")
    unset(AU4_PRODUCT_PROFILE CACHE)
elseif(TEST_CASE STREQUAL "workbench_profile")
    set(AU4_PRODUCT_PROFILE "workbench" CACHE STRING "" FORCE)
elseif(TEST_CASE STREQUAL "cached_workbench_override")
    set(AU4_PRODUCT_PROFILE "workbench" CACHE STRING "" FORCE)
    set(AU_BUILD_CLOUD_AUDIOCOM ON CACHE BOOL "" FORCE)
    set(AU_BUILD_MUSEHUB_MODULE ON CACHE BOOL "" FORCE)
    set(AU_BUILD_USAGEINFO_MODULE ON CACHE BOOL "" FORCE)
    set(MUSE_MODULE_CLOUD ON CACHE BOOL "" FORCE)
    set(MUSE_MODULE_UPDATE ON CACHE BOOL "" FORCE)
    set(MUSE_MODULE_NETWORK ON CACHE BOOL "" FORCE)
    set(MUSE_MODULE_LEARN ON CACHE BOOL "" FORCE)
else()
    message(FATAL_ERROR "Unknown TEST_CASE: ${TEST_CASE}")
endif()

include("${PRODUCT_PROFILE_POLICY}")

if(TEST_CASE STREQUAL "default_profile")
    assert_strequal(AU4_PRODUCT_PROFILE "audacity")
else()
    assert_strequal(AU4_PRODUCT_PROFILE "workbench")
    assert_false(AU_BUILD_CLOUD_AUDIOCOM)
    assert_false(AU_BUILD_MUSEHUB_MODULE)
    assert_false(AU_BUILD_USAGEINFO_MODULE)
    assert_false(MUSE_MODULE_CLOUD)
    assert_false(MUSE_MODULE_UPDATE)
    assert_false(MUSE_MODULE_NETWORK)
    assert_false(MUSE_MODULE_LEARN)
    assert_true(MUSE_MODULE_DIAGNOSTICS)
    assert_true(MUSE_MODULE_LANGUAGES)
endif()

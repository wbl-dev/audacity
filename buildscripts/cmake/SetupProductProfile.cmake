set(AU4_PRODUCT_PROFILE "audacity" CACHE STRING "Product profile: audacity or workbench")
set_property(CACHE AU4_PRODUCT_PROFILE PROPERTY STRINGS audacity workbench)

if(NOT AU4_PRODUCT_PROFILE STREQUAL "audacity" AND NOT AU4_PRODUCT_PROFILE STREQUAL "workbench")
    message(FATAL_ERROR "AU4_PRODUCT_PROFILE must be audacity or workbench")
endif()

set(AU4_WORKBENCH_PROFILE OFF)

if(AU4_PRODUCT_PROFILE STREQUAL "workbench")
    set(AU4_WORKBENCH_PROFILE ON)

    set(AU_BUILD_CLOUD_AUDIOCOM OFF CACHE BOOL "Build cloud audiocom module" FORCE)
    set(AU_BUILD_MUSEHUB_MODULE OFF CACHE BOOL "Build Muse Hub module" FORCE)
    set(AU_BUILD_USAGEINFO_MODULE OFF CACHE BOOL "Build usage info module" FORCE)
    set(MUSE_MODULE_CLOUD OFF CACHE BOOL "Build cloud module" FORCE)
    set(MUSE_MODULE_UPDATE OFF CACHE BOOL "Build update module" FORCE)
    set(MUSE_MODULE_NETWORK OFF CACHE BOOL "Build network module" FORCE)
    set(MUSE_MODULE_LEARN OFF CACHE BOOL "Build learn module" FORCE)
    set(MUSE_MODULE_DIAGNOSTICS ON CACHE BOOL "Build diagnostics module" FORCE)
    set(MUSE_MODULE_LANGUAGES ON CACHE BOOL "Build languages module" FORCE)

    # Also override normal variables established by the base profile in this scope.
    set(AU_BUILD_CLOUD_AUDIOCOM OFF)
    set(AU_BUILD_MUSEHUB_MODULE OFF)
    set(AU_BUILD_USAGEINFO_MODULE OFF)
    set(MUSE_MODULE_CLOUD OFF)
    set(MUSE_MODULE_UPDATE OFF)
    set(MUSE_MODULE_NETWORK OFF)
    set(MUSE_MODULE_LEARN OFF)
    set(MUSE_MODULE_DIAGNOSTICS ON)
    set(MUSE_MODULE_LANGUAGES ON)
endif()

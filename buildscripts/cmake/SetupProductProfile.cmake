set(AU4_PRODUCT_PROFILE "audacity" CACHE STRING "Product profile: audacity or workbench")
set_property(CACHE AU4_PRODUCT_PROFILE PROPERTY STRINGS audacity workbench)

if(NOT AU4_PRODUCT_PROFILE STREQUAL "audacity" AND NOT AU4_PRODUCT_PROFILE STREQUAL "workbench")
    message(FATAL_ERROR "AU4_PRODUCT_PROFILE must be audacity or workbench")
endif()

set(AU4_WORKBENCH_PROFILE OFF)

if(AU4_PRODUCT_PROFILE STREQUAL "workbench")
    set(AU4_WORKBENCH_PROFILE ON)

    # Override effective values in this configure without rewriting the user's
    # cache. A later configure can therefore switch back to the Audacity profile.
    set(AU_BUILD_CLOUD_AUDIOCOM OFF)
    set(AU_BUILD_MUSEHUB_MODULE OFF)
    set(AU_BUILD_USAGEINFO_MODULE OFF)
    set(MUSE_MODULE_CLOUD OFF)
    set(MUSE_MODULE_UPDATE OFF)
    set(MUSE_MODULE_NETWORK OFF)
    set(MUSE_MODULE_LEARN OFF)
    set(MUSE_MODULE_DIAGNOSTICS ON)
    set(MUSE_MODULE_LANGUAGES ON)
    set(MUSE_MODULE_TESTFLOW OFF)
endif()

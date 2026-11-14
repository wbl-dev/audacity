if (NOT AU_MODULE_EFFECTS_AUDIO_UNIT)
    add_subdirectory(audio_unit)
endif()

if (NOT AU_MODULE_EFFECTS_LV2)
    add_subdirectory(lv2)
endif()

if (NOT AU_MODULE_EFFECTS_VST)
    add_subdirectory(vst)
endif()


if (NOT MUSE_MODULE_EXTENSIONS)
    add_subdirectory(extensions)
    add_subdirectory(effects_extensions)
endif()

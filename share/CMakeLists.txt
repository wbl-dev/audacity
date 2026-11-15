#
# Audacity: A Digital Audio Editor
#

add_subdirectory(locale)
if (MUSE_MODULE_TESTFLOW)
    add_subdirectory(testflowscripts)
endif()
add_subdirectory(icons)
add_subdirectory(workspaces)

# The AU4 effects engine loads its built-in equalization curves from resources.
set(EQ_DEFAULT_CURVES_SOURCE "${PROJECT_SOURCE_DIR}/au3/resources/EQDefaultCurves.xml")
if(OS_IS_WIN)
    set(EQ_DEFAULT_CURVES_BUILD_DIR "${CMAKE_BINARY_DIR}/${MUSE_APP_INSTALL_RESOURCES_LOCATION}")
elseif(OS_IS_LIN)
    set(EQ_DEFAULT_CURVES_BUILD_DIR "${CMAKE_BINARY_DIR}/src/${MUSE_APP_INSTALL_RESOURCES_LOCATION}")
else()
    set(EQ_DEFAULT_CURVES_BUILD_DIR "${CMAKE_BINARY_DIR}/src/app/${MUSE_APP_INSTALL_RESOURCES_LOCATION}")
endif()
set(EQ_DEFAULT_CURVES_BUILD_FILE "${EQ_DEFAULT_CURVES_BUILD_DIR}/EQDefaultCurves.xml")
add_custom_command(
    OUTPUT "${EQ_DEFAULT_CURVES_BUILD_FILE}"
    DEPENDS "${EQ_DEFAULT_CURVES_SOURCE}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${EQ_DEFAULT_CURVES_BUILD_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${EQ_DEFAULT_CURVES_SOURCE}" "${EQ_DEFAULT_CURVES_BUILD_FILE}"
    VERBATIM
)
add_custom_target(eq-default-curves ALL DEPENDS "${EQ_DEFAULT_CURVES_BUILD_FILE}")
install(FILES "${EQ_DEFAULT_CURVES_SOURCE}" DESTINATION "${MUSE_APP_INSTALL_RESOURCES_LOCATION}")

IF (AU_MODULE_EFFECTS_NYQUIST)
    add_subdirectory(nyquist-runtime)
    add_subdirectory(nyquist-plug-ins)
ENDIF()

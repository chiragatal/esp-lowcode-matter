idf_component_register(SRC_DIRS .
                       INCLUDE_DIRS .
                       REQUIRES lp_sw_timer esp_amp)

target_include_directories(
    ${COMPONENT_LIB} PRIVATE ${COMPONENT_INCLUDES}
)

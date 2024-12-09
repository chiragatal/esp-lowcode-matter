idf_component_register(SRC_DIRS led utils ws2812 .
                       INCLUDE_DIRS led utils ws2812 .
                       REQUIRES lp_sw_timer lp_rmt)

target_include_directories(
    ${COMPONENT_LIB} PRIVATE ${COMPONENT_INCLUDES}
)

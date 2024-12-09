idf_component_register(SRC_DIRS .
                       INCLUDE_DIRS .
                       REQUIRES ulp hal)

target_include_directories(
    ${COMPONENT_LIB} PRIVATE ${COMPONENT_INCLUDES}
)

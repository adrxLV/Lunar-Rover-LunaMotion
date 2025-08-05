install(
    TARGETS lrn_exe
    RUNTIME COMPONENT lrn_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()

# ==================================================================
#  tubex-3rd - cmake configuration file
# ==================================================================

add_subdirectory(pyibex)

if(WITH_CAPD)
  add_subdirectory(ode)
endif()
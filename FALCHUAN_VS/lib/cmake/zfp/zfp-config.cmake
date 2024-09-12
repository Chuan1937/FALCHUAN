# - Config file for the zfp package
#
# It defines the following variables
#  ZFP_INCLUDE_DIRS - include directories for zfp
#  ZFP_LIBRARIES    - libraries to link against
#  ZFP_WITH_OPENMP  - indicates if the zfp library has been built with OpenMP support
#  ZFP_WITH_CUDA    - indicates if the zfp library has been built with CUDA support
#  ZFP_CFP_ENABLED  - indicated if the cfp library has been built
#
# And the following imported targets:
#   zfp::zfp
#
# If cfp is enabled the following variabled are also defined
#  CFP_INCLUDE_DIRS - include directories for cfp
#  CFP_LIBRARIES    - libraries to link against (cfp only)
#
# As well as the following imported targets:
#   zfp::cfp

include("${CMAKE_CURRENT_LIST_DIR}/zfp-config-version.cmake")

include(FindPackageHandleStandardArgs)
set(${CMAKE_FIND_PACKAGE_NAME}_CONFIG "${CMAKE_CURRENT_LIST_FILE}")
find_package_handle_standard_args(${CMAKE_FIND_PACKAGE_NAME} CONFIG_MODE)

if(NOT TARGET zfp::zfp)
  include("${CMAKE_CURRENT_LIST_DIR}/zfp-targets.cmake")
  set(ZFP_LIBRARIES "zfp::zfp")
  get_target_property(ZFP_INCLUDE_DIRS zfp::zfp INTERFACE_INCLUDE_DIRECTORIES)
endif()

set(ZFP_CFP_ENABLED OFF)
if(ZFP_CFP_ENABLED AND NOT TARGET zfp::cfp)
  include("${CMAKE_CURRENT_LIST_DIR}/cfp-targets.cmake")
  set(CFP_LIBRARIES "zfp::cfp")
  get_target_property(CFP_INCLUDE_DIRS zfp::cfp INTERFACE_INCLUDE_DIRECTORIES)
endif()

set(ZFP_WITH_OPENMP ON)
if(ZFP_WITH_OPENMP)
  find_package(OpenMP REQUIRED COMPONENTS C)
endif()

set(ZFP_WITH_CUDA OFF)

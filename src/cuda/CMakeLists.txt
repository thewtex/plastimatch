##-----------------------------------------------------------------------------
##  See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
##-----------------------------------------------------------------------------
project (src_plastimatch_cuda)

set_directory_properties (PROPERTIES INCLUDE_DIRECTORIES "")
set_directory_properties (PROPERTIES COMPILE_DEFINITIONS "")

include_directories (BEFORE ${PLM_BINARY_DIR})
include_directories (BEFORE ${CMAKE_SOURCE_DIR}/src/base)
include_directories (BEFORE ${CMAKE_SOURCE_DIR}/src/cuda)
include_directories (BEFORE ${CMAKE_SOURCE_DIR}/src/register)
include_directories (BEFORE ${CMAKE_SOURCE_DIR}/src/sys)
include_directories (BEFORE ${CMAKE_SOURCE_DIR}/src/util)
include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR})
include_directories (AFTER ${MSINTTYPES_INCLUDE_DIR})

##-----------------------------------------------------------------------------
##  SOURCE FILES
##-----------------------------------------------------------------------------
set (PLMCUDA_LIBRARY_SRC
  cuda_kernel_util.cu
  cuda_mem.cu
  cuda_probe.cu
  cuda_texture.cu
  cuda_util.cu
  )

##-----------------------------------------------------------------------------
##  LIBRARY DEPENDENCIES
##-----------------------------------------------------------------------------
set (PLMCUDA_LIBRARY_DEPENDENCIES 
  plmsys
  ${CUDA_LIBRARIES}
  )

##-----------------------------------------------------------------------------
##  SPECIAL BUILD RULES: CUDA
##-----------------------------------------------------------------------------
if (CUDA_FOUND)
  if (PLM_USE_GPU_PLUGINS)
    plm_add_gpu_plugin_library (plmcuda "${PLMCUDA_LIBRARY_SRC}")
    target_link_libraries (plmcuda
      ${PLMCUDA_LIBRARY_DEPENDENCIES}
      )
  else ()
    cuda_compile (CUDA_WRAPPERS ${PLMCUDA_LIBRARY_SRC})
    set (PLMCUDA_LIBRARY_SRC
      ${PLMCUDA_LIBRARY_SRC}
      ${CUDA_WRAPPERS}
      )
    plm_add_library (
      plmcuda
      "${PLMCUDA_LIBRARY_SRC}" 
      "${PLMCUDA_LIBRARY_DEPENDENCIES}"
      ""
      "${PLASTIMATCH_INCLUDE_DIRECTORIES}"
      "")
  endif ()
endif ()

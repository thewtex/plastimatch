##-----------------------------------------------------------------------------
##  See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
##-----------------------------------------------------------------------------
project (src_plastimatch_cli)

include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR})
include_directories (AFTER ${LUA_INCLUDE_DIR})

##-----------------------------------------------------------------------------
##  SOURCE FILES
##-----------------------------------------------------------------------------
#set (PLMCLP_LIBRARY_SRC
#  plm_clp.cxx plm_clp.h
#  )

set (PLASTIMATCH_SRC
  pcmd_add.cxx 
  pcmd_adjust.cxx 
  pcmd_autolabel_train.cxx 
  pcmd_autolabel.cxx 
  pcmd_bbox.cxx pcmd_bbox.h
  pcmd_benchmark.cxx 
  pcmd_boundary.cxx pcmd_boundary.h
  pcmd_compare.cxx 
  pcmd_compose.cxx 
  pcmd_crop.cxx 
  pcmd_dice.cxx 
  pcmd_diff.cxx 
  pcmd_dmap.cxx
  pcmd_dose.cxx pcmd_dose.h
  pcmd_drr.cxx 
  pcmd_dvh.cxx 
  pcmd_fdk.cxx 
  pcmd_filter.cxx
  pcmd_gamma.cxx
  pcmd_header.cxx
  pcmd_intersect.cxx
  pcmd_jacobian.cxx
  pcmd_lm_warp.cxx
  pcmd_mabs.cxx 
  pcmd_mask.cxx 
  pcmd_maximum.cxx
  pcmd_ml_convert.cxx pcmd_ml_convert.h 
  pcmd_multiply.cxx
  pcmd_probe.cxx 
  pcmd_register.cxx pcmd_register.h 
  pcmd_resample.cxx 
  pcmd_scale.cxx
#  pcmd_script.cxx 
  pcmd_segment.cxx 
  pcmd_sift.cxx
  pcmd_stats.cxx 
  pcmd_synth.cxx 
  pcmd_synth_vf.cxx 
  pcmd_threshold.cxx
  pcmd_thumbnail.cxx 
  pcmd_union.cxx
  pcmd_warp.cxx 
  pcmd_warp_dij.cxx 
  pcmd_warp_pointset.cxx 
  pcmd_wed.cxx pcmd_wed.h
  pcmd_xio_dvh.cxx 
  pcmd_xf_convert.cxx 
  pcmd_xf_invert.cxx pcmd_xf_invert.h
  plastimatch_main.cxx
  plastimatch_startup.cxx
  )

set (LANDMARK_WARP_SRC
  landmark_warp_main.cxx
  )

set (DRR_SRC
  drr_main.cxx 
  drr_opts.cxx
  )

set (PLM_CLI_LIBRARIES ${PLASTIMATCH_LIBS} plmclp)

##-----------------------------------------------------------------------------
##  BUILD TARGETS
##-----------------------------------------------------------------------------
plm_add_executable_v3 (plastimatch
  "${PLASTIMATCH_SRC}"
  "${PLASTIMATCH_INCLUDE_DIRECTORIES}"
  "${PLM_CLI_LIBRARIES}"
  "${PLASTIMATCH_LDFLAGS}"
  ${INSTALL_ALWAYS})

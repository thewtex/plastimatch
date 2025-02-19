##-----------------------------------------------------------------------------
##  Download ITK from internet and compile
##-----------------------------------------------------------------------------
set (proj ITK)

set (itk_url https://downloads.sourceforge.net/project/plastimatch/Third%20Party/InsightToolkit-4.12.2-patch-1.tar.gz)
set (itk_md5sum dfebf9edd19c4e53103c812aca2f8c9f)

ExternalProject_Add (${proj}
  DOWNLOAD_DIR ${proj}-download
  URL ${itk_url}
  URL_MD5 ${itk_md5sum}
  SOURCE_DIR ${proj}
  BINARY_DIR ${proj}-build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
  -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
  -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
  -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
  -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
  -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
  #    -DCMAKE_CXX_STANDARD:STRING=${CMAKE_CXX_STANDARD}
  -DCMAKE_CXX_STANDARD_REQUIRED:BOOL=${CMAKE_CXX_STANDARD_REQUIRED}
  -DCMAKE_CXX_EXTENSIONS:BOOL=${CMAKE_CXX_EXTENSIONS}
  -DBUILD_EXAMPLES:BOOL=OFF
  -DBUILD_SHARED_LIBS:BOOL=ON
  -DBUILD_TESTING:BOOL=OFF
  -DModule_ITKReview:BOOL=ON
  INSTALL_COMMAND ""
  )

set (ITK_DIR ${CMAKE_BINARY_DIR}/${proj}-build)

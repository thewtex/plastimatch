SET (CTEST_SOURCE_DIRECTORY "$ENV{HOME}/build/nightly/src/plastimatch")
SET (CTEST_BINARY_DIRECTORY "$ENV{HOME}/build/nightly/gelato_03/plastimatch")
SET (CTEST_CMAKE_COMMAND "/usr/local/bin/cmake")
SET (CTEST_COMMAND "/usr/local/bin/ctest -D Nightly")
SET (CTEST_INITIAL_CACHE "
//Name of generator.
CMAKE_GENERATOR:INTERNAL=Unix Makefiles

//Name of the build
BUILDNAME:STRING=03-lin64-PisrCD

//Name of the computer/site where compile is being run
SITE:STRING=gelato-gcc4.4.5

//The directory containing ITKConfig.cmake.  This is either the
// root of the build tree, or PREFIX/lib/InsightToolkit for an
// installation.
ITK_DIR:PATH=/home/gcs6/build/itk-3.20.1

//Root of DCMTK install tree (optional).
DCMTK_DIR:PATH=/PHShome/gcs6/build/install/dcmtk-3.6.0

//Disable REG-2-3
PLM_CONFIG_DISABLE_REG23:BOOL=ON

// Use anonymous checkout
SVN_UPDATE_OPTIONS:STRING=--username anonymous --password \\\"\\\"

//Build with shared libraries.
BUILD_SHARED_LIBS:BOOL=ON
")

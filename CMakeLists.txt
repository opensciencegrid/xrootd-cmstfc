
cmake_minimum_required( VERSION 2.6 )
project( xrootd-cmstfc )

set( CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake )

find_package( Xrootd REQUIRED )
find_package( XercesC REQUIRED )
find_package( Pcre REQUIRED )

if( CMAKE_COMPILER_IS_GNUCXX )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror" )
endif()

if( CMAKE_COMPILER_IS_GNUCC )
  set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror" )
endif()

include_directories( "${PROJECT_SOURCE_DIR}" "${XROOTD_INCLUDES}"
"${XERCES_INCLUDES}" "${PCRE_INCLUDES}" )

include(CheckCXXSourceCompiles)
set(CMAKE_REQUIRED_INCLUDES "${XROOTD_INCLUDES}")
SET(CMAKE_EXTRA_INCLUDE_FILES "XrdOuc/XrdOucName2Name.hh")
check_cxx_source_compiles(
"\#include \"XrdOuc/XrdOucName2Name.hh\"
 int main() {XrdOucName2NameVec *vec=NULL; if (vec) {} return 0;}
" HAVE_N2N_VEC
)
set(CMAKE_REQUIRED_INCLUDES)

set(N2N_VEC_FILES)
if (HAVE_N2N_VEC)
  set(N2N_VEC_FILES src/N2NVec.cc src/N2NVec.hh)
  add_definitions( -DHAVE_N2N_VEC=1 )
endif ()

add_library( XrdCmsTfc MODULE src/XrdCmsTfc.cc src/XrdCmsTfc.hh ${N2N_VEC_FILES} )
target_link_libraries(XrdCmsTfc ${XROOTD_UTILS} ${XERCES_LIB} ${PCRE_LIB})

if (NOT DEFINED CMAKE_INSTALL_LIBDIR)
  SET(CMAKE_INSTALL_LIBDIR "lib")
endif()

install(
  TARGETS XrdCmsTfc
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} )

install(
  FILES ${CMAKE_SOURCE_DIR}/src/XrdCmsTfc.hh
  DESTINATION include )

set_target_properties(
  XrdCmsTfc
  PROPERTIES
  VERSION   "0.0.1"
  SOVERSION "0"
  LINK_INTERFACE_LIBRARIES "" )


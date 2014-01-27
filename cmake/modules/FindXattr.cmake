# - Try to find the xattr header
# Once done this will define 
# 
#  XATTR_FOUND - system has xattr.h 
#  XATTR_INCLUDE_DIR - the xattr include directory 
# 
 
FIND_PATH(XATTR_INCLUDE_DIR attr/xattr.h) 
 
IF(XATTR_INCLUDE_DIR) 
   SET(XATTR_FOUND TRUE) 
ENDIF(XATTR_INCLUDE_DIR) 
 
IF(XATTR_FOUND) 
  IF(NOT XAttr_FIND_QUIETLY) 
    MESSAGE(STATUS "Found xattr.h") 
  ENDIF(NOT XAttr_FIND_QUIETLY) 
ELSE(XATTR_FOUND) 
  IF(XAttr_FIND_REQUIRED) 
    MESSAGE(FATAL_ERROR "Could not find xattr.h") 
  ENDIF(XAttr_FIND_REQUIRED) 
ENDIF(XATTR_FOUND) 

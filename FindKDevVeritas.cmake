#
# Find the KDevelop Veritas library
#
# Example usage of this module:
# find_package(KDevVeritas)
# 
# The version number and REQUIRED flag are optional. You can set CMAKE_PREFIX_PATH
# variable to help it find the required files and directories

# KDEVVERITAS_FOUND                   - set to TRUE if the platform was found and the version is compatible FALSE otherwise
#
# KDEVVERITAS_INCLUDE_DIR             - include dir of the platform, for example /usr/include/kdevplatform
# KDEVVERITAS_LIBRARY                 - veritas module library
#
# Copyright 2009 Andreas Pakulat <apaku@gmx.de>
# Redistribution and use is allowed according to the terms of the BSD license.

find_path( KDEVVERITAS_INCLUDE_DIR veritas/test.h
           PATH_SUFFIXES kdevplatform
      )
find_library( KDEVVERITAS_LIBRARY kdevplatformveritas
      )

# handle the QUIETLY and REQUIRED arguments and set JPEG_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(KDevVeritas DEFAULT_MSG KDEVVERITAS_LIBRARY KDEVVERITAS_INCLUDE_DIR)

mark_as_advanced(KDEVVERITAS_INCLUDE_DIR KDEVVERITAS_LIBRARY)


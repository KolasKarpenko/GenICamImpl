cmake_minimum_required (VERSION 3.0)



project(GenICam)

#extlib

if (WIN32)
  add_subdirectory(GenICamWin64Lib)
  add_subdirectory(freeglutWin64)
  add_subdirectory(zlibWin64)
else ()
  add_subdirectory(GenICamLinux64Lib)
endif()

add_subdirectory(Promise)

#end extlib


add_subdirectory(GevDeviceLib)
add_subdirectory(TestApp)

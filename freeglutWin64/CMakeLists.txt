cmake_minimum_required (VERSION 3.0)

project(FreeGlutWin64Lib)

set(GLUT_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include PARENT_SCOPE)

set(GLUT_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/lib/x64/freeglut.lib PARENT_SCOPE)

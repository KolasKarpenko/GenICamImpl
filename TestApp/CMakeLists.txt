cmake_minimum_required (VERSION 3.0)

project(TestApp)

if (NOT WIN32)
  find_package(OpenGL)
  find_package(GLUT)
  find_package(ZLIB)
endif()

include_directories(${GENICAMLIB_INCLUDE} ${GEVDEVICELIB_INCLUDE} ${PROMISELIB_INCLUDE} ${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})
add_executable(TestApp main.cpp)

target_link_libraries(TestApp
GevDeviceLib
${OPENGL_LIBRARIES} 
${GLUT_LIBRARY}
${ZLIB_LIBRARIES}
${GENICAMLIB_CLAllSerial_LIB}
${GENICAMLIB_CLProtocol_LIB}
${GENICAMLIB_FirmwareUpdate_LIB}
${GENICAMLIB_GCBase_LIB}
${GENICAMLIB_GenApi_LIB}
${GENICAMLIB_GenCP_LIB}
${GENICAMLIB_Log_LIB}
${GENICAMLIB_log4cpp_LIB}
${GENICAMLIB_XmlParser_LIB}
)

if(WIN32)
  target_link_libraries(TestApp wsock32 ws2_32 iphlpapi)
else()
  target_link_libraries(TestApp pthread)
endif()

cmake_minimum_required(VERSION 3.1...3.25)
project(
  aplay-arecord-v2
  VERSION 1.0
  LANGUAGES CXX)

SET(CMAKE_INSTALL_PREFIX ./install CACHE PATH "default install prefix" FORCE)

find_library(ALSA asound)

#add_library(${PROJECT_NAME} SHARED systemdDbusServiceLib.cpp systemdDbusServiceLib.h)
#target_link_libraries(${PROJECT_NAME} PRIVATE ${SYSTEMD})
#install(TARGETS ${PROJECT_NAME}
#  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
#)

add_executable(test 
  arecord2.cpp recorder.hpp
  common.hpp config.hpp handle.hpp snd_pcm_params.hpp capture_handle.hpp
)
target_link_libraries(test PRIVATE ${ALSA})
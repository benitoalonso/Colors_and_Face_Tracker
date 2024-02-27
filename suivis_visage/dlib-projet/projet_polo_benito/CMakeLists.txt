cmake_minimum_required(VERSION 2.8.12)

project(projet_polo_benito)

add_subdirectory(../dlib dlib_build)

# To compile this program all you need to do is ask cmake.  You would type
# these commands from within the directory containing this CMakeLists.txt
# file:
#   mkdir build
#   cd build
#   cmake ..
#   cmake --build . --config Release
#
# The cmake .. command looks in the parent folder for a file named

#------------------------------------------------------------------------------------------------
macro(add_example name)
   add_executable(${name} ${name}.cpp)
   target_link_libraries(${name} dlib::dlib )
endmacro()

# if an example requires GUI, call this macro to check DLIB_NO_GUI_SUPPORT to include or exclude
macro(add_gui_example name)
   if (DLIB_NO_GUI_SUPPORT)
      message("No GUI support, so we won't build the ${name} example.")
   else()
      add_example(${name})
   endif()
endmacro()

#------------------------------------BENITO & POLO------------------------------------------------------------
if (DLIB_NO_GUI_SUPPORT)
   message("No GUI support, so we won't build the webcam_face_pose_ex example.")
else()
   find_package(OpenCV QUIET)
   find_package(GTK2)
   find_package(PkgConfig REQUIRED)
   #pkg_check_modules(GTK2 REQUIRED gtk+-2.0)

   if (OpenCV_FOUND)
      if(GTK2_FOUND)
	   include_directories(${GTK2_INCLUDE_DIRS})
	   include_directories(${OpenCV_INCLUDE_DIRS})
	      
	   link_directories(${GTK2_LIBRARY_DIRS})
   	   #Add other flags to the compiler
   	   #add_definitions(${GTK2_CFLAGS_OTHER})
   
	   #mon code à moi:
	   add_executable(visageandfiltres visageandfiltres.cpp)
	   target_link_libraries(visageandfiltres dlib::dlib ${GTK2_LIBRARIES} ${OpenCV_LIBS})
       endif(GTK2_FOUND)      
   else()
      message("OpenCV not found, so we won't build the webcam_face_pose_ex example.")
   endif(OpenCV_FOUND)
endif(DLIB_NO_GUI_SUPPORT)











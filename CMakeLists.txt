cmake_minimum_required(VERSION 2.8.8)
project(pointmatcher_vtk)

# Paraview
find_package(ParaView 3.98 REQUIRED COMPONENTS vtkPVPythonCatalyst vtkRenderingFreeTypeOpenGL vtkInteractionStyle)
include("${PARAVIEW_USE_FILE}")

# libpointmatcher
find_package(pointmatcher 1.1.0 REQUIRED)
include_directories("${POINTMATCHER_INCLUDE_DIRS}")
message(STATUS "Using libpointmatcher version ${pointmatcher_VERSION}")


include_directories(include)

set(SHARED_LIBS "false" CACHE BOOL "To build shared (true) or static (false) library")

if (SHARED_LIBS)
	add_library(pointmatcher_vtk SHARED src/conversions.cpp)
else(SHARED_LIBS)
	add_library(pointmatcher_vtk src/conversions.cpp)
endif(SHARED_LIBS)

add_subdirectory(examples)


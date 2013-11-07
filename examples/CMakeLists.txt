
add_executable(demo demo.cpp)
target_link_libraries(demo pointmatcher_vtk vtkPVPythonCatalyst ${POINTMATCHER_LIBRARIES} vtkRenderingFreeTypeOpenGL vtkInteractionStyle)




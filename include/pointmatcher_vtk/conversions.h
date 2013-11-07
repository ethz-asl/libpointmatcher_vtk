#ifndef __POINTMATCHER_VTK_CONVERSION_H
#define __POINTMATCHER_VTK_CONVERSION_H

#include "pointmatcher/PointMatcher.h"

// vtk
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h> 


namespace PointMatcher_vtk
{
	// FIXME: should we output vtkSmartPointer or the object directly?


	//! convert point cloud with is descriptors to a vtk polydata vertices
	template<typename T>
	vtkSmartPointer<vtkPolyData> pointMatcherCloudToPolyData(const typename PointMatcher<T>::DataPoints& pmCloud);

	//! convert of vector of transformation parameters to a vtk polydata line with orientation added as vector for each translation coordinates
	template<typename T>
	vtkSmartPointer<vtkPolyData> transParametersToPolyData(const std::vector< typename PointMatcher< T >::TransformationParameters > path);



	// conversion of templated type to specific vtk array types
	template <typename T>
	struct VTK_type_generator;

	template <>
	struct VTK_type_generator<double> 
	{
	  static const vtkIdType type = VTK_DOUBLE;
		typedef vtkDoubleArray ArrayType;
	};

	template <>
	struct VTK_type_generator<float> 
	{
	  static const vtkIdType type = VTK_FLOAT;
		typedef vtkFloatArray ArrayType;
	};


	
} //PointMatcher_vtk

#endif //__POINTMATCHER_VTK_CONVERSION_H

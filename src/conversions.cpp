
#include "pointmatcher_vtk/conversions.h"

// vtk
#include <vtkVertexGlyphFilter.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPointData.h>
#include <vtkPolyLine.h>
#include <vtkCellArray.h>

using namespace std;
using namespace PointMatcherSupport;


namespace PointMatcher_vtk
{



template<typename T>
vtkSmartPointer<vtkPolyData> pointMatcherCloudToPolyData(const typename PointMatcher<T>::DataPoints& pmCloud)
{

	typedef typename VTK_type_generator<T>::ArrayType ArrayGeneric;
	const int nbPoints = pmCloud.features.cols();
	const int dim = pmCloud.features.rows() - 1;

	// TODO: throw an expection instead
	assert(dim == 2 || dim == 3);
	
	vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
  
	vtkSmartPointer<vtkPoints> points =  vtkSmartPointer<vtkPoints>::New();
	points->SetDataType(VTK_type_generator<T>::type);
	
	for(int i=0; i < nbPoints; i++)
	{
		const float x = pmCloud.features(0,i);
		const float y = pmCloud.features(1,i);
		
		// Handle missing dimension
		if(dim == 3)
			points->InsertNextPoint(x, y, pmCloud.features(2,i));
		else
			points->InsertNextPoint(x, y, 0);
	}

	polydata->SetPoints(points);

	// Generate a vertex for all points
	vtkSmartPointer<vtkVertexGlyphFilter> vertexFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
	vertexFilter->SetInputData(polydata);
	vertexFilter->Update();
	polydata->ShallowCopy(vertexFilter->GetOutput());

	// Loop through all descriptor and dispatch appropriate VTK tags
	for(BOOST_AUTO(it, pmCloud.descriptorLabels.begin()); it != pmCloud.descriptorLabels.end(); it++)
	{

		
		if(it->text == "color")
		{
			vtkSmartPointer<vtkUnsignedCharArray> arrayChar = vtkSmartPointer<vtkUnsignedCharArray>::New();
			arrayChar->SetNumberOfComponents(it->span);
			arrayChar->SetName(it->text.c_str());
			
			const BOOST_AUTO(desc, pmCloud.getDescriptorViewByName(it->text));
			
			for(int i=0; i<nbPoints; i++)
			{
				const unsigned char r = 255*desc(0,i);
				const unsigned char g = 255*desc(1,i);
				const unsigned char b = 255*desc(2,i);
				const unsigned char a = 255*desc(3,i);
			
				arrayChar->InsertNextTuple4(r, g, b, a);
			}

			polydata->GetPointData()->AddArray(arrayChar);

		}
		else
		{
			vtkSmartPointer<ArrayGeneric> array = vtkSmartPointer<ArrayGeneric>::New();

			array->SetNumberOfComponents(it->span);
			array->SetName(it->text.c_str());
			
			const BOOST_AUTO(desc, pmCloud.getDescriptorViewByName(it->text));
			for(int i=0; i<nbPoints; i++)
			{
					array->InsertNextTupleValue(desc.col(i).data());
			}

			if(it->text == "normals" && dim == 3)
			{
				polydata->GetPointData()->SetNormals(array);
			}
			else 
			{
				polydata->GetPointData()->AddArray(array);
			}
	
		}
		
	}
		
	return polydata;
	
}

template
vtkSmartPointer<vtkPolyData> pointMatcherCloudToPolyData<float>(const typename PointMatcher<float>::DataPoints& pmCloud);
template
vtkSmartPointer<vtkPolyData> pointMatcherCloudToPolyData<double>(const typename PointMatcher<double>::DataPoints& pmCloud);

// Convert path to vtk a polydata line
template<typename T>
vtkSmartPointer<vtkPolyData> transParametersToPolyData(const std::vector< typename PointMatcher< T >::TransformationParameters > path)
{
	typedef typename PointMatcher< T >::Vector Vector;
	typedef typename PointMatcher< T >::Matrix Matrix;
	typedef typename VTK_type_generator<T>::ArrayType ArrayGeneric;
	
	const int dimFirst = path[0].cols() - 1;
	const int nbPoints = path.size();

	vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
	
	vtkSmartPointer<vtkPoints> points =  vtkSmartPointer<vtkPoints>::New();
	points->SetDataType(VTK_type_generator<T>::type);

	// Prepare cells to draw lines
	vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
	polyLine->GetPointIds()->SetNumberOfIds(nbPoints);
	
	// Prepare descriptors
	vtkSmartPointer<ArrayGeneric> nx = vtkSmartPointer<ArrayGeneric>::New();
	nx->SetNumberOfComponents(3);
	nx->SetName("nx");
	vtkSmartPointer<ArrayGeneric> ny = vtkSmartPointer<ArrayGeneric>::New();
	ny->SetNumberOfComponents(3);
	ny->SetName("ny");
	vtkSmartPointer<ArrayGeneric> nz = vtkSmartPointer<ArrayGeneric>::New();
	nz->SetNumberOfComponents(3);
	nz->SetName("nz");



	for(int i=0; i<nbPoints; i++)
	{
		const int dim = path[i].cols() - 1;
		
		// TODO: throw an expection instead
		assert(dim == 2 || dim == 3);
		assert(dim == dimFirst);

		const Vector t = path[i].rightCols(1);
		const Matrix r = path[i].topLeftCorner(3, 3);// this handle the padding for 2D
		
		nx->InsertNextTupleValue(r.col(0).data());
		// Handle missing dimension
		if(dim == 3)
		{
			points->InsertNextPoint(t(0), t(1), t(2));
			ny->InsertNextTupleValue(r.col(1).data());
			nz->InsertNextTupleValue(r.col(2).data());
		}
		else
		{
			points->InsertNextPoint(t(0), t(1), 0);
		}

		polyLine->GetPointIds()->SetId(i,i);
	}

	// Add coordinates to the dataset
	polydata->SetPoints(points);
	
	// Add the lines to the dataset
  vtkSmartPointer<vtkCellArray> cells =  vtkSmartPointer<vtkCellArray>::New();
	cells->InsertNextCell(polyLine);
	polydata->SetLines(cells);
	
	polydata->GetPointData()->AddArray(nx);

	if(dimFirst == 3)
	{
		polydata->GetPointData()->AddArray(ny);
		polydata->GetPointData()->AddArray(nz);
	}


	// Generate a vertex for all points
	//vtkSmartPointer<vtkVertexGlyphFilter> vertexFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
	//vertexFilter->SetInputData(polydata);
	//vertexFilter->Update();
	//polydata->ShallowCopy(vertexFilter->GetOutput());


	
	return polydata;
}

template
vtkSmartPointer<vtkPolyData> transParametersToPolyData<float>(const std::vector< typename PointMatcher< float >::TransformationParameters > path);
template
vtkSmartPointer<vtkPolyData> transParametersToPolyData<double>(const std::vector< typename PointMatcher< double >::TransformationParameters > path);



} //PointMatcher_vtk

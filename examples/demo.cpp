
// pointmatcher
#include "pointmatcher/PointMatcher.h"
#include "pointmatcher_vtk/conversions.h"

// paraview
#include <vtkCPProcessor.h>
#include <vtkCPPythonScriptPipeline.h>
#include <vtkCPDataDescription.h>
#include <vtkCPInputDataDescription.h>

// vtk 
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>


using namespace std;
using namespace PointMatcherSupport;


// This example code load a python script with a predefined
// processing pipeline and the path to 'data' folder (search for
// file named 3Dtest_*.csv) and stream them to Paraview.
// In Paraview, Tools -> Connect to Catalyst to see the point clouds.
//
// It is also showing a conversion from pointmatcher point cloud to vtk
// polyData.


class CoProcessor
{

	typedef PointMatcher<float> PM;
  
	vtkCPProcessor* processor;

public:
	CoProcessor();
	void initialize(string configPath);
  void finalize();
  void coProcess(PM::DataPoints cloud, double time, unsigned int timeStep, bool lastTimeStep);
};

// Constructor
CoProcessor::CoProcessor()
{
	processor = NULL;
}

void CoProcessor::initialize(string configPath)
{
	if(processor == NULL)
	{
		processor = vtkCPProcessor::New();
		processor->Initialize();
	}
	else
	{
		processor->RemoveAllPipelines();
	}
		
	// Load pre configured pipeline from file
	vtkNew<vtkCPPythonScriptPipeline> pipeline;
	pipeline->Initialize(configPath.c_str());

	// add to the current pipeline
	processor->AddPipeline(pipeline.GetPointer());

	// Note: we can append more pipelines if needed
}

void CoProcessor::finalize()
{
	if(processor != NULL)
	{
		processor->Delete();
		processor = NULL;
	}
	
}

void CoProcessor::coProcess(PM::DataPoints cloud, double time, unsigned int timeStep, bool lastTimeStep)
{
	// convert DataPoints to vtkPolyData
	cerr << "converting pm to vtk..." << endl;
	vtkSmartPointer<vtkPolyData> polydata = PointMatcher_vtk::pointMatcherCloudToPolyData<float>(cloud);
	cerr << "conversion done" << endl;

	// send vtkPolyData to be processed
	vtkNew<vtkCPDataDescription> dataDescription;
	dataDescription->AddInput("input");
	dataDescription->SetTimeData(time, timeStep);
	cout << "time: " << time << ", timeStep: " << timeStep << endl;
	
	if(lastTimeStep == true)
	{
		dataDescription->ForceOutputOn();
	}
	
	
	if(processor->RequestDataDescription(dataDescription.GetPointer()) != 0)
	{
		cerr << "sending  polydata" << endl;
		dataDescription->GetInputDescriptionByName("input")->SetGrid(polydata);
		processor->CoProcess(dataDescription.GetPointer());
		cerr << "finish to send polydata" << endl;
	}

}

int main(int argc, char* argv[])
{
	// verify the arguments
	if(argc != 3)
	{
		cerr << endl;
		cerr << "Error: wrong number of arguments" << endl;
		cerr << "     Usage:   " << argv[0] << " <python_pipeline>.py data/folder/" << endl;
		cerr << "     Example: " << argv[0] << " ../data/demo.py ../data/" << endl;
		cerr << endl;

		return 1;
	}

	string dataPath = argv[2];

	typedef PointMatcher<float> PM;

	std::vector<PM::DataPoints> listCloud;

	cerr << "preloading test data..." << endl;
	
	//listCloud.push_back(PM::DataPoints::load("../data/object_trashBins.vtk"));
	listCloud.push_back(PM::DataPoints::load(dataPath + "3Dtest_0.csv"));
	listCloud.push_back(PM::DataPoints::load(dataPath + "3Dtest_1.csv"));
	listCloud.push_back(PM::DataPoints::load(dataPath + "3Dtest_2.csv"));
	listCloud.push_back(PM::DataPoints::load(dataPath + "3Dtest_3.csv"));
	listCloud.push_back(PM::DataPoints::load(dataPath + "3Dtest_4.csv"));

	cerr << "finish to preload test data" << endl;
	
	CoProcessor cp;
	cp.initialize(argv[1]);

	unsigned int timeStep = 0;
	int i = 0;

	while(true)
	{
    double time = timeStep * 0.1;
		
		if(i >= listCloud.size())
			i = 0;

		cerr << i << " sending " << listCloud[i].features.cols() << " points" << endl;

		cp.coProcess(listCloud[i], time, timeStep, false);

		timeStep++;
		i++;

		cerr << endl;
	}

	cp.finalize();

  return 0;
}


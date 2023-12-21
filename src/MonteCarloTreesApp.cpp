#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"  // Include CameraUi header for camera movement
#include "cinder/Log.h"
#include <random>
#include <ctime>  // For seeding the random number generator
#include <fstream>
#include <iostream>
#include "./../vc2019/RandomWalk.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class MonteCarloTreesApp : public App {
  public:
	void setup() override;
	void update() override;
	void draw() override;
	void mouseDown(MouseEvent event) override;
	void mouseDrag(MouseEvent event) override;
	void mouseWheel(MouseEvent event) override;
	void keyDown(KeyEvent event) override;
	void keyUp(KeyEvent event) override;

	//! enableFileLogging will enable logging to a given file.
	//! This file will not rotate, but you can control the file appending.
	// From https://github.com/cinder/Cinder/blob/master/samples/Logging/src/LoggingApp.cpp
	static void enableFileLogging();

private:
	CameraPersp mCamera;
	CameraUi mCameraUi;  // CameraUi instance for camera control
	vec3 mCameraTarget;
	vector<vec3> mStartPoints;
	vector<vec3> mWalkPoints;
	Color backgroundColor = { 0.3f, 0.3f, 0.3f };
	Color pointColor = { 0.3f, 0.1f, 0.85f };
	Color vertexColor = { 0.0f, 0.5f, 0.5f };
	Color walkColor = { 0.9f, 0.1f, 0.1f };
	Color textColor = { 1.0f, 1.0f, 1.0f };
	// Convert 3D position to 2D screen position
	vec3 textPosition = vec3 (0.0f, -0.01f, 4.2f); // Text position
	vec2 textScreenPos = mCamera.worldToScreen(textPosition, static_cast<float>(getWindowWidth()), static_cast<float>(getWindowHeight()));

	// Path of the random walks
	int numberOfSteps = 100;
	int numberOfStartingPoints = 5;
	// Weight to define how much the interaction should add up
	float interactionWeight = 0.01f;
	vector<vector<vec3>> walkPath;

	// Screening of the potential within distance screeningSize of a particle
	// It seems the random walk flies away when it come too close
	float screeningSize = 0.1f;

	// Step counter of the random walks
	int currentStep = 0;

	// Boolean to keep track of updating
	bool updating = true;
};

void MonteCarloTreesApp::setup()
{
	enableFileLogging();
	CI_LOG_D("Starting setup at " << getElapsedSeconds() << " s");
	
	// Z-buffering
	gl::enableDepthWrite();
	gl::enableDepthRead();

	// Set up initial camera position
	mCamera.lookAt(vec3(5.0, 5.0, 5.0), vec3(0, 0, 0), vec3(0, -1, 0));
	mCamera.setPerspective(45.0f, getWindowAspectRatio(), 0.1f, 1000.0f);
	mCameraTarget = vec3(0);
	mCameraUi = CameraUi(&mCamera, getWindow());

	// Initialize step counter of the random walks
	currentStep = 0;

	// Initialize path of the random walks
	walkPath.resize(numberOfStartingPoints, vector<vec3>(numberOfSteps + 1));

	// Generate initial points
	mStartPoints = RandomWalk::generatePoints(numberOfStartingPoints, -1, 1, -1, 1, -1, 1);

	// Initialize booleans
	updating = true;
}

void MonteCarloTreesApp::enableFileLogging()
{
	//! This call will append log messages to the file `cinder.log` in the folder `/tmp/logging`.
	//! If the folder path `/tmp/logging` does not exist, it will be created for you.

	log::makeLogger<log::LoggerFile>(app::getAppPath() / "cinder.log", false);
}

void MonteCarloTreesApp::update()
{
	if (updating) { CI_LOG_D("\tCurrentstep: \t" << currentStep); }

	// Update for each step of the random walk
	if (currentStep < walkPath[0].size())
	{
		updating = true;
		if (currentStep == 0)
		{
			for (int index = 0; index < numberOfStartingPoints; index++)
			{
				walkPath[index][currentStep] = mStartPoints[index];
			}
		}
		else
		{
			for(int index = 0; index < numberOfStartingPoints; index++)
			{
				vec3 previousPoint = walkPath[index][currentStep - 1];
				if(currentStep < 2)
				{
					// The first walk should not be weighted yet, since the direction of a point with a point is a Nan
					walkPath[index][currentStep] = RandomWalk::generateRandomWalkCircle(previousPoint.x, previousPoint.y, previousPoint.z, 0.2f);
				}
				else
				{
					walkPath[index][currentStep] = RandomWalk::generateWeightedRandomWalk(
						previousPoint.x, previousPoint.y, previousPoint.z, mStartPoints, screeningSize, interactionWeight);
				}
			}
			
		}

		// Print the current position to the console
		//console() << "Step " << currentStep << ": X=" << walkPath[currentStep].x
		//	<< ", Y=" << walkPath[currentStep].y
		//	<< ", Z=" << walkPath[currentStep].z << endl;

		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		currentStep++;
	}
	else updating = false;
}

void MonteCarloTreesApp::draw()
{
	if(updating){ CI_LOG_D(""); }

	gl::clear( backgroundColor );
	gl::setMatrices(mCamera);

	// Draw coordinate system
	gl::drawCoordinateFrame();

	// Render initial points
	gl::color(pointColor);
	for (const auto& point : mStartPoints) { gl::drawSphere(point, 0.1f); }

	// Render random walks
	gl::color(vertexColor);
	gl::begin(GL_LINE_STRIP);
	for (const auto& point : mStartPoints) {
		gl::vertex(point);
	}
	gl::end();

	// Render the random walk path up to the current step
	gl::color(walkColor);
	for (int point = 0; point < numberOfStartingPoints; point++){
		gl::begin(GL_LINE_STRIP);
		for (size_t i = 0; i < currentStep; ++i)
		{
			gl::vertex(walkPath[point][i]);
		}
		gl::end();
	}

	// Display the current step number
	gl::drawString("Step # " + std::to_string(currentStep - 1) + " / " + std::to_string(numberOfSteps), textScreenPos, textColor, Font("Arial", 12));

	//console() << "Eye Point: " << mCamera.getEyePoint() << std::endl;
	//vec3 lookAtPoint = mCamera.getEyePoint() + mCamera.getViewDirection();
	//console() << "Look At Point: " << lookAtPoint << std::endl;
	//console() << "Screen Position: " << textScreenPos << std::endl;

	// Check for OpenGL errors
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		CI_LOG_E("OpenGL Error: " << error);
	}

	//std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void MonteCarloTreesApp::mouseDown(MouseEvent event) {
	mCameraUi.mouseDown(event);
}

void MonteCarloTreesApp::mouseDrag(MouseEvent event) {
	mCameraUi.mouseDrag(event);
}

void MonteCarloTreesApp::mouseWheel(MouseEvent event) {
	mCameraUi.mouseWheel(event);
}

void MonteCarloTreesApp::keyDown(KeyEvent event){
	// Nothing yet
}

void MonteCarloTreesApp::keyUp(KeyEvent event) {
	// Nothing yet
}

CINDER_APP(MonteCarloTreesApp, RendererGl)

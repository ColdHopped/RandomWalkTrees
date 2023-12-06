#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"  // Include CameraUi header for camera movement
#include "cinder/Log.h"
#include <random>
#include <ctime>  // For seeding the random number generator
#include <fstream>
#include <iostream>

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
	vec2 textScreenPos = mCamera.worldToScreen(textPosition, getWindowWidth(), getWindowHeight());

	// Path of the random walks
	int numberOfSteps = 100;
	int numberOfStartingPoints = 3;
	// Weight to define how much the interaction should add up
	float interactionWeight = 0.1;
	vector<vector<vec3>> walkPath;

	// Step counter of the random walks
	int currentStep = 0;

	// Boolean to keep track of updating
	bool updating = true;

	// Method to generate random points in XYZ
	static vector<vec3> generatePoints(int numPoints, float minX, float maxX, float minY, float maxY, float minZ, float maxZ);

	// Method to generate a random walk
	// Random walk is generated in spherical coordinates, with the definitions as in https://en.wikipedia.org/wiki/Spherical_coordinate_system
	// Starting point is given with startX, startY and startZ
	static vec3 generateRandomWalk(float startX, float startY, float startZ, float maxR = 1, float maxPhi = M_PI, float maxTheta = 2 * M_PI);

	// Random walk with weight
	// Weights increase in the direction of a starting point
	vec3 generateWeightedRandomWalk(float x, float y, float z);
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
	mStartPoints = generatePoints(numberOfStartingPoints, -1, 1, -1, 1, -1, 1);

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
				if(currentStep == 1)
				{
					// The first walk should not be weighted yet, since the direction of a point with a point is a Nan
					walkPath[index][currentStep] = generateRandomWalk(previousPoint.x, previousPoint.y, previousPoint.z, 0.2);
				}
				else
				{
					walkPath[index][currentStep] = generateWeightedRandomWalk(previousPoint.x, previousPoint.y, previousPoint.z);	
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

vector<vec3> MonteCarloTreesApp::generatePoints(int numPoints, float minX, float maxX, float minY, float maxY, float minZ, float maxZ)
{
	// Seed the RNG
	std::random_device rd;
	std::mt19937 rng(rd());  // NOLINT(cert-msc51-cpp)

	// Define distributions for each coordinate
	std::uniform_real_distribution<float> xDistribution(minX, maxX);
	std::uniform_real_distribution<float> yDistribution(minY, maxY);
	std::uniform_real_distribution<float> zDistribution(minZ, maxZ);

	// Vector to store the generated points
	std::vector<glm::vec3> generatedPoints;

	// Generate random starting points
	for (int i = 0; i < numPoints; ++i) {
		float x = xDistribution(rng);
		float y = yDistribution(rng);
		float z = zDistribution(rng);

		generatedPoints.emplace_back(x, y, z);
	}

	return generatedPoints;
}

vec3 MonteCarloTreesApp::generateRandomWalk(float startX, float startY, float startZ, float maxR, float maxPhi, float maxTheta)
{
	// Input checks
	if (maxR < 0) {
		CI_LOG_E("Error: maxR should not be smaller than 0. Setting maxR to 0.");
		maxR = 0;
	}

	// Seed the RNG
	std::random_device rd;
	std::mt19937 rng(rd());

	// Define distributions for each coordinate
	std::uniform_real_distribution<float> rDistribution(0, maxR);
	std::uniform_real_distribution<float> phiDistribution(0, maxPhi);
	std::uniform_real_distribution<float> thetaDistribution(0, maxTheta);

	// Generate random walk
	float r = rDistribution(rng);
	float phi = phiDistribution(rng);
	float theta = thetaDistribution(rng);

	vec3 randomWalk = { startX + r * cos(phi) * sin(theta), startY + r * sin(phi) * sin(theta), startZ + r * cos(theta) };

	return randomWalk;
}

vec3 MonteCarloTreesApp::generateWeightedRandomWalk(float x, float y, float z)
{
	// Generate normalized random walk
	vec3 randomWalk = generateRandomWalk(x, y, z, 1, M_PI, 2 * M_PI);

	// Loop over starting points to find attractions
	auto inputPoint = vec3(x, y, z);
	auto interaction = vec3 (0.0f, 0.0f, 0.0f);
	for(auto& point : mStartPoints)
	{
		// Find direction between input point and starting point
		ci::vec3 direction = glm::normalize(inputPoint - point);
		// Find distance between input point and starting point
		float distance = glm::length(inputPoint - point);
		// Define interaction magnitude as 1/r^2
		float forceMagnitude = 1.0f / (distance * distance);
		// Calculate interaction
		interaction += forceMagnitude * direction;
	}

	// Print the resulting interaction to the console
	console() << "Interaction: X=" << interaction.x
		<< ", Y=" << interaction.y
		<< ", Z=" << interaction.z << endl;

	// Calculate resulting random walk
	return randomWalk * interaction * interactionWeight;
}

CINDER_APP(MonteCarloTreesApp, RendererGl)

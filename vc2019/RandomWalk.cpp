#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"

#include <random>
#include "RandomWalk.h"

#include <cinder/app/App.h>
#include <cinder/app/App.h>
#include <cinder/app/App.h>
#include <cinder/app/App.h>

using namespace ci;
using namespace ci::app;
using namespace std;

vector<glm::vec3> RandomWalk::generatePoints(int numPoints, float minX, float maxX, float minY, float maxY, float minZ, float maxZ)
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

vec3 RandomWalk::generateRandomWalkCircle(float startX, float startY, float startZ, float maxR, float maxPhi, float maxTheta)
{
	// Input checks
	if (maxR < 0) {
		CI_LOG_E("Error: maxR should not be smaller than 0. Setting maxR to 0.");
		maxR = 0;
	}
	if (maxPhi < 0) {
		CI_LOG_E("Error: maxPhi should not be smaller than 0. Setting maxPhi to 0.");
		maxPhi = 0;
	}
	if (maxTheta < 0) {
		CI_LOG_E("Error: maxTheta should not be smaller than 0. Setting maxTheta to 0.");
		maxTheta = 0;
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

vec3 RandomWalk::generateRandomWalkCartesian(float startX, float startY, float startZ, float maxX, float maxY, float maxZ)
{
	// Input checks
	// no input checks

	// Seed the RNG
	std::random_device rd;
	std::mt19937 rng(rd());

	// Define distributions for each coordinate
	std::uniform_real_distribution<float> xDistribution(maxX < 0 ? maxX : 0, maxX < 0 ? 0 : maxX);
	std::uniform_real_distribution<float> yDistribution(maxY < 0 ? maxY : 0, maxY < 0 ? 0 : maxY);
	std::uniform_real_distribution<float> zDistribution(maxZ < 0 ? maxZ : 0, maxZ < 0 ? 0 : maxZ);

	// Generate random walk
	float x = xDistribution(rng);
	float y = yDistribution(rng);
	float z = zDistribution(rng);

	vec3 randomWalk = { startX + x, startY + y, startZ + z };

	return randomWalk;
}

vec3 RandomWalk::generateWeightedRandomWalk(float x, float y, float z, vector<vec3> mStartPoints, float screeningSize, float interactionWeight)
{
	// Use current position to find interaction forces
	// Loop over starting points to find attractions
	auto inputPoint = vec3(x, y, z);
	auto interaction = vec3(0.0f, 0.0f, 0.0f);
	for (auto& point : mStartPoints)
	{
		// Calculate difference vector
		vec3 difference = inputPoint - point;
		// Find direction between input point and starting point
		vec3 direction = glm::normalize(difference);
		// Find distance between input point and starting point
		float distance = glm::length(difference);
		// Define interaction magnitude as 1/r^2
		float forceMagnitude = 1.0f / (distance * distance);

		// Particles seem to fly away when too close to a particle; interaction explodes
		if (distance < screeningSize)
		{
			console() << "Particle screened." << endl;
			interaction = { 1, 1, 1 };
			break;
		}

		// Calculate interaction
		interaction -= forceMagnitude * direction;
	}

	// Take into account the interaction weight (scaling factor, to line up all units)
	interaction = interaction * interactionWeight;

	// Generate normalized random walk with interaction as magnitude
	//float r = glm::length(interaction);
	//float phi = glm::acos(interaction.z / r);
	//float theta = glm::sign(interaction.y) * glm::acos(interaction.x / glm::sqrt(pow(interaction.x, 2) + pow(interaction.y, 2)));
	vec3 randomWalk = generateRandomWalkCartesian(x, y, z, interaction.x, interaction.y, interaction.z);

	// Print the resulting interaction to the console
	console() << "|Interaction| = " << glm::length(interaction) << endl;

	// Calculate resulting random walk
	return randomWalk;
}

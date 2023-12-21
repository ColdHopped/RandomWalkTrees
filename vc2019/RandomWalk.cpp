#include "cinder/gl/gl.h"
#include <random>
#include "RandomWalk.h"

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

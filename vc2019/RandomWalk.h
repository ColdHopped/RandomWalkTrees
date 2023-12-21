#include "cinder/gl/gl.h"

class RandomWalk
{
private:


public:
	// Method to generate random points in XYZ
	static std::vector<glm::vec3> generatePoints(int numPoints, float minX, float maxX, float minY, float maxY, float minZ, float maxZ);

	// Method to generate a random walk
	// Random walk is generated in spherical coordinates, with the definitions as in https://en.wikipedia.org/wiki/Spherical_coordinate_system
	// Starting point is given with startX, startY and startZ
	static glm::vec3 generateRandomWalkCircle(float startX, float startY, float startZ, float maxR = 1, float maxPhi = M_PI, float maxTheta = 2 * M_PI);

	// Method to generate a random walk
	// Random walk is generated in Cartesian coordinates, so cardinal directions X, Y and Z
	// Starting point is given with startX, startY and startZ
	static glm::vec3 generateRandomWalkCartesian(float startX, float startY, float startZ, float maxX, float maxY, float maxZ);

	// Random walk with weight
	// Weights increase in the direction of a starting point
	static glm::vec3 generateWeightedRandomWalk(float x, float y, float z, std::vector<glm::vec3> mStartPoints, float screeningSize, float interactionWeight);

};


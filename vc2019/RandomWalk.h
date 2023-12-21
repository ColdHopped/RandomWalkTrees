#include "cinder/gl/gl.h"

class RandomWalk
{
private:


public:
	// Method to generate random points in XYZ
	static std::vector<glm::vec3> generatePoints(int numPoints, float minX, float maxX, float minY, float maxY, float minZ, float maxZ);


};


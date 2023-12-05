#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class MonteCarloTreesApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void MonteCarloTreesApp::setup()
{
}

void MonteCarloTreesApp::mouseDown( MouseEvent event )
{
}

void MonteCarloTreesApp::update()
{
}

void MonteCarloTreesApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( MonteCarloTreesApp, RendererGl )

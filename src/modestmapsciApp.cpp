#include "cinder/app/AppBasic.h"
#include "Map.h"
#include "BingMapsProvider.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class modestmapsciApp : public AppBasic {
 public:

	void setup();
	void keyUp( KeyEvent event );
	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void mouseWheel( MouseEvent event );
	void draw();
	void resize( ResizeEvent event);
	
	Map map;
	Vec2f pMouse;
};

void modestmapsciApp::setup()
{
	setFrameRate(60.0);
	map.setup(new BingMapsProvider(), this->getWindowWidth(), this->getWindowHeight());
}

void modestmapsciApp::keyUp( KeyEvent event) 
{
	char key = event.getChar();
	if (key == '+' || key == '=') {
		if (map.getZoom() < 19) {
			map.zoomIn();
		}
	}
	else if (key == '-' || key == '_') {
		if (map.getZoom() > 0) {
			map.zoomOut();
		}
	}	
	else if (key == 'f' || key == 'F') {
		setFullScreen(!isFullScreen());
	}
}

void modestmapsciApp::mouseDown( MouseEvent event )
{
	pMouse = event.getPos();
}

void modestmapsciApp::mouseDrag( MouseEvent event )
{
	Vec2f diff = event.getPos() - pMouse;
	map.panBy(diff.x, diff.y);
	pMouse = event.getPos();	
}

void modestmapsciApp::mouseWheel( MouseEvent event )
{
	float delta = event.getWheelIncrement();
	Vec2f pos = event.getPos();
	map.scaleBy(delta > 0 ? 1.05 : 1.0/1.05, pos.x, pos.y);
}

void modestmapsciApp::draw()
{
	gl::clear( Color( 0.1f, 0.1f, 0.1f ) );
	map.draw();
}

void modestmapsciApp::resize( ResizeEvent event )
{
	map.width = event.getWidth();
	map.height = event.getHeight();
}


// This line tells Cinder to actually create the application
CINDER_APP_BASIC( modestmapsciApp, RendererGl )

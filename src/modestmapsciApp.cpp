#include "cinder/app/AppBasic.h"
#include "Map.h"
#include "BingMapsProvider.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class modestmapsciApp : public AppBasic {
 public:

	void setup();
	void touchesBegan( TouchEvent event );
	void touchesMoved( TouchEvent event );
	void touchesEnded( TouchEvent event );
	void keyUp( KeyEvent event );
	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void mouseWheel( MouseEvent event );
	void mouseMove( MouseEvent event );
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
	int key = event.getCode();
	if (key == KeyEvent::KEY_LEFT) {
		map.panLeft();
	}
	else if (key == KeyEvent::KEY_RIGHT) {
		map.panRight();
	}
	else if (key == KeyEvent::KEY_UP) {
		map.panUp();
	}
	else if (key == KeyEvent::KEY_DOWN) {
		map.panDown();
	}
	else if (key == KeyEvent::KEY_KP_PLUS || key == KeyEvent::KEY_PLUS || key == KeyEvent::KEY_EQUALS) {
		if (map.getZoom() < 19) {
			map.zoomIn();
		}
	}
	else if (key == KeyEvent::KEY_KP_MINUS || key == KeyEvent::KEY_MINUS || key == KeyEvent::KEY_UNDERSCORE) {
		if (map.getZoom() > 0) {
			map.zoomOut();
		}
	}	
	else if (key == KeyEvent::KEY_f) {
		setFullScreen(!isFullScreen());
	}
}

void modestmapsciApp::mouseMove( MouseEvent event )
{
	cout << map.pointLocation(Vec2d(event.getX(), event.getY())) << endl;
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
	if (fabs(delta)) {
		if (event.isShiftDown()) {
			map.rotateBy(delta > 0 ? M_PI/72.0 : -M_PI/72.0, pos.x, pos.y);
		}
		else {
			map.scaleBy(delta > 0 ? 1.05 : 1.0/1.05, pos.x, pos.y);
		}
	}
}

void modestmapsciApp::draw()
{
	gl::clear( Color( 0.1f, 0.1f, 0.1f ) );
	map.draw();
}

void modestmapsciApp::touchesBegan( TouchEvent event )
{
	cout << event << endl;
}
void modestmapsciApp::touchesMoved( TouchEvent event )
{
	cout << event << endl;
}
void modestmapsciApp::touchesEnded( TouchEvent event )
{
	cout << event << endl;
}
	
void modestmapsciApp::resize( ResizeEvent event )
{
	map.width = event.getWidth();
	map.height = event.getHeight();
}


// This line tells Cinder to actually create the application
CINDER_APP_BASIC( modestmapsciApp, RendererGl )

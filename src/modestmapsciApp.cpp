#include "cinder/app/AppBasic.h"
#include "Map.h"
#include "BingMapsProvider.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class modestmapsciApp : public AppBasic {
 public:

	void prepareSettings(Settings *settings);
	void setup();
	void touchesBegan( TouchEvent event );
	void touchesMoved( TouchEvent event );
	void touchesEnded( TouchEvent event );
	void keyUp( KeyEvent event );
//	void mouseDown( MouseEvent event );
//	void mouseDrag( MouseEvent event );
//	void mouseWheel( MouseEvent event );
//	void mouseMove( MouseEvent event );
	void draw();
	void resize( ResizeEvent event);
	
	Map map;
	Vec2f pMouse;
	
	std::map<uint32_t, Vec2f> prevTouches;
};

void modestmapsciApp::prepareSettings(Settings *settings) {
	settings->enableMultiTouch(true);
}

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

/*void modestmapsciApp::mouseMove( MouseEvent event )
{
	//cout << map.pointLocation(Vec2d(event.getX(), event.getY())) << endl;
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
}*/

void modestmapsciApp::draw()
{
	gl::clear( Color( 0.0f, 0.0f, 0.0f ) );
	map.draw();
	gl::color( Color( 1.0f, 1.0f, 1.0f ) );
	std::vector<TouchEvent::Touch> touches = getActiveTouches();
	for (int i = 0; i < touches.size(); i++) {
		gl::drawStrokedCircle( touches[i].getPos(), 20 );
	}
}



void modestmapsciApp::touchesBegan( TouchEvent event )
{
	for (int i = 0; i < event.getTouches().size(); i++) {
		prevTouches[event.getTouches()[i].getId()] = event.getTouches()[i].getPos();
	}
}
	
void modestmapsciApp::touchesMoved( TouchEvent event )
{
	std::vector<TouchEvent::Touch> touches = getActiveTouches();
	if (touches.size() == 1) {
		Vec2f prevTouch = prevTouches[touches[0].getId()];
		map.panBy(touches[0].getX() - prevTouch.x, touches[0].getY() - prevTouch.y);
		prevTouches[touches[0].getId()] = touches[0].getPos();
	}
	if (touches.size() == 2) {
		Vec2f p0 = touches[0].getPos();
		Vec2f p1 = touches[1].getPos();
		cout << p0 << " " << p1 << endl;
		double startDist = p0.distance(p1);
		Vec2f startCenter = p0.lerp(0.5, p1);
		double startRot = atan2(p1.y-p0.y,p1.x-p0.x);
		Vec2f p2 = prevTouches[touches[0].getId()];
		Vec2f p3 = prevTouches[touches[1].getId()];
		cout << p2 << " " << p3 << endl;
		double endDist = p2.distance(p3);
		Vec2f endCenter = p2.lerp(0.5, p3);
		double endRot = atan2(p3.y-p2.y,p3.x-p2.x);
		//////
		double sc = startDist / endDist;
		double r = startRot - endRot;
		map.panBy(startCenter.x-endCenter.x,startCenter.y-endCenter.y);
		map.scaleBy(sc, endCenter.x, endCenter.y);
		map.rotateBy(r, endCenter.x, endCenter.y); 
		//////
		prevTouches[touches[0].getId()] = p0;
		prevTouches[touches[1].getId()] = p1;
	}
}
void modestmapsciApp::touchesEnded( TouchEvent event )
{

}
	
void modestmapsciApp::resize( ResizeEvent event )
{
	map.width = event.getWidth();
	map.height = event.getHeight();
}


// This line tells Cinder to actually create the application
CINDER_APP_BASIC( modestmapsciApp, RendererGl )

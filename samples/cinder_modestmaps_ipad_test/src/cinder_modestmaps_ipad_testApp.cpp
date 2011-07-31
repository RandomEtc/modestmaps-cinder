#include "cinder/app/AppCocoaTouch.h"
#include "cinder/app/Renderer.h"
#include "cinder/Surface.h"
#include "cinder/gl/Texture.h"
#include "cinder/Camera.h"
#include "Map.h"
//#include "TemplatedMapProvider.h"
#include "BingMapsProvider.h"
//#include "OpenStreetMapProvider.h"

using namespace ci;
using namespace ci::app;
using namespace ci::modestmaps;
using namespace std;

class cinder_modestmaps_ipad_testApp : public AppCocoaTouch {
public:
	
	void setup();
	void touchesBegan( TouchEvent event );
	void touchesMoved( TouchEvent event );
	void touchesEnded( TouchEvent event );

	void draw();
	void resize( ResizeEvent event);
	
	Map map;
	CameraOrtho camera;
	Vec2f pMouse;
	
	std::map<uint32_t, Vec2f> prevTouches;
};

void cinder_modestmaps_ipad_testApp::setup()
{
	float w = getWindowWidth();
	float h = getWindowHeight();
	camera.setOrtho(0.0f,w,h,0.0f,-1.0f,1.0f);
	map.setup(new BingMapsProvider(), getWindowWidth(), getWindowHeight());
}

void cinder_modestmaps_ipad_testApp::draw()
{
	gl::clear( Color( 0x50/255.0f, 0x59/255.0f, 0x64/255.0f ) ); 

	gl::setMatrices( camera );
	
	map.draw();
	
	// draw touch points:
//	gl::color( Color( 1.0f, 1.0f, 1.0f ) );
//	std::vector<TouchEvent::Touch> touches = getActiveTouches();
//	for (int i = 0; i < touches.size(); i++) {
//		gl::drawStrokedCircle( touches[i].getPos(), 20 );
//	}
	
}



void cinder_modestmaps_ipad_testApp::touchesBegan( TouchEvent event )
{
	for (int i = 0; i < event.getTouches().size(); i++) {
		prevTouches[event.getTouches()[i].getId()] = event.getTouches()[i].getPos();
	}
}

void cinder_modestmaps_ipad_testApp::touchesMoved( TouchEvent event )
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
		Vec2f p2 = prevTouches[touches[0].getId()];
		Vec2f p3 = prevTouches[touches[1].getId()];

		double sc = p0.distance(p1) / p2.distance(p3);
		double r = atan2(p1.y-p0.y,p1.x-p0.x) - atan2(p3.y-p2.y,p3.x-p2.x);

		Vec2f endCenter = p0.lerp(0.5, p1);
		Vec2f startCenter = p2.lerp(0.5, p3);
		
		map.panBy(endCenter - startCenter);
		map.scaleBy(sc, endCenter.x, endCenter.y);
		map.rotateBy(r, endCenter.x, endCenter.y); 

		prevTouches[touches[0].getId()] = p0;
		prevTouches[touches[1].getId()] = p1;
	}
}
void cinder_modestmaps_ipad_testApp::touchesEnded( TouchEvent event )
{
	for (int i = 0; i < event.getTouches().size(); i++) {
        if (prevTouches.find(event.getTouches()[i].getId()) != prevTouches.end()) {
            prevTouches.erase(event.getTouches()[i].getId());
        }
	}	
}

void cinder_modestmaps_ipad_testApp::resize( ResizeEvent event )
{
	float w = getWindowWidth();
	float h = getWindowHeight();
	camera.setOrtho(0.0f,w,h,0.0f,-1.0f,1.0f);
	map.setSize( w, h );
}

CINDER_APP_COCOA_TOUCH( cinder_modestmaps_ipad_testApp, RendererGl )

#include "cinder/app/AppNative.h"
#include "cinder/gl/TextureFont.h"
#include "Map.h"
#include "OpenStreetMapProvider.h"
#include "TemplatedMapProvider.h"

using namespace ci;
using namespace ci::gl;
using namespace ci::app;
using namespace ci::modestmaps;
using namespace std;

class MultiTouchApp : public AppNative {
public:
	
    void prepareSettings( Settings *settings );
    
	void setup();
	void update();
	void draw();
    
	void resize( ResizeEvent event);

	void keyUp( KeyEvent event );
    
	void touchesBegan( TouchEvent event );
	void touchesMoved( TouchEvent event );
	void touchesEnded( TouchEvent event );
    	
	Map mMap;	
	std::map<uint32_t, Vec2f> mPrevTouches;

    TextureFontRef mTextureFont;
};

void MultiTouchApp::prepareSettings( Settings *settings )
{
    settings->enableMultiTouch( true );
}

void MultiTouchApp::setup()
{
    mTextureFont = TextureFont::create( Font("Helvetica", 12) );
	mMap.setup( OpenStreetMapProvider::create(), getWindowSize());
//	mMap.setup( TemplatedMapProvider::create("http://localhost:8000/{Z}/{Y}-{X}.png", 0, 5), getWindowSize());
    mMap.setExtent( MapExtent(61.087969, 49.250497, 3.686775, -12.353263) );
//    setFullScreen( true );
}

void MultiTouchApp::update()
{
    mMap.update();
}

void MultiTouchApp::keyUp( KeyEvent event ) 
{
//    mMap = Map();
//	mMap.setup( OpenStreetMapProvider::create(), getWindowSize());
    if (event.getChar() == 'o') {
        mMap.setMapProvider( OpenStreetMapProvider::create() );
    }
    else if (event.getChar() == 'l') {
        mMap.setMapProvider( TemplatedMapProvider::create("http://localhost:8000/{Z}/{Y}-{X}.png", 0, 5) );
    }
//    else if (event.getKey() == 'b') {
    // TODO: Bing
//    }    
}

void MultiTouchApp::draw()
{
	gl::clear( Color::white() );
    
    Vec2f windowSize = getWindowSize();
	gl::setMatricesWindow( windowSize );
	
    // draw map
	mMap.draw();

    gl::enableAlphaBlending();
	
    // draw copyright notice
    string notice = "Map data CC-BY-SA OpenStreetMap.org contributors.";
    Vec2f noticeSize = mTextureFont->measureString(notice);
    Vec2f noticePadding(10.0f,10.0f - mTextureFont->getFont().getAscent());
    gl::color( Color::black() );
    mTextureFont->drawString(notice, windowSize - noticeSize - noticePadding + Vec2f(1,1));
    gl::color( Color::white() );
    mTextureFont->drawString(notice, windowSize - noticeSize - noticePadding);
    
    // draw touch points:
    gl::color( ColorA( 1.0f, 1.0f, 1.0f, 0.25f ) );
    std::vector<TouchEvent::Touch> touches = getActiveTouches();
    for (int i = 0; i < touches.size(); i++) {
        gl::drawSolidCircle( touches[i].getPos(), 20 );
    }	
    
    gl::disableAlphaBlending();
}

void MultiTouchApp::touchesBegan( TouchEvent event )
{
	for (int i = 0; i < event.getTouches().size(); i++) {
		mPrevTouches[event.getTouches()[i].getId()] = event.getTouches()[i].getPos();
	}
}

void MultiTouchApp::touchesMoved( TouchEvent event )
{
	std::vector<TouchEvent::Touch> touches = getActiveTouches();
	if (touches.size() == 1) {
		Vec2f prevTouch = mPrevTouches[touches[0].getId()];
		mMap.panBy(touches[0].getX() - prevTouch.x, touches[0].getY() - prevTouch.y);
		mPrevTouches[touches[0].getId()] = touches[0].getPos();
	}
	if (touches.size() == 2) {
		Vec2f p0 = touches[0].getPos();
		Vec2f p1 = touches[1].getPos();
		Vec2f p2 = mPrevTouches[touches[0].getId()];
		Vec2f p3 = mPrevTouches[touches[1].getId()];
        
		double sc = p0.distance(p1) / p2.distance(p3);
		double r = atan2(p1.y-p0.y,p1.x-p0.x) - atan2(p3.y-p2.y,p3.x-p2.x);
        
		Vec2f endCenter = p0.lerp(0.5, p1);
		Vec2f startCenter = p2.lerp(0.5, p3);
		
		mMap.panBy(endCenter - startCenter);
		mMap.scaleBy(sc, endCenter.x, endCenter.y);
		mMap.rotateBy(r, endCenter.x, endCenter.y); 
        
		mPrevTouches[touches[0].getId()] = p0;
		mPrevTouches[touches[1].getId()] = p1;
	}
}

void MultiTouchApp::touchesEnded( TouchEvent event )
{
	for (int i = 0; i < event.getTouches().size(); i++) {
        if (mPrevTouches.find(event.getTouches()[i].getId()) != mPrevTouches.end()) {
            mPrevTouches.erase(event.getTouches()[i].getId());
        }
	}	
}

void MultiTouchApp::resize( ResizeEvent event )
{
//    MapExtent extent = mMap.getExtent();    
	mMap.setSize( getWindowSize() );
//    mMap.setExtent( extent, false ); // TODO: optionally, don't fit biggest dimension but preserve smallest instead?
}

CINDER_APP_NATIVE( MultiTouchApp, RendererGl )

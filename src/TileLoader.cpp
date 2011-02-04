/*
 *  TileLoader.cpp
 *  modestmaps-ci
 *
 *  Created by Tom Carden on 8/27/10.
 *  Copyright 2010 Stamen Design. All rights reserved.
 *
 */

#include "TileLoader.h"
#include "cinder/ip/Fill.h"

#if defined( CINDER_COCOA )
#include <objc/objc-auto.h>
#endif

namespace cinder { namespace modestmaps {

void TileLoader::requestTile( const Url &url, const Coordinate &key )
{
	pendingCompleteMutex.lock();
	pending.insert(key);
	pendingCompleteMutex.unlock();	
	//std::cout << "initing thread for " << url.str() << " and " << key << std::endl;	
	//loadSurfaceUrl(url, key);
	std::thread loaderThread( &TileLoader::loadSurfaceUrl, this, url, key );
}


void TileLoader::loadSurfaceUrl(const Url &url, const Coordinate &coord )
{
#if defined( CINDER_COCOA )
	// borrowed from https://llvm.org/svn/llvm-project/lldb/trunk/source/Host/macosx/Host.mm
  #if MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_5
	// On Leopard and earlier there is no way objc_registerThreadWithCollector
	// function, so we do it manually.
	auto_zone_register_thread(auto_zone());
  #else
	// On SnowLoepard and later we just call the thread registration function.
	objc_registerThreadWithCollector();
  #endif	
#endif	
	
	std::cout << "threaded loading " << url.str() << " for " << coord << std::endl;
	
	Surface image;
	try {
		image = Surface( loadImage( loadUrl( url ) ) );
	}
	catch( ... ) {
		std::cout << "Failed to load: " << url.str() << std::endl;
		// create a dummy tile
		image = Surface( 256, 256, true );
		ip::fill( &image, Color( 1.0f, 0.0f, 0.0f ) );
	}
	
	pendingCompleteMutex.lock();
	completed[coord] = image;
	pending.erase(coord);  
	pendingCompleteMutex.unlock();
}

void TileLoader::processQueue(std::vector<Coordinate> &queue, AbstractMapProvider *provider)
{
	while (pending.size() < MAX_PENDING && queue.size() > 0) {
		Coordinate coord = *(queue.begin());
		Coordinate key = Coordinate(coord);
		std::vector<std::string> urls = provider->getTileUrls(coord);
		if (!urls.empty()) {
			//std::cout << "loading " << urls[0] << " for " << coord << std::endl;
			// TODO: more than one image
			Url url( urls[0] );
			requestTile(url, key);
		}
		queue.erase(queue.begin());
	}
}

void TileLoader::transferTextures(std::map<Coordinate, gl::Texture> &images)
{
	pendingCompleteMutex.lock();
	while (!completed.empty()) {
		std::map<Coordinate, Surface>::iterator iter = completed.begin();
		images[iter->first] = gl::Texture(iter->second);		
		completed.erase(iter);
	}
	pendingCompleteMutex.unlock();
}

} } // namespace
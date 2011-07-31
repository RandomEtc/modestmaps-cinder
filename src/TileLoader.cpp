/*
 *  TileLoader.cpp
 *  modestmaps-ci
 *
 *  Created by Tom Carden on 8/27/10.
 *  Copyright 2010 Stamen Design. All rights reserved.
 *
 */

#include "TileLoader.h"

#if defined( CINDER_COCOA )
#include <objc/objc-auto.h>
#endif

namespace cinder { namespace modestmaps {

void TileLoader::doThreadedPaint( const Coordinate &coord )
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

	Surface image = provider->createSurface( coord );
    
	pendingCompleteMutex.lock();
	completed[coord] = image;
	pending.erase(coord);  
	pendingCompleteMutex.unlock();
}

void TileLoader::processQueue(std::vector<Coordinate> &queue )
{
	while (pending.size() < MAX_PENDING && queue.size() > 0) {
		Coordinate key = *(queue.begin());

        pendingCompleteMutex.lock();
        pending.insert(key);
        pendingCompleteMutex.unlock();	
        
        std::thread loaderThread( &TileLoader::doThreadedPaint, this, key );
        
		queue.erase(queue.begin());
	}
}

void TileLoader::transferTextures(std::map<Coordinate, gl::Texture> &images)
{
	pendingCompleteMutex.lock();
	while (!completed.empty()) {
		std::map<Coordinate, Surface>::iterator iter = completed.begin();
        if (iter->second) {
            images[iter->first] = gl::Texture(iter->second);		
        }
		completed.erase(iter);
	}
	pendingCompleteMutex.unlock();
}

} } // namespace
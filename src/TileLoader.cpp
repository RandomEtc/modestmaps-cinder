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

	Surface image;
    
    if (provider) {
        image = provider->createSurface( coord );
    }
    
	pendingCompleteMutex.lock();
    if (pending.count(coord) > 0) {
        if (image) {
            completed[coord] = image;
        }
        pending.erase(coord);  
    } // otherwise clear was called so we should abandon this image to the ether
	pendingCompleteMutex.unlock();
}

void TileLoader::processQueue(std::vector<Coordinate> &queue )
{
	while (pending.size() < MAX_PENDING && queue.size() > 0) {
		Coordinate key = *(queue.begin());
		queue.erase(queue.begin());

        pendingCompleteMutex.lock();
        pending.insert(key);
        pendingCompleteMutex.unlock();	
        
        // TODO: consider using a single thread and a queue, rather than a thread per load?
        std::thread loaderThread( &TileLoader::doThreadedPaint, this, key );        
	}
}

void TileLoader::transferTextures(std::map<Coordinate, gl::Texture> &images)
{
    // TODO: consider using try_lock here because we can just wait til next frame if it fails
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
    
bool TileLoader::isPending(const Coordinate &coord)
{
    bool coordIsPending = false;
    pendingCompleteMutex.lock();
    coordIsPending = (pending.count(coord) > 0);
    pendingCompleteMutex.unlock();
    return coordIsPending;
}
    
void TileLoader::setMapProvider( MapProviderRef _provider )
{
	pendingCompleteMutex.lock();
    completed.clear();
    pending.clear();
	pendingCompleteMutex.unlock();
    provider = _provider;
}

} } // namespace
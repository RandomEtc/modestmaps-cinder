/*
 *  TileLoader.cpp
 *  modestmaps-ci
 *
 *  Created by Tom Carden on 8/27/10.
 *  Copyright 2010 Stamen Design. All rights reserved.
 *
 */

#include "TileLoader.h"

void TileLoader::requestTile( const Url &url, const Coordinate key )
{
	pendingCompleteMutex.lock();
	pending.insert(key);
	pendingCompleteMutex.unlock();
	std::thread loaderThread( &TileLoader::loadSurfaceUrl, this, url, key );	
}

void TileLoader::loadSurfaceUrl(const Url &url, const Coordinate &coord )
{
	Surface image( loadImage( loadUrl( url ) ) );
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
		if (urls.size() > 0) {
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


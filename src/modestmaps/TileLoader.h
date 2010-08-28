/*
 *  TileLoader.h
 *  modestmaps-ci
 *
 *  Created by Tom Carden on 8/27/10.
 *  Copyright 2010 Stamen Design. All rights reserved.
 *
 */

#include <set>
#include "cinder/Url.h"
#include "cinder/Thread.h"
#include "Coordinate.h"
#include "AbstractMapProvider.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"

using namespace ci;

// limit simultaneous calls to loadImage
#define MAX_PENDING 8

class TileLoader
{
public:
	void requestTile( const Url &url, const Coordinate key );
	
	void loadSurfaceUrl( const Url &url, const Coordinate &coord );
	
	void processQueue( std::vector<Coordinate> &queue, AbstractMapProvider *provider );
	
	void transferTextures( std::map<Coordinate, gl::Texture> &images);

	bool isPending(const Coordinate &coord) {
		return pending.count(coord) > 0;
	}
	
private:
	std::mutex pendingCompleteMutex;	
	std::set<Coordinate> pending;
	std::map<Coordinate, Surface> completed;
		
};

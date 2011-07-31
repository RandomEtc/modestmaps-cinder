#pragma once

#include <set>
#include <map>
#include "cinder/Thread.h"
#include "cinder/gl/Texture.h"
#include "Coordinate.h"
#include "MapProvider.h"

namespace cinder { namespace modestmaps {

// limit simultaneous calls to loadImage
#define MAX_PENDING 8

class TileLoader
{
public:
    
    TileLoader( MapProvider *_provider ): provider(_provider) {}
    
	void processQueue( std::vector<Coordinate> &queue );
	
	void transferTextures( std::map<Coordinate, gl::Texture> &images);

	bool isPending(const Coordinate &coord) {
		return pending.count(coord) > 0;
	}
	
private:
    
    void doThreadedPaint( const Coordinate &coord );
    
	std::mutex pendingCompleteMutex;	
	std::set<Coordinate> pending;
	std::map<Coordinate, Surface> completed;
		
    MapProvider *provider;
};

} } // namespace
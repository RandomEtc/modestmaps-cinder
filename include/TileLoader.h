#pragma once

#include <set>
#include <map>
#include "cinder/Cinder.h"
#include "cinder/Thread.h"
#include "cinder/gl/Texture.h"
#include "Coordinate.h"
#include "MapProvider.h"

namespace cinder { namespace modestmaps {

// limit simultaneous calls to loadImage
#define MAX_PENDING 8

class TileLoader;
typedef std::shared_ptr<TileLoader> TileLoaderRef;

class TileLoader
{
	
private:
    
    TileLoader( MapProviderRef _provider ): provider(_provider) {}
    
    void doThreadedPaint( const Coordinate &coord );
    
	std::mutex pendingCompleteMutex;	
	std::set<Coordinate> pending;
	std::map<Coordinate, Surface> completed;
    
    MapProviderRef provider;
    
public:
    
    static TileLoaderRef create( MapProviderRef provider )
    {
        return TileLoaderRef( new TileLoader( provider ) );
    }
    
	void processQueue( std::vector<Coordinate> &queue );
	
	void transferTextures( std::map<Coordinate, gl::Texture> &images);

	bool isPending(const Coordinate &coord) {
		return pending.count(coord) > 0;
	}
    
};
    
} } // namespace
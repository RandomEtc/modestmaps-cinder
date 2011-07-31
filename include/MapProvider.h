//
//  MapProvider.h
//  MultiTouch
//
//  Created by Tom Carden on 7/31/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#pragma once

#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/Surface.h"

#include "Location.h"
#include "Coordinate.h"

namespace cinder { namespace modestmaps {
    
class MapProvider {
public:
    
    virtual ~MapProvider() {}
    
    // facts about the tiles that exist:
	virtual Vec2i getTileSize() = 0;
    virtual int getMaxZoom() = 0;
    virtual int getMinZoom() = 0;

    // how the map turns geography into tiles:
	virtual Coordinate locationCoordinate(const Location &location) = 0;
	virtual Location coordinateLocation(const Coordinate &coordinate) = 0;
    
    // called from a background thread:
    virtual Surface createSurface( const Coordinate &coord ) = 0;
};
    
typedef std::shared_ptr<MapProvider> MapProviderRef;

} }

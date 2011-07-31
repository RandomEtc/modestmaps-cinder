#pragma once

#include <vector>
#include <string>
#include <sstream>

#include "cinder/Rand.h"
#include "cinder/Utilities.h"

#include "AbstractMapProvider.h"
#include "MercatorProjection.h"

namespace cinder { namespace modestmaps {
	
class TemplatedMapProvider : public AbstractMapProvider {
	
protected:
    
    std::string mUrlTemplate;
    int mMinZoom;
    int mMaxZoom;

	TemplatedMapProvider( const std::string &urlTemplate ): 
        mUrlTemplate(urlTemplate),
        mMinZoom(0),
        mMaxZoom(18),
        // this is the projection and transform you'll want for any Google-style map tile source:
        AbstractMapProvider( MercatorProjection::createWebMercator() )
    { }
    
	TemplatedMapProvider( const std::string &urlTemplate, const int &minZoom, const int &maxZoom ): 
        mUrlTemplate(urlTemplate),
        mMinZoom(minZoom),
        mMaxZoom(maxZoom),
        // this is the projection and transform you'll want for any Google-style map tile source:
        AbstractMapProvider( MercatorProjection::createWebMercator() )
    { }
    
public:
	
    static MapProviderRef create( const std::string &urlTemplate )
    {
        return MapProviderRef( new TemplatedMapProvider( urlTemplate ) );
    }

    static MapProviderRef create( const std::string &urlTemplate, const int &minZoom, const int &maxZoom )
    {
        return MapProviderRef( new TemplatedMapProvider( urlTemplate, minZoom, maxZoom ) );
    }
    
	int getMaxZoom() {
		return mMaxZoom;
	}
	
	int getMinZoom() {
		return mMinZoom;
	}    
    
	std::vector<std::string> getTileUrls(const Coordinate &rawCoordinate) {
		std::vector<std::string> urls;
		if (rawCoordinate.zoom >= 0 && rawCoordinate.zoom <= 19 
			&& rawCoordinate.row >= 0 && rawCoordinate.row < pow(2, rawCoordinate.zoom)) {
			Coordinate coordinate = sourceCoordinate(rawCoordinate);
			std::string url(mUrlTemplate);
			url.replace(url.find("{Z}"), 3, toString(coordinate.zoom));
			url.replace(url.find("{X}"), 3, toString(coordinate.column));
			url.replace(url.find("{Y}"), 3, toString(coordinate.row));
			urls.push_back(url);
		}
		return urls;
	}
	
};

} } // namespace
#pragma once

#include <vector>
#include <string>
#include <sstream>

#include "cinder/Rand.h"
#include "AbstractMapProvider.h"
#include "MercatorProjection.h"

namespace cinder { namespace modestmaps {
	
class BingMapsProvider : public AbstractMapProvider {
	
private:
	
	// TODO: we need to get these from the Bing API
	// ...so don't bother marking them const because they aren't :)
	std::vector<std::string> subdomains;
	
	BingMapsProvider(): 
		// this is the projection and transform you'll want for any Google-style map tile source:
        AbstractMapProvider( MercatorProjection::createWebMercator() )
	{
		// TODO: a better way to init a constant length vector or array of strings?
		subdomains.push_back("t0");
		subdomains.push_back("t1");
		subdomains.push_back("t2");
		subdomains.push_back("t3");
	}
	
	std::string getQuadKey(const int column, const int row, const int zoom) {
		std::stringstream key;
		for (int i = 1; i <= zoom; i++) {
			int digit = (((row >> zoom - i) & 1) << 1) | ((column >> zoom - i) & 1);
			key << digit;
		}
		return key.str();
	}
	
public: 
    
    static MapProviderRef create() {
        return MapProviderRef(new BingMapsProvider());
    }
    
	std::vector<std::string> getTileUrls(const Coordinate &rawCoordinate) {
		std::vector<std::string> urls;
		if (rawCoordinate.zoom >= 1 && rawCoordinate.zoom <= 19 
			&& rawCoordinate.row >= 0 && rawCoordinate.row < pow(2, rawCoordinate.zoom)) {
			Coordinate coordinate = sourceCoordinate(rawCoordinate);
			std::stringstream url;
			std::string subdomain = subdomains[ci::Rand::randInt(0, subdomains.size())];
			std::string quadkey = getQuadKey((int)coordinate.column, (int)coordinate.row, (int)coordinate.zoom);
			url << "http://" << subdomain << ".tiles.virtualearth.net/tiles/a" << quadkey << ".jpeg?g=543"; //&mkt={culture}&token={token}";
			urls.push_back(url.str());
		}
		return urls;
	}
	
};

} } // namespace

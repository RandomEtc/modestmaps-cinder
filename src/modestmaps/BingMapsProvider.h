#ifndef BING_MAPS_PROVIDER
#define BING_MAPS_PROVIDER

#include <vector>
#include <string>
#include <sstream>

#include "cinder/Rand.h"
#include "AbstractMapProvider.h"
#include "MercatorProjection.h"

class BingMapsProvider : public AbstractMapProvider {
	
public:
	
	std::vector<std::string> subdomains;
	
	BingMapsProvider(): 
		// this is the projection and transform you'll want for any Google-style map tile source:
		AbstractMapProvider(new MercatorProjection(26, Transformation(1.068070779e7, 0.0, 3.355443185e7, 0.0, -1.068070890e7, 3.355443057e7)))
	{
		subdomains.push_back("t0");
		subdomains.push_back("t1");
		subdomains.push_back("t2");
		subdomains.push_back("t3");
	}
	
	int tileWidth() {
		return 256;
	}
	
	int tileHeight() {
		return 256;
	}
	
	std::string getQuadKey(int column, int row, int zoom) {
		std::stringstream key;
		for (int i = 1; i <= zoom; i++) {
			int digit = (((row >> zoom - i) & 1) << 1) | ((column >> zoom - i) & 1);
			key << digit;
		}
		return key.str();
	}
	
	std::vector<std::string> getTileUrls(Coordinate rawCoordinate) {
		std::vector<std::string> urls;
		if (rawCoordinate.row >= 0 && rawCoordinate.row < pow(2, rawCoordinate.zoom)) {
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

#endif
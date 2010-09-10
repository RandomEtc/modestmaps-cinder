#ifndef OPENSTREETMAPPROVIDER
#define OPENSTREETMAPPROVIDER

#include <vector>
#include <string>
#include <sstream>

#include "cinder/Rand.h"
#include "AbstractMapProvider.h"
#include "MercatorProjection.h"

class OpenStreetMapProvider : public AbstractMapProvider {
	
public:
	
	std::vector<std::string> subdomains;
	
	OpenStreetMapProvider(): 
		// this is the projection and transform you'll want for any Google-style map tile source:
		AbstractMapProvider(new MercatorProjection(26, Transformation(1.068070779e7, 0.0, 3.355443185e7, 0.0, -1.068070890e7, 3.355443057e7)))
	{
		subdomains.push_back("");
		subdomains.push_back("a.");
		subdomains.push_back("b.");
		subdomains.push_back("c.");
	}
	
	int tileWidth() {
		return 256;
	}
	
	int tileHeight() {
		return 256;
	}
	
	std::vector<std::string> getTileUrls(Coordinate rawCoordinate) {
		std::vector<std::string> urls;
		if (rawCoordinate.row >= 0 && rawCoordinate.row < pow(2, rawCoordinate.zoom)) {
			Coordinate coordinate = sourceCoordinate(rawCoordinate);
			std::stringstream url;
			std::string subdomain = subdomains[ci::Rand::randInt(0, subdomains.size())];
			url << "http://" << subdomain << "tile.openstreetmap.org/" << (int)coordinate.zoom << "/" << (int)coordinate.column << "/" << (int)coordinate.row << ".png";
			std::cout << rawCoordinate << " --> " << url.str() << std::endl;
			urls.push_back(url.str());
		}
		return urls;
	}
	
};

#endif
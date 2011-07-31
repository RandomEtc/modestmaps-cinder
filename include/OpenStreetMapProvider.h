#pragma once

#include <vector>
#include <string>
#include <sstream>

#include "cinder/Rand.h"
#include "AbstractMapProvider.h"
#include "MercatorProjection.h"

namespace cinder { namespace modestmaps {
	
class OpenStreetMapProvider : public AbstractMapProvider {
	
public:
	
	std::vector<std::string> subdomains;
	
	OpenStreetMapProvider(): 
        // this is the projection and transform you'll want for any Google-style map tile source:
        AbstractMapProvider(new MercatorProjection( 0, 
                                                   Transformation::deriveTransformation( -M_PI,  M_PI, 0, 0, 
                                                                                          M_PI,  M_PI, 1, 0, 
                                                                                         -M_PI, -M_PI, 0, 1 ) ) )
	{
		// TODO: is there a better way to initialize a constant size vector or array of strings?
		subdomains.push_back("");
		subdomains.push_back("a.");
		subdomains.push_back("b.");
		subdomains.push_back("c.");
	}
		
	std::vector<std::string> getTileUrls(const Coordinate &rawCoordinate) {
		std::vector<std::string> urls;
		if (rawCoordinate.row >= 0 && rawCoordinate.row < pow(2, rawCoordinate.zoom)) {
			Coordinate coordinate = sourceCoordinate(rawCoordinate);
			std::stringstream url;
			std::string subdomain = subdomains[ci::Rand::randInt(0, subdomains.size())];
			url << "http://" << subdomain << "tile.openstreetmap.org/" << (int)coordinate.zoom << "/" << (int)coordinate.column << "/" << (int)coordinate.row << ".png";
			//std::cout << rawCoordinate << " --> " << url.str() << std::endl;
			urls.push_back(url.str());
		}
		return urls;
	}
	
};

} } // namespace
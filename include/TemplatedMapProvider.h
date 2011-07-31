#pragma once

#include <vector>
#include <string>
#include <sstream>

#include "cinder/Rand.h"
#include "AbstractMapProvider.h"
#include "MercatorProjection.h"

namespace cinder { namespace modestmaps {
	
class TemplatedMapProvider : public AbstractMapProvider {
	
public:
	
	std::string urlTemplate;
	
	TemplatedMapProvider(std::string _urlTemplate): 
        urlTemplate(_urlTemplate),
        // this is the projection and transform you'll want for any Google-style map tile source:
        AbstractMapProvider(new MercatorProjection( 0, 
                                                   Transformation::deriveTransformation( -M_PI,  M_PI, 0, 0, 
                                                                                          M_PI,  M_PI, 1, 0, 
                                                                                         -M_PI, -M_PI, 0, 1 ) ) )
    { }
	
	int tileWidth() {
		return 256;
	}
	
	int tileHeight() {
		return 256;
	}
	
	std::string format(double n) {
		std::stringstream s;
		s << (int)n;
		return s.str();
	}
	
	std::vector<std::string> getTileUrls(const Coordinate &rawCoordinate) {
		std::vector<std::string> urls;
		if (rawCoordinate.zoom >= 0 && rawCoordinate.zoom <= 19 
			&& rawCoordinate.row >= 0 && rawCoordinate.row < pow(2, rawCoordinate.zoom)) {
			Coordinate coordinate = sourceCoordinate(rawCoordinate);
			std::string url = urlTemplate;
			url.replace(url.find("{Z}"), 3, format(coordinate.zoom));
			url.replace(url.find("{X}"), 3, format(coordinate.column));
			url.replace(url.find("{Y}"), 3, format(coordinate.row));
			urls.push_back(url);
		}
		return urls;
	}
	
};

} } // namespace
#ifndef ABSTRACT_PROVIDER
#define ABSTRACT_PROVIDER

#include <cmath>
#include <vector>
#include <string>

#include "AbstractProjection.h"
#include "Coordinate.h"
#include "Location.h"

class AbstractMapProvider {
	
public:
	
	AbstractProjection *projection;
	
	AbstractMapProvider(AbstractProjection *_projection): projection(_projection) {}
	
	// TODO: I think virtual destructor stuff is needed here?
	
	virtual std::vector<std::string> getTileUrls(const Coordinate &coord)=0;
	virtual int tileWidth()=0;
	virtual int tileHeight()=0;
	
	Coordinate locationCoordinate(const Location &location) {
		return projection->locationCoordinate(location);
	}
	
	Location coordinateLocation(const Coordinate &coordinate) {
		return projection->coordinateLocation(coordinate);
	}
	
	Coordinate sourceCoordinate(const Coordinate &coordinate) {
		const double gridSize = pow(2.0, coordinate.zoom);
		
		double wrappedColumn = coordinate.column;
		if(wrappedColumn >= gridSize) {
			wrappedColumn = fmod(wrappedColumn, gridSize);
		}
		else {
			while (wrappedColumn < 0) {
				wrappedColumn += gridSize;
			}
		}
		
		return Coordinate(coordinate.row, wrappedColumn, coordinate.zoom);
	}
	
};

#endif
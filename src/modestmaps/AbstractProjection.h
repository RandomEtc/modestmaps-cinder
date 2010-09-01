#ifndef ABSTRACT_PROJECTION
#define ABSTRACT_PROJECTION

#include <cmath>
#include "cinder/Vector.h"

#include "Transformation.h"
#include "Coordinate.h"
#include "Location.h"

using namespace ci;

class AbstractProjection {

public:

	double zoom;
	Transformation transformation;

	AbstractProjection(double _zoom): zoom(_zoom), transformation(Transformation()) { }
	AbstractProjection(double _zoom, Transformation _t): zoom(_zoom), transformation(_t) { }

	virtual Vec2d rawProject(Vec2d point)=0;
	virtual Vec2d rawUnproject(Vec2d point)=0;

	Vec2d project(Vec2d point) {
		return transformation.transform(rawProject(point));
	}

	Vec2d unproject(Vec2d point) {
		return rawUnproject(transformation.untransform(point));
	}

	Coordinate locationCoordinate(Location location) {
		Vec2d point = project(Vec2d((M_PI/180.0) * location.lon, (M_PI/180.0) * location.lat));
		return Coordinate(point.y, point.x, zoom);
	}

	Location coordinateLocation(Coordinate coordinate) {
		coordinate = coordinate.zoomTo(zoom);
		Vec2d point = unproject(Vec2d(coordinate.column, coordinate.row));
		return Location((180.0/M_PI) * point.y, (180.0/M_PI) * point.x);
	}

};

#endif
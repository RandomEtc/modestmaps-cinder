#pragma once

#include <cmath>
#include "cinder/Cinder.h"
#include "cinder/Vector.h"

#include "Transformation.h"
#include "Coordinate.h"
#include "Location.h"

namespace cinder { namespace modestmaps {
    
class AbstractProjection {

public:

	double zoom;
	Transformation transformation;

	AbstractProjection(double _zoom): zoom(_zoom), transformation(Transformation()) { }
	AbstractProjection(double _zoom, Transformation _t): zoom(_zoom), transformation(_t) { }

	virtual ~AbstractProjection() {}
	
	virtual Vec2d rawProject(const Vec2d &point)=0;
	virtual Vec2d rawUnproject(const Vec2d &point)=0;

	Vec2d project(const Vec2d &point) {
		return transformation.transform(rawProject(point));
	}

	Vec2d unproject(const Vec2d &point) {
		return rawUnproject(transformation.untransform(point));
	}

	Coordinate locationCoordinate(const Location &location) {
		Vec2d point = project(Vec2d((M_PI/180.0) * location.lon, (M_PI/180.0) * location.lat));
		return Coordinate(point.y, point.x, zoom);
	}

	Location coordinateLocation(const Coordinate &coordinate) {
		// TODO: is this built into Cinder anyplace?
		static const double rad2deg = 180.0/M_PI;
		Coordinate zoomed = coordinate.zoomTo(zoom);
		Vec2d point = unproject(Vec2d(zoomed.column, zoomed.row));
		return Location(rad2deg * point.y, rad2deg * point.x);
	}

};

typedef std::shared_ptr<AbstractProjection> ProjectionRef;
    
} } // namespace
#pragma once

#include "AbstractProjection.h"
#include "Transformation.h"

namespace cinder { namespace modestmaps {
	
class MercatorProjection : public AbstractProjection {
	
public:
	
	MercatorProjection(double _zoom=0): AbstractProjection(_zoom, Transformation()) { }
	MercatorProjection(double _zoom, Transformation t): AbstractProjection(_zoom,t) { }
	
	Vec2d rawProject(const Vec2d &point) {
		return Vec2d(point.x, log(tan(0.25 * M_PI + 0.5 * point.y)));
	}
	
	Vec2d rawUnproject(const Vec2d &point) {
		return Vec2d(point.x, 2.0 * atan(pow(M_E, point.y)) - 0.5 * M_PI);
	}
	
};

} } // namespace
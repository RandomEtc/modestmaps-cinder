#pragma once

#include "AbstractProjection.h"
#include "Transformation.h"

namespace cinder { namespace modestmaps {
	
class LinearProjection : public AbstractProjection {
	
public:

	LinearProjection(double _zoom=0): AbstractProjection(_zoom, Transformation()) { }
	LinearProjection(double _zoom, Transformation t): AbstractProjection(_zoom,t) { }

	Vec2d rawProject(const Vec2d &point) {
		return Vec2d(point.x, point.y);
	}
	
	Vec2d rawUnproject(const Vec2d &point) {
		return Vec2d(point.x, point.y);
	}
	
};

} } // namespace
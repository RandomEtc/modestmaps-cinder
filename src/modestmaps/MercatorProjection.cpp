#include "MercatorProjection.h"
#include "cinder/Vector.h"

Vec2d MercatorProjection::rawProject(Vec2d point) {
	return Vec2d(point.x, log(tan(0.25 * M_PI + 0.5 * point.y)));
}

Vec2d MercatorProjection::rawUnproject(Vec2d point) {
	return Vec2d(point.x, 2.0 * atan(pow(M_E, point.y)) - 0.5 * M_PI);
}

#include "LinearProjection.h"

Vec2d LinearProjection::rawProject(Vec2d point) {
	return Vec2d(point.x, point.y);
}

Vec2d LinearProjection::rawUnproject(Vec2d point) {
	return Vec2d(point.x, point.y);
}

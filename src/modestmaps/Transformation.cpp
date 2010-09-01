#include "Transformation.h"
#include "cinder/Vector.h"

Vec2d Transformation::transform(Vec2d point) {
	return Vec2d(ax*point.x + bx*point.y + cx, ay*point.x + by*point.y + cy);
}

Vec2d Transformation::untransform(Vec2d point) {
	return Vec2d((point.x*by - point.y*bx - cx*by + cy*bx) / (ax*by - ay*bx), (point.x*ay - point.y*ax - cx*ay + cy*ax) / (bx*ay - by*ax));
}

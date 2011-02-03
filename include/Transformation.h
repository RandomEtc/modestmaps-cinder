#ifndef TRANSFORMATION
#define TRANSFORMATION

#include "cinder/Vector.h"

using namespace cinder;

class Transformation {
	
public:
	
	double ax, bx, cx, ay, by, cy;

	Transformation() : ax(1), bx(0), cx(0), ay(0), by(1), cy(0) {}
	
	Transformation(float _ax, float _bx, float _cx, float _ay, float _by, float _cy) : ax(_ax), bx(_bx), cx(_cx), ay(_ay), by(_by), cy(_cy) {}
	
	Transformation(const Transformation &t) : ax(t.ax), bx(t.bx), cx(t.cx), ay(t.ay), by(t.by), cy(t.cy) {}
	
	Vec2d transform(const Vec2d &point) {
		return Vec2d(ax*point.x + bx*point.y + cx, ay*point.x + by*point.y + cy);
	}
	
	Vec2d untransform(const Vec2d &point){
		return Vec2d((point.x*by - point.y*bx - cx*by + cy*bx) / (ax*by - ay*bx), (point.x*ay - point.y*ax - cx*ay + cy*ax) / (bx*ay - by*ax));
	}
	
};

#endif
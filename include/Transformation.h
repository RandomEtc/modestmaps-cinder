#pragma once

#include "cinder/Vector.h"

namespace cinder { namespace modestmaps {

class Transformation {
	
public:
	
	double ax, bx, cx, ay, by, cy;

	Transformation() : ax(1), bx(0), cx(0), ay(0), by(1), cy(0) {}
	
	Transformation(const double &_ax, const double &_bx, const double &_cx, 
                   const double &_ay, const double &_by, const double &_cy): ax(_ax), bx(_bx), cx(_cx), ay(_ay), by(_by), cy(_cy) {}
    
	Transformation(const Transformation &t) : ax(t.ax), bx(t.bx), cx(t.cx), ay(t.ay), by(t.by), cy(t.cy) {}
	
	Vec2d transform(const Vec2d &point) {
		return Vec2d(ax*point.x + bx*point.y + cx, ay*point.x + by*point.y + cy);
	}
	
	Vec2d untransform(const Vec2d &point){
		return Vec2d((point.x*by - point.y*bx - cx*by + cy*bx) / (ax*by - ay*bx), (point.x*ay - point.y*ax - cx*ay + cy*ax) / (bx*ay - by*ax));
	}
	
    // Generates a transform based on three pairs of points,
    // a1 -> a2, b1 -> b2, c1 -> c2.
    static Transformation deriveTransformation( const double &a1x, const double &a1y, 
                                                const double &a2x, const double &a2y,
                                                const double &b1x, const double &b1y, 
                                                const double &b2x, const double &b2y,
                                                const double &c1x, const double &c1y,
                                                const double &c2x, const double &c2y ) {
        const Vec3f d = linearSolution( a1x, a1y, a2x,
                                        b1x, b1y, b2x,
                                        c1x, c1y, c2x );
        const Vec3f e = linearSolution( a1x, a1y, a2y,
                                        b1x, b1y, b2y,
                                        c1x, c1y, c2y );
        return Transformation(d.x, d.y, d.z, e.x, e.y, e.z);
    };
    
    // Solves a system of linear equations.
    //
    //     t1 = (a * r1) + (b + s1) + c
    //     t2 = (a * r2) + (b + s2) + c
    //     t3 = (a * r3) + (b + s3) + c
    //
    // r1 - t3 are the known values.
    // a, b, c are the unknowns to be solved.
    // returns the a, b, c coefficients.
    static Vec3d linearSolution( const double &r1, const double &s1, const double &t1, 
                                 const double &r2, const double &s2, const double &t2, 
                                 const double &r3, const double &s3, const double &t3 ) {
        const double a = (((t2 - t3) * (s1 - s2)) - ((t1 - t2) * (s2 - s3)))
        / (((r2 - r3) * (s1 - s2)) - ((r1 - r2) * (s2 - s3)));
        
        const double b = (((t2 - t3) * (r1 - r2)) - ((t1 - t2) * (r2 - r3)))
        / (((s2 - s3) * (r1 - r2)) - ((s1 - s2) * (r2 - r3)));
        
        const double c = t1 - (r1 * a) - (s1 * b);
        
        return Vec3d(a,b,c);
    };    
    
};

} } // namespace

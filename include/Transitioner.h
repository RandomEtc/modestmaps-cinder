/*
 *  Transitioner.h
 *  pollen-cartagram
 *
 *  Created by Tom Carden on 2/4/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *   
 *  From "Smooth and efficient zooming and panning"
 *  by Jarke J. van Wijk and Wim A.A. Nuij
 *  You only need to understand section 3 (equations 1 through 5) 
 *  and then you can skip to equation 9, implemented below: 
 *
 */

#import <iostream>
#import "Map.h"
#import "Coordinate.h"
#import "Location.h"
#import "cinder/CinderMath.h"
#import "cinder/Vector.h"

namespace cinder { namespace modestmaps {
	
class Transitioner {
private:

	Map *map;
	
	// target position and scale
	Vec2d c0, c1;
	double w0, w1;
	
	// animation params
	double V, rho;
	
	// inbetween variables
	double u0, u1;
	double r0, r1, S, k;
		
	inline double b(const int &i);
	inline double r(const double &b);
	inline double u(const double &s);
	inline double w(const double &s);
			
public:
	
	bool active;
	double t0;

	// see section 6 for user testing to derive these values (they can be tuned)
	Transitioner(double _V = 0.9, double _rho = 1.42): V(_V), rho(_rho) {}
	
	void setMap(Map *_map) {
		map = _map;
		active = false;
	}
	
	// t0 = start time, seconds
	void setTarget(const Location &l1, const double &z1, const double &_t0) {
		
		std::cout << "setting target " << l1 << " @ " << z1 << " start time " << _t0 << std::endl;
		
		Coordinate start = map->getMapProvider()->locationCoordinate(map->getCenter());
        Coordinate end   = map->getMapProvider()->locationCoordinate(l1);
		
		start = start.zoomTo(0);
		end   = end.zoomTo(0);

        std::cout << "start " << start << std::endl;
        std::cout << "end " << end << std::endl;
		
		c0 = Vec2d(start.column, start.row);
        c1 = Vec2d(end.column, end.row);

		std::cout << "c0 " << c0 << std::endl;
        std::cout << "c1 " << c1 << std::endl;

		// how much world can we see at zoom 0?
		w0 = visibleWorld();
		
		// z1 is ds times bigger than this zoom:
		double ds = pow(2, z1 - map->getZoom());
		
		// so how much world at zoom z1?
		w1 = w0 / ds;

		std::cout << "w0 " << w0 << std::endl;
        std::cout << "w1 " << w1 << std::endl;

		///////////////////////////////////
				
		// simple interpolation of positions will be fine:
		u0 = 0;
        u1 = c0.distance(c1);

		std::cout << "u0 " << u0 << std::endl;
        std::cout << "u1 " << u1 << std::endl;

		r0 = r(b(0));
        r1 = r(b(1));
        S = (r1-r0) / rho; // "distance"

		std::cout << "r0 " << r0 << std::endl;
        std::cout << "r1 " << r1 << std::endl;
		
		// special case
		if (fabs(u0-u1) < 0.000001) {
			std::cout << fabs(u0-u1) << "< 0.000001" << std::endl;
			if (fabs(w0-w1) < 0.000001) {
				std::cout << fabs(w0-w1) << "< 0.000001" << std::endl;
				active = false;
				std::cout << "NOT TRANSITIONING" << std::endl;
				return;
			}
			k = w1 < w0 ? -1 : 1;
			S = fabs(log(w1/w0)) / rho;
		}

		std::cout << "S " << S << std::endl;
        std::cout << "k " << k << std::endl;
		
		active = true;
		t0 = _t0;
	}
	
	double visibleWorld() {
		// how much world can we see at zoom 0?
		Coordinate topLeft = map->pointCoordinate(Vec2d::zero()).zoomTo(0);
		Coordinate bottomRight = map->pointCoordinate(map->getSize()).zoomTo(0);
		return math<double>::max(bottomRight.column-topLeft.column, bottomRight.row-topLeft.row);
	}
	
	// t1 = current time, seconds
	void update(const double &t1) {
		
		if (!active) {
			return;
		}
		
		double t = (t1 - t0);
		double s = V * t;
		if (s > S) {
			s = S;
			active = false;
		}
		
		double us = u(s);
		Vec2d pos = lerp(c0,c1,(us-u0)/(u1-u0));
		
		double ws = w(s);

		Coordinate c(pos.y,pos.x,0);
		Location l = map->getMapProvider()->coordinateLocation(c);
		
		// how much world can we see at zoom 0?
		double w0 = visibleWorld();
		
		// so what's our new zoom?
		double z = map->getZoom() + (log(w0/ws) / log(2.0));
		map->setCenterZoom(l,z);
	}	
	
};
	
// i = 0 or 1
inline double Transitioner::b(const int &i) {
	double n = pow(w1,2.0) - pow(w0,2.0) + ((i ? -1 : 1) * pow(rho,4.0) * pow(u1-u0, 2.0));
	double d = 2.0 * (i ? w1 : w0) * pow(rho,2.0) * (u1-u0);
	return n / d;
}
	
// give this a b(0) or b(1)
inline double Transitioner::r(const double &b) {
	return log(-b + sqrt(pow(b,2.0)+1.0));
}	
	
inline double Transitioner::u(const double &s) {
	if (fabs(u0-u1) < 0.000001) {
		return u0;
	}	
	double a = w0/pow(rho,2.0);
	double b = a * cosh(r0) * tanh(rho*s + r0);
	double c = a * sinh(r0);
	return b - c + u0;
}
	
inline double Transitioner::w(const double &s) {
	if (fabs(u0-u1) < 0.000001) {
		return w0 * exp(k * rho * s);	
	}
	return w0 * cosh(r0) / cosh(rho*s + r0);
}
	
	

} } // namespace
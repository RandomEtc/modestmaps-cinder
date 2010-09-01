
#ifndef MERCATOR_PROJECTION
#define MERCATOR_PROJECTION

#include "AbstractProjection.h"
#include "Transformation.h"

class MercatorProjection : public AbstractProjection {
	
public:
	
	MercatorProjection(double _zoom=0): AbstractProjection(_zoom, Transformation()) { }
	MercatorProjection(double _zoom, Transformation t): AbstractProjection(_zoom,t) { }
	
	Vec2d rawProject(Vec2d point);
	Vec2d rawUnproject(Vec2d point);
	
};

#endif
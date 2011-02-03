#pragma once

namespace cinder { namespace modestmaps {
	
class QueueSorter {
public:
	
	const Coordinate center;
	
	QueueSorter(const Coordinate &c): center(c) {}
	
	inline static double dist(const double &x1, const double &y1, const double &x2, const double &y2) {
		double dx = x2-x1;
		double dy = y2-y1;
		return sqrt(dx*dx+dy*dy);
	}

	bool operator()(const Coordinate &c1, const Coordinate &c2) const {
		if (c1.zoom == center.zoom) {
			if (c2.zoom == center.zoom) {
				double d1 = dist(center.column, center.row, c1.column+0.5, c1.row+0.5);
				double d2 = dist(center.column, center.row, c2.column+0.5, c2.row+0.5);
				return d1 < d2;
			}
		}
		else if (c2.zoom == center.zoom) {
			return false;
		}
		else {
			double d1 = fabs(c1.zoom - center.zoom);
			double d2 = fabs(c2.zoom - center.zoom);
			return  d1 < d2;
		}
		return false;
	}
		
};

} } // namespace
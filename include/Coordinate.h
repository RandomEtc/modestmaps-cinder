#pragma once

#include <ostream>

namespace cinder { namespace modestmaps {
	
class Coordinate {
	
public:
	
	double row;	
	double column;
	double zoom;
	
	Coordinate() : row(0), column(0), zoom(0) { }
	
	Coordinate(double _row, double _column, double _zoom) : row(_row), column(_column), zoom(_zoom) {}
	
	Coordinate(const Coordinate &coord) : row(coord.row), column(coord.column), zoom(coord.zoom) {}
	
	friend std::ostream &operator<<(std::ostream &stream, const Coordinate &c) {
		// TODO: number format
		stream << "(" << c.row << ", " << c.column << " @" << c.zoom << ")";
		return stream;
	}
	
    bool operator == (const Coordinate &c) const
    {
        return c.row == row && c.column == column && c.zoom == zoom;
    }

	bool operator < (const Coordinate &c) const
    {
		//cout << *this << " compared to " << c << endl;
        return zoom < c.zoom || (zoom == c.zoom && row < c.row) || (zoom == c.zoom && row == c.row && column < c.column);
    }
	
	Coordinate container() const {
		return Coordinate(floor(row), floor(column), zoom);
	}
	
	Coordinate zoomTo(const double &destination) const {
		return zoomBy(destination - zoom);
	}
	
	Coordinate zoomBy(const double &distance) const {
		const double adjust = pow(2, distance);
		return Coordinate(row * adjust, column * adjust, zoom + distance);
	}
	
	Coordinate up(const double &distance=1) const {
		return Coordinate(row - distance, column, zoom);
	}
	
	Coordinate right(const double &distance=1) const {
		return Coordinate(row, column + distance, zoom);
	}
	
	Coordinate down(const double &distance=1) const {
		return Coordinate(row + distance, column, zoom);
	}
	
	Coordinate left(const double &distance=1) const {
		return Coordinate(row, column - distance, zoom);
	}
	
};

} } // namespace
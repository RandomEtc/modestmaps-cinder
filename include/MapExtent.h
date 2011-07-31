//
//  MapExtent.h
//  MultiTouch
//
//  Created by Tom Carden on 7/31/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#pragma once

#include <vector>
#include <cmath>
#include "Location.h"

namespace cinder { namespace modestmaps {

class MapExtent {
        
public:
    
    // TODO: OK for rectangular projections, but we need a better way for other projections
    double north;
    double south;
    double east;
    double west;
    
    /** @param n the most northerly latitude
     *  @param s the southern latitude
     *  @param e the eastern-most longitude
     *  @param w the westest longitude */
    explicit MapExtent(const double &n=0, const double &s=0, const double &e=0, const double &w=0): north(n), south(s), east(e), west(w) { canonicalize(); }

    explicit MapExtent(const Location &l1, const Location &l2): north(l1.lat), south(l2.lat), east(l1.lon), west(l2.lon) { canonicalize(); }

    MapExtent( const MapExtent &extent ): north(extent.north), south(extent.south), east(extent.east), west(extent.west) { canonicalize(); }
    
    /** calculate the north/south/east/west extremes of the given array of locations */
    explicit MapExtent(const std::vector<Location> &locations)
    {
        north = locations[0].lat;
        south = locations[0].lat;
        east = locations[0].lon;
        west = locations[0].lon;
        for (int i = 1; i < locations.size(); i++) {
            enclose(locations[i]);
        }
    }    

    void canonicalize()
    {
        const double n = north;
        const double s = south;
        const double e = east;
        const double w = west;
        north = std::max(n, s);
        south = std::min(n, s);
        east = std::max(e, w);
        west = std::min(e, w);
    }
    
    MapExtent clone() const
    {
        return MapExtent(north, south, east, west);
    }
    
    /** enlarges this extent so that the given extent is inside it */
    void encloseExtent(const MapExtent &extent)
    {
        north = std::max(extent.north, north);
        south = std::min(extent.south, south);
        east = std::max(extent.east, east);
        west = std::min(extent.west, west);		    
    }
    
    /** enlarges this extent so that the given location is inside it */
    void enclose(const Location &location)
    {
        north = std::max(location.lat, north);
        south = std::min(location.lat, south);
        east = std::max(location.lon, east);
        west = std::min(location.lon, west);
    }
    
    Location getNorthWest() const
    {
        return Location(north, west);
    }
    
    Location getSouthWest() const
    {
        return Location(south, west);
    }
    
    Location getNorthEast() const
    {
        return Location(north, east);
    }
    
    Location getSouthEast() const
    {
        return Location(south, east);
    }
    
    void setNorthWest(const Location &nw)
    {
        north = nw.lat;
        west = nw.lon;
    }
    
    void setSouthWest(const Location &sw)
    {
        south = sw.lat;
        west = sw.lon;
    }
    
    void setNorthEast(const Location &ne)
    {
        north = ne.lat;
        east = ne.lon;
    }
    
    void setSouthEast(const Location &se)
    {
        south = se.lat;
        east = se.lon;
    }
    
    Location getCenter() const
    {   
        // TODO: check this function        
        return Location(south + (north - south) / 2.0, west + (east - west) / 2.0);
    }
    
    void setCenter(const Location &location)
    {   
        // TODO: check this function        
        double w = east - west;
        double h = north - south;
        north = location.lat - h / 2.0;
        south = location.lat + h / 2.0;
        east = location.lon + w / 2.0;
        west = location.lon - w / 2.0;
    }
    
    void inflate(const double &lat, const double &lon)
    {
        // TODO: check this function        
        north += lat;
        south -= lat;
        west -= lon;
        east += lon;
    }
    
//    public function getRect():Rectangle
//    {
//        var rect:Rectangle = new Rectangle(Math.min(east, west), Math.min(north, south));
//        rect.right = Math.max(east, west);
//        rect.bottom = Math.max(north, south);
//        return rect;
//    }
    
    bool contains(const Location &location) const
    {
        // TODO: check this function
        return !(location.lon > east || location.lon < west || location.lat > north || location.lat < south);
    }
    
    bool contains(const MapExtent &extent) const
    {
        return contains(extent.getNorthWest()) && contains(extent.getSouthEast());
    }
    
    /** @return "north, south, east, west" */
	friend std::ostream &operator<<(std::ostream &stream, const MapExtent &e) {
		// TODO: number format
		stream << "(" << e.north << ", " 
                      << e.south << ", " 
                      << e.east << ", " 
                      << e.west << ")";
		return stream;
	}
    
};
    
} }

#include "Map.h"

namespace cinder { namespace modestmaps {

void Map::setup(AbstractMapProvider* _provider, Vec2d _size) {
	provider = _provider;
    size = _size;
	centerCoordinate = Coordinate(0.5,0.5,0);  // half the world width,height at zoom 0
	// fit to screen
	double z = log(std::min(size.x,size.y) / 256.0) / log(2);
	centerCoordinate = centerCoordinate.zoomTo(z);
	// start with north up:
	rotation = 0.0;
}
	
void Map::update() {
	// TODO: Move non-drawing logic here
}

void Map::draw() {
	
	// if we're in between zoom levels, we need to choose the nearest:
	int baseZoom = constrain((int)round(centerCoordinate.zoom), provider->minZoom(), provider->maxZoom());

	// these are the top left and bottom right tile coordinates
	// we'll be loading everything in between:
	Coordinate tl = pointCoordinate(Vec2d::zero()).zoomTo(baseZoom);
	Coordinate tr = pointCoordinate(Vec2d(size.x,0)).zoomTo(baseZoom);
	Coordinate bl = pointCoordinate(Vec2d(0,size.y)).zoomTo(baseZoom);
	Coordinate br = pointCoordinate(size).zoomTo(baseZoom);
	
	// find start and end columns
	int minCol = floor(std::min(std::min(tl.column,tr.column),std::min(bl.column,br.column)));
	int maxCol = floor(std::max(std::max(tl.column,tr.column),std::max(bl.column,br.column)));
	int minRow = floor(std::min(std::min(tl.row,tr.row),std::min(bl.row,br.row)));
	int maxRow = floor(std::max(std::max(tl.row,tr.row),std::max(bl.row,br.row)));
	
	// pad a bit, for luck (well, because we might be zooming out between zoom levels)
	minCol -= GRID_PADDING;
	minRow -= GRID_PADDING;
	maxCol += GRID_PADDING;
	maxRow += GRID_PADDING;
	
	visibleKeys.clear();
	
	// grab coords for visible tiles
	for (int col = minCol; col <= maxCol; col++) {
		for (int row = minRow; row <= maxRow; row++) {
			
			// source coordinate wraps around the world:
			//Coordinate coord = provider->sourceCoordinate(Coordinate(row,col,baseZoom));
			Coordinate coord = Coordinate(row,col,baseZoom);
			
			// keep this for later:
			visibleKeys.insert(coord);
			
			if (images.count(coord) == 0) { // || (ofGetElapsedTimeMillis() - images[coord]->loadTime < 255)) {
				
				// fetch it if we don't have it
				grabTile(coord);
				
				// see if we have  a parent coord for this tile?
				bool gotParent = false;
				for (int i = (int)coord.zoom; i > 0; i--) {
					Coordinate zoomed = coord.zoomTo(i).container();
//					if (images.count(zoomed) > 0) {
//						visibleKeys.insert(zoomed);
//						gotParent = true;
//						break;
//					}
					// mark all parent tiles valid
					visibleKeys.insert(zoomed);
					gotParent = true;
					if (images.count(zoomed) == 0) {
						// force load of parent tiles we don't already have
						grabTile(zoomed);
					}					
				}
				
				// or if we have any of the children
//				if (!gotParent) {
//					Coordinate zoomed = coord.zoomBy(1).container();
//					std::vector<Coordinate> kids;
//					kids.push_back(zoomed);
//					kids.push_back(zoomed.right());
//					kids.push_back(zoomed.down());
//					kids.push_back(zoomed.right().down());
//					for (int i = 0; i < kids.size(); i++) {
//						if (images.count(kids[i]) > 0) {
//							visibleKeys.insert(kids[i]);
//						}
//					}            
//				}
				
			}
			
		} // rows
	} // columns
	
	// TODO: sort by zoom so we draw small zoom levels (big tiles) first:
	// can this be done with a different comparison function on the visibleKeys set?
	//Collections.sort(visibleKeys, zoomComparator);

    glPushMatrix();
    glRotatef(180.0*rotation/M_PI, 0, 0, 1);

	int numDrawnImages = 0;	
	std::set<Coordinate>::iterator citer;
	for (citer = visibleKeys.begin(); citer != visibleKeys.end(); citer++) {
		Coordinate coord = *citer;
		
		double scale = pow(2.0, centerCoordinate.zoom - coord.zoom);
		double tileWidth = provider->tileWidth() * scale;
		double tileHeight = provider->tileHeight() * scale;
		Vec2d center = size * 0.5;
		Coordinate theCoord = centerCoordinate.zoomTo(coord.zoom);
		
		double tx = center.x + (coord.column - theCoord.column) * tileWidth;
		double ty = center.y + (coord.row - theCoord.row) * tileHeight;
		
		if (images.count(coord) > 0) {
			gl::Texture tile = images[coord];
			// we want this image to be at the end of recentImages, if it's already there we'll remove it and then add it again
			recentImages.erase(remove(recentImages.begin(), recentImages.end(), tile), recentImages.end());
			gl::draw( tile, Rectf(tx, ty, tx+tileWidth, ty+tileHeight) );
			numDrawnImages++;
			recentImages.push_back(tile);
		}
	}

    glPopMatrix();
	
	// stop fetching things we can't see:
	// (visibleKeys also has the parents and children, if needed, but that shouldn't matter)
	//queue.retainAll(visibleKeys);
	std::vector<Coordinate>::iterator iter = queue.begin();
	while (iter != queue.end()) {
		Coordinate key = *iter;
		if (visibleKeys.count(key) == 0){ 
			iter = queue.erase(iter);
		}
		else {
			++iter;
		}
	}
	
	// TODO sort what's left by distance from center:
	//queueSorter.setCenter(new Coordinate( (minRow + maxRow) / 2.0f, (minCol + maxCol) / 2.0f, zoom));
	//Collections.sort(queue, queueSorter);
	
	// load up to 4 more things:
	processQueue();
	
	// clear some images away if we have too many...
	int numToKeep = std::max(numDrawnImages,MAX_IMAGES_TO_KEEP);
	if (recentImages.size() > numToKeep) {
		// first clear the pointers from recentImages
		recentImages.erase(recentImages.begin(), recentImages.end()-numToKeep);
		//images.values().retainAll(recentImages);
		// TODO: re-think the stl collections used so that a simpler retainAll equivalent is available
		// now look in the images map and if the value is no longer in recent images then get rid of it
		std::map<Coordinate,gl::Texture>::iterator iter = images.begin();
		std::map<Coordinate,gl::Texture>::iterator endIter = images.end();
		for (; iter != endIter;) {
			gl::Texture tile = iter->second;
			std::vector<gl::Texture>::iterator result = find(recentImages.begin(), recentImages.end(), tile);
			if (result == recentImages.end()) {
				images.erase(iter++);
			}
			else {
				++iter;
			}
		}
	}
	
}

void Map::panBy(const Vec2d &delta) { panBy(delta.x, delta.y); }
	
void Map::panBy(const double &dx, const double &dy) {
	double dxr = dx*cos(rotation) + dy*sin(rotation);
	double dyr = dy*cos(rotation) - dx*sin(rotation);
	centerCoordinate.column -= dxr / provider->tileWidth();
	centerCoordinate.row -= dyr / provider->tileHeight();
}
void Map::scaleBy(const double &s) {
	scaleBy(s, size * 0.5);
}
void Map::scaleBy(const double &s, const Vec2d &c) {
	scaleBy(s, c.x, c.y);
}
void Map::scaleBy(const double &s, const double &cx, const double &cy) {
	double r = rotation;
	rotateBy(-r,cx,cy);
    Vec2d center = size * 0.5;
	panBy(-cx+center.x, -cy+center.y);
	centerCoordinate = centerCoordinate.zoomBy(log(s) / log(2.0));
	panBy(cx-center.x, cy-center.y);
	rotateBy(r,cx,cy);
}
void Map::rotateBy(const double &r, const double &cx, const double &cy) {
	panBy(-cx, -cy);
	rotation += r;
	panBy(cx, cy);
}

//////////////////

double Map::getZoom() {
	return centerCoordinate.zoom;
}

Location Map::getCenter() {
	return provider->coordinateLocation(centerCoordinate);
}

Coordinate Map::getCenterCoordinate() {
	return Coordinate(centerCoordinate); // TODO: return const? 
}

void Map::setCenter(const Coordinate &center) {
	centerCoordinate = center;
}

void Map::setCenter(const Location &location) {
	setCenter(provider->locationCoordinate(location).zoomTo(getZoom()));
}

void Map::setCenterZoom(const Location &location, const double &zoom) {
	setCenter(provider->locationCoordinate(location).zoomTo(zoom));
}

void Map::setZoom(const double &zoom) {
	centerCoordinate = centerCoordinate.zoomTo(zoom);
}

void Map::zoomBy(const double &dir) {
	centerCoordinate = centerCoordinate.zoomBy(dir);	
}

void Map::zoomIn()  { zoomBy(1);  }
void Map::zoomOut() { zoomBy(-1); }

// TODO: extent functions
//	    public function setExtent(extent:MapExtent):void
//	    public function getExtent():MapExtent

// TODO: make it so you can safely get and set the provider
/*AbstractMapProvider getMapProvider() {
 return provider;
 }
 
 void setMapProvider(AbstractMapProvider _provider) {
 provider = _provider;
 images.clear();
 queue.clear();
 pending.clear();
 }*/

Vec2d Map::coordinatePoint(const Coordinate &target)
{
	/* Return an x, y point on the map image for a given coordinate. */
	
	Coordinate coord = target;
	
	if(coord.zoom != centerCoordinate.zoom) {
		coord = coord.zoomTo(centerCoordinate.zoom);
	}
	
	// distance from the center of the map
	Vec2d point = size * 0.5;
	point.x += TILE_SIZE * (coord.column - centerCoordinate.column);
	point.y += TILE_SIZE * (coord.row - centerCoordinate.row);

	Vec2d rotated(point);
	rotated.rotate(rotation);
	
	return rotated;
}

Coordinate Map::pointCoordinate(const Vec2d &point) {
	/* Return a coordinate on the map image for a given x, y point. */		
	// new point coordinate reflecting distance from map center, in tile widths
	Vec2d rotated(point);
	rotated.rotate(-rotation);
	Coordinate coord(centerCoordinate);
	coord.column += (rotated.x - size.x * 0.5) / TILE_SIZE;
	coord.row += (rotated.y - size.y * 0.5) / TILE_SIZE;
	return coord;
}

Vec2d Map::locationPoint(const Location &location) {
	return coordinatePoint(provider->locationCoordinate(location));
}

Location Map::pointLocation(const Vec2d &point) {
	return provider->coordinateLocation(pointCoordinate(point));
}

void Map::panUp()    { panBy(0,size.y/8.0);  }
void Map::panDown()  { panBy(0,-size.y/8.0); }
void Map::panLeft()  { panBy(size.x/8.0,0);   }
void Map::panRight() { panBy(-size.x/8.0,0);  }

void Map::panAndZoomIn(const Location &location) {
	setCenterZoom(location, getZoom() + 1);
}

void Map::panTo(const Location &location) {
	setCenter(location);
}

//////////////////////////////////////////////////////////////////////////

void Map::grabTile(const Coordinate &coord) {
	bool isPending = tileLoader.isPending(coord);
	bool isQueued = find(queue.begin(), queue.end(), coord) != queue.end();
	bool isAlreadyLoaded = images.count(coord) > 0;
	if (!isPending && !isQueued && !isAlreadyLoaded) {
		queue.push_back(Coordinate(coord));
	}
}

void Map::processQueue() {	
	sort(queue.begin(), queue.end(), QueueSorter(getCenterCoordinate().zoomTo(getZoom())));		
	tileLoader.processQueue(queue, provider);
	tileLoader.transferTextures(images);
}

void Map::setSize(Vec2d _size) {
    size = _size;
}
	
Vec2d Map::getSize() {
	return size;
}
	
} } // namespace
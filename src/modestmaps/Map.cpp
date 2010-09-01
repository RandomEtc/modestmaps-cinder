
#include "Map.h"

void Map::setup(AbstractMapProvider* _provider, double _width, double _height) {
	provider = _provider;
	width = _width;
	height = _height;
	centerCoordinate = Coordinate(0.5,0.5,0);  // half the world width,height at zoom 0
	// fit to screen
	double z = log(std::min(width,height) / 256.0) / log(2);
	centerCoordinate = centerCoordinate.zoomTo(z);
	// start with north up:
	rotation = 0.0;
}
	
void Map::update() {
	// TODO: Move non-drawing logic here
}

void Map::draw() {
	
	// if we're in between zoom levels, we need to choose the nearest:
	int baseZoom = round(centerCoordinate.zoom);
	
	// these are the top left and bottom right tile coordinates
	// we'll be loading everything in between:
	Coordinate startCoord = pointCoordinate(Vec2d(0,0)).zoomTo(baseZoom).container();
	Coordinate endCoord = pointCoordinate(Vec2d(width,height)).zoomTo(baseZoom).container().right().down();
	
	// find start and end columns
	int minCol = startCoord.column;
	int maxCol = endCoord.column;
	int minRow = startCoord.row;
	int maxRow = endCoord.row;
	
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
					if (images.count(zoomed) > 0) {
						visibleKeys.insert(zoomed);
						gotParent = true;
						break;
					}
				}
				
				// or if we have any of the children
				if (!gotParent) {
					Coordinate zoomed = coord.zoomBy(1).container();
					std::vector<Coordinate> kids;
					kids.push_back(zoomed);
					kids.push_back(zoomed.right());
					kids.push_back(zoomed.down());
					kids.push_back(zoomed.right().down());
					for (int i = 0; i < kids.size(); i++) {
						if (images.count(kids[i]) > 0) {
							visibleKeys.insert(kids[i]);
						}
					}            
				}
				
			}
			
		} // rows
	} // columns
	
	// TODO: sort by zoom so we draw small zoom levels (big tiles) first:
	// can this be done with a different comparison function on the visibleKeys set?
	//Collections.sort(visibleKeys, zoomComparator);
	
	int numDrawnImages = 0;	
	std::set<Coordinate>::iterator citer;
	for (citer = visibleKeys.begin(); citer != visibleKeys.end(); citer++) {
		Coordinate coord = *citer;
		
		double scale = pow(2.0, centerCoordinate.zoom - coord.zoom);
		double tileWidth = provider->tileWidth() * scale;
		double tileHeight = provider->tileHeight() * scale;
		Vec2d center(width/2.0, height/2.0);
		Coordinate theCoord = centerCoordinate.zoomTo(coord.zoom);
		
		double tx = center.x + (coord.column - theCoord.column) * tileWidth;
		double ty = center.y + (coord.row - theCoord.row) * tileHeight;
		
		if (images.count(coord) > 0) {
			gl::Texture tile = images[coord];
			// we want this image to be at the end of recentImages, if it's already there we'll remove it and then add it again
			recentImages.erase(remove(recentImages.begin(), recentImages.end(), tile), recentImages.end());
			glPushMatrix();
			glRotated(180.0*rotation/M_PI, 0, 0, 1);
			gl::draw( tile, Rectf(tx, ty, tx+tileWidth, ty+tileHeight) );
			glPopMatrix();
			numDrawnImages++;
			recentImages.push_back(tile);
		}
	}
	
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
				tile.unbind();
			}
			else {
				++iter;
			}
		}
	}
	
}

void Map::panBy(double dx, double dy) {
	double dxr = dx*cos(rotation) + dy*sin(rotation);
	double dyr = dy*cos(rotation) - dx*sin(rotation);
	centerCoordinate.column -= dxr / provider->tileWidth();
	centerCoordinate.row -= dyr / provider->tileHeight();
}
void Map::scaleBy(double s) {
	scaleBy(s, width/2.0, height/2.0);
}
void Map::scaleBy(double s, double cx, double cy) {
	double r = rotation;
	rotateBy(-r,cx,cy);
	panBy(-cx+width/2.0, -cy+height/2.0);
	centerCoordinate = centerCoordinate.zoomBy(log(s) / log(2.0));
	panBy(cx-width/2.0, cy-height/2.0);
	rotateBy(r,cx,cy);
}
void Map::rotateBy(double r, double cx, double cy) {
	panBy(-cx, -cy);
	rotation += r;
	panBy(cx, cy);
}

//////////////////

/** @return zoom level of currently visible tile layer */
int Map::getZoom() {
	return round(centerCoordinate.zoom);
}

Location Map::getCenter() {
	return provider->coordinateLocation(centerCoordinate);
}

Coordinate Map::getCenterCoordinate() {
	return Coordinate(centerCoordinate); // TODO: return const? 
}

void Map::setCenter(Coordinate center) {
	centerCoordinate = center;
}

void Map::setCenter(Location location) {
	setCenter(provider->locationCoordinate(location).zoomTo(getZoom()));
}

void Map::setCenterZoom(Location location, int zoom) {
	setCenter(provider->locationCoordinate(location).zoomTo(zoom));
}

void Map::setZoom(double zoom) {
	centerCoordinate = centerCoordinate.zoomTo(zoom);
}

void Map::zoomBy(double dir) {
	centerCoordinate = centerCoordinate.zoomBy(dir);	
}

void Map::zoomIn() {
	centerCoordinate = centerCoordinate.zoomBy(1);
}  

void Map::zoomOut() {
	centerCoordinate = centerCoordinate.zoomBy(-1);
}

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

Vec2d Map::coordinatePoint(Coordinate coord)
{
	/* Return an x, y point on the map image for a given coordinate. */
	
	if(coord.zoom != centerCoordinate.zoom) {
		coord = coord.zoomTo(centerCoordinate.zoom);
	}
	
	// distance from the center of the map
	Vec2d point(width/2.0, height/2.0);
	point.x += TILE_SIZE * (coord.column - centerCoordinate.column);
	point.y += TILE_SIZE * (coord.row - centerCoordinate.row);

	Vec2d rotated(point);
	rotated.rotate(rotation);
	
	return rotated;
}

Coordinate Map::pointCoordinate(Vec2d point) {
	/* Return a coordinate on the map image for a given x, y point. */		
	// new point coordinate reflecting distance from map center, in tile widths
	Vec2d rotated(point);
	rotated.rotate(-rotation);
	Coordinate coord(centerCoordinate);
	coord.column += (rotated.x - width/2) / TILE_SIZE;
	coord.row += (rotated.y - height/2) / TILE_SIZE;
	return coord;
}

Vec2d Map::locationPoint(Location location) {
	return coordinatePoint(provider->locationCoordinate(location));
}

Location Map::pointLocation(Vec2d point) {
	return provider->coordinateLocation(pointCoordinate(point));
}

// TODO: pan by proportion of screen size, not by coordinate grid
void Map::panUp() {
	panBy(0,height/8.0);
}
void Map::panDown() {
	panBy(0,-height/8.0);
}
void Map::panLeft() {
	panBy(width/8.0,0);
}
void Map::panRight() {
	panBy(-width/8.0,0);
}

void Map::panAndZoomIn(Location location) {
	// TODO: animate?
	setCenterZoom(location, getZoom() + 1);
}

void Map::panTo(Location location) {
	// TODO: animate?
	setCenter(location);
}

//////////////////////////////////////////////////////////////////////////

void Map::grabTile(Coordinate coord) {
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


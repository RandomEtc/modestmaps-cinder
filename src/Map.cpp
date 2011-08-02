
#include "Map.h"

namespace cinder { namespace modestmaps {         
    
void Map::setup( MapProviderRef _mapProvider, Vec2d _size )
{
    tileLoader = TileLoader::create( _mapProvider );
    mapProvider = _mapProvider;
    size = _size;
    centerCoordinate = Coordinate(0.5,0.5,0);  // half the world width,height at zoom 0
    // fit to screen
    double z = log(std::min(size.x,size.y) / 256.0) / log(2); // FIXME: use provider's getTileSize
    centerCoordinate = centerCoordinate.zoomTo(z);
    // start with north up:
    rotation = 0.0;
}     
	
void Map::update() {
	// TODO: Move non-drawing logic here
    std::cout << "images:       " << images.size() << std::endl;
    std::cout << "queue:        " << queue.size() << std::endl;
    std::cout << "recentImages: " << recentImages.size() << std::endl;
    std::cout << "visibleKeys:  " << visibleKeys.size() << std::endl;
    std::cout << "-------------------" << std::endl << std::endl;
}

void Map::draw() {
	
	// if we're in between zoom levels, we need to choose the nearest:
	int baseZoom = constrain((int)round(centerCoordinate.zoom), mapProvider->getMinZoom(), mapProvider->getMaxZoom());

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
			
			Coordinate coord(row,col,baseZoom);
			
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
        Vec2d tileSize = mapProvider->getTileSize() * scale;
		Vec2d center = size * 0.5;
		Coordinate theCoord = centerCoordinate.zoomTo(coord.zoom);
		
		double tx = center.x + (coord.column - theCoord.column) * tileSize.x;
		double ty = center.y + (coord.row - theCoord.row) * tileSize.y;
		
		if (images.count(coord) > 0) {
			gl::Texture tile = images[coord];
			// we want this image to be at the end of recentImages, if it's already there we'll remove it and then add it again
//			recentImages.erase(remove(recentImages.begin(), recentImages.end(), tile), recentImages.end());
            std::vector<Coordinate>::iterator result = find(recentImages.begin(), recentImages.end(), coord);
            if (result != recentImages.end()) {
                recentImages.erase(result);
            }
			gl::draw( tile, Rectf(tx, ty, tx+tileSize.x, ty+tileSize.y) );
			numDrawnImages++;
			recentImages.push_back(coord);
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
            Coordinate coord = iter->first;
//			gl::Texture tile = iter->second;
			std::vector<Coordinate>::iterator result = find(recentImages.begin(), recentImages.end(), coord);
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
    const double sinr = sin(rotation);
    const double cosr = cos(rotation);
	const double dxr = dx*cosr + dy*sinr;
	const double dyr = dy*cosr - dx*sinr;
    const Vec2d tileSize = mapProvider->getTileSize();
	centerCoordinate.column -= dxr / tileSize.x;
	centerCoordinate.row -= dyr / tileSize.y;
}
void Map::scaleBy(const double &s) {
	scaleBy(s, size * 0.5);
}
void Map::scaleBy(const double &s, const Vec2d &c) {
	scaleBy(s, c.x, c.y);
}
void Map::scaleBy(const double &s, const double &cx, const double &cy) {
    const double prevRotation = rotation;
	rotateBy(-prevRotation,cx,cy);
    Vec2d center = size * 0.5;
	panBy(-cx+center.x, -cy+center.y);
	centerCoordinate = centerCoordinate.zoomBy(log(s) / log(2.0));
	panBy(cx-center.x, cy-center.y);
	rotateBy(prevRotation,cx,cy);
}
void Map::rotateBy(const double &r, const double &cx, const double &cy) {
	panBy(-cx, -cy);
	rotation += r;
	panBy(cx, cy);
}

//////////////////

double Map::getZoom() const {
	return centerCoordinate.zoom;
}

Location Map::getCenter() const {
	return mapProvider->coordinateLocation(centerCoordinate);
}

Coordinate Map::getCenterCoordinate() const {
	return centerCoordinate;
}

void Map::setCenter(const Coordinate &center) {
	centerCoordinate = center;
}

void Map::setCenter(const Location &location) {
	setCenter(mapProvider->locationCoordinate(location).zoomTo(getZoom()));
}

void Map::setCenterZoom(const Location &location, const double &zoom) {
	setCenter(mapProvider->locationCoordinate(location).zoomTo(zoom));
}

void Map::setZoom(const double &zoom) {
	centerCoordinate = centerCoordinate.zoomTo(zoom);
}

void Map::zoomBy(const double &dir) {
	centerCoordinate = centerCoordinate.zoomBy(dir);	
}

void Map::zoomIn()  { zoomBy(1);  }
void Map::zoomOut() { zoomBy(-1); }

void Map::setExtent( const MapExtent &extent, bool forceIntZoom )
{
    Coordinate TL = mapProvider->locationCoordinate( extent.getNorthWest() ).zoomTo( getZoom() );
    Coordinate BR = mapProvider->locationCoordinate( extent.getSouthEast() ).zoomTo( getZoom() );
    
    const Vec2d tileSize = mapProvider->getTileSize();
    
    // multiplication factor between horizontal span and map width
    const double hFactor = (BR.column - TL.column) / (size.x / tileSize.x);
    
    // multiplication factor expressed as base-2 logarithm, for zoom difference
    const double hZoomDiff = log(hFactor) / log(2);
    
    // possible horizontal zoom to fit geographical extent in map width
    const double hPossibleZoom = TL.zoom - (forceIntZoom ? ceil(hZoomDiff) : hZoomDiff);
    
    // multiplication factor between vertical span and map height
    const double vFactor = (BR.row - TL.row) / (size.y / tileSize.y);
    
    // multiplication factor expressed as base-2 logarithm, for zoom difference
    const double vZoomDiff = log(vFactor) / log(2);
    
    // possible vertical zoom to fit geographical extent in map height
    const double vPossibleZoom = TL.zoom - (forceIntZoom ? ceil(vZoomDiff) : vZoomDiff);
    
    // initial zoom to fit extent vertically and horizontally
    double initZoom = std::min(hPossibleZoom, vPossibleZoom);
    
    // additionally, make sure it's not outside the boundaries set by provider limits
    initZoom = std::min(initZoom, (double)mapProvider->getMaxZoom());
    initZoom = std::max(initZoom, (double)mapProvider->getMinZoom());
    
    // coordinate of extent center
    const double centerRow = (TL.row + BR.row) / 2.0;
    const double centerColumn = (TL.column + BR.column) / 2.0;
    const double centerZoom = (TL.zoom + BR.zoom) / 2.0;
    setCenter( Coordinate(centerRow, centerColumn, centerZoom).zoomTo(initZoom) );    
}
    
MapExtent Map::getExtent() const
{
    return MapExtent( pointLocation( Vec2d::zero() ), pointLocation( size ) );
}

void Map::setMapProvider( MapProviderRef _mapProvider )
{
    tileLoader->setMapProvider( _mapProvider );
    images.clear();	
    queue.clear();
    recentImages.clear();
    visibleKeys.clear();
    mapProvider = _mapProvider;
}

Vec2d Map::coordinatePoint(const Coordinate &target) const
{
	/* Return an x, y point on the map image for a given coordinate. */
	
	Coordinate coord = target;
	
	if(coord.zoom != centerCoordinate.zoom) {
		coord = coord.zoomTo(centerCoordinate.zoom);
	}

	// distance from the center of the map
    const Vec2d tileSize = mapProvider->getTileSize();
	Vec2d point = size * 0.5;
	point.x += tileSize.x * (coord.column - centerCoordinate.column);
	point.y += tileSize.y * (coord.row - centerCoordinate.row);

	Vec2d rotated(point);
	rotated.rotate(rotation);
	
	return rotated;
}

Coordinate Map::pointCoordinate(const Vec2d &point) const {
	/* Return a coordinate on the map image for a given x, y point. */		
	// new point coordinate reflecting distance from map center, in tile widths
	Vec2d rotated(point);
    const Vec2d tileSize = mapProvider->getTileSize();    
	rotated.rotate(-rotation);
	Coordinate coord(centerCoordinate);
	coord.column += (rotated.x - size.x * 0.5) / tileSize.x;
	coord.row += (rotated.y - size.y * 0.5) / tileSize.y;
	return coord;
}

Vec2d Map::locationPoint(const Location &location) const {
	return coordinatePoint(mapProvider->locationCoordinate(location));
}

Location Map::pointLocation(const Vec2d &point) const {
	return mapProvider->coordinateLocation(pointCoordinate(point));
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
	bool isPending = tileLoader->isPending(coord);
	bool isQueued = find(queue.begin(), queue.end(), coord) != queue.end();
	bool isAlreadyLoaded = images.count(coord) > 0;
	if (!isPending && !isQueued && !isAlreadyLoaded) {
		queue.push_back(coord);
	}
}

void Map::processQueue() {	
	sort(queue.begin(), queue.end(), QueueSorter(getCenterCoordinate().zoomTo(getZoom())));		
	tileLoader->processQueue(queue);
	tileLoader->transferTextures(images);
}

void Map::setSize(Vec2d _size) {
    size = _size;
}
	
Vec2d Map::getSize() const {
	return size;
}
	
} } // namespace
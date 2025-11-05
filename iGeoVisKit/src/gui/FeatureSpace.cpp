#include "PchApp.h"

#include "MainWindow.h"
#include "imagery/ImageHandler.h"
extern MainWindow* mainWindow;
// #include "Window.h"  // 绉婚櫎瀵筗indow.h鐨勪緷璧?
#include "FeatureSpace.h"
#include "opengl/Renderer.h"
#include "GLContainer.h"
#include "config.h"
#include "utils/Console.h"

#define POLYDATA_DEBUG 0
#define ENABLE_ROI_POINTS 1

int FeatureSpace::numFeatureSpaces=0;

const int FeatureSpace::FEATURE_WINDOW_WIDTH=650;       // inital width of feature space window
const int FeatureSpace::FEATURE_WINDOW_HEIGHT=550;      // inital height of feature space window
const int FeatureSpace::TOOLBAR_HEIGHT=30;              // height of toolbar area at bottom of window

extern ROISet *regionsSet;

// create & display new feature space window
// FeatureSpace::FeatureSpace(int LOD, bool only_ROIs, int b1, int b2, int b3) {
FeatureSpace::FeatureSpace(int theLOD, int band1, int band2, int band3, bool onlyROIs_arg)
    // : QWidget(nullptr)  // Qt鏂瑰紡
{
    // 鍒濆鍖栨垚鍛樺彉閲?
    this->theLOD = theLOD;
    this->band1 = band1;
    this->band2 = band2;
    this->band3 = band3;
    this->onlyROIs = onlyROIs_arg;
    
    fsTileset = NULL;
    fsgl = NULL;
    glContainer = NULL;
    numberPoints = 0;
    
    // 浣跨敤Qt鏂瑰紡鍒涘缓GLContainer
    // glContainer = new GLContainer(this, this, 0, 0, 400, 400);
    // glContainer->setupUI();
    
    numFeatureSpaces++;
    
    Console::write("FeatureSpace -- seting up opengl stuff...\n");
    // fsgl = new FeatureSpaceGL(glContainer->GetHandle(), theLOD, band1, band2, band3);  // 鏇挎崲涓篞t鏂瑰紡
    // fsgl = new FeatureSpaceGL(glContainer, theLOD, band1, band2, band3);  // Qt鏂瑰紡
    
    Console::write("FeatureSpace -- getting pixel data...\n");
    getPixelData();

    Console::write("FeatureSpace -- calling OnResize\n");    
    OnResize();
    
    // Show();  // Qt鏂瑰紡浣跨敤show()
    // if (glContainer) {
    //     glContainer->show();
    // }
}

// setup feature space
void FeatureSpace::init()
{
    fsgl=NULL;
    Create();  // 鐩存帴璋冪敤锛屼笉鍐嶉渶瑕佹鏌ヨ繑鍥炲€?
       
    numFeatureSpaces++;
    
    Console::write("FeatureSpace -- seting up opengl stuff...\n");
    // 构造与 Qt 解耦的 FeatureSpaceGL
    fsgl = new FeatureSpaceGL(
        theLOD,
        band1,
        band2,
        band3
    );

    // 在 GLContainer 内部创建 GLView 并接线 Renderer
    if (glContainer && m_glView == nullptr) {
        // 为容器建立零边距布局
        QVBoxLayout *lyt = new QVBoxLayout(glContainer);
        lyt->setContentsMargins(0,0,0,0);
        lyt->setSpacing(0);
        m_glView = new GLView(glContainer);
        m_glView->setMinimumSize(4,4);
        m_glView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        lyt->addWidget(m_glView);

        // 接线渲染器与渲染回调
        if (mainWindow && mainWindow->getRenderer()) {
            m_glView->setRendererInstance(mainWindow->getRenderer());
            m_glView->addRenderCallback([this](Renderer &r){
                if (this->fsgl) {
                    r.renderFeatureSpace(this->fsgl);
                }
            });
        }

        // 输入事件接线（沿用 GLContainer 处理逻辑）
        QObject::connect(m_glView, &GLView::mousePressed, [this](int x, int y){
            this->OnGLContainerLeftMouseDown(x, y);
        });
        QObject::connect(m_glView, &GLView::mouseMoved, [this](int buttons, int x, int y){
            this->OnGLContainerMouseMove(buttons, x, y);
        });
        QObject::connect(m_glView, &GLView::resized, [this](int w, int h){
            Q_UNUSED(w); Q_UNUSED(h);
            // 首帧有效绘制：触发一次更新
            if (m_glView) m_glView->update();
        });
    }

    Console::write("FeatureSpace -- getting pixel data...\n");
    getPixelData();

	Console::write("FeatureSpace -- calling OnResize\n");    
    OnResize();
	
    show();  // Qt鏂瑰紡鏄剧ず绐楀彛
}

// draw contents of GLContainer with opengl
void FeatureSpace::PaintGLContainer() {
	fsgl->draw();
}

//getPixelData gets all point coordinates from within ROIs, and
//then gets all data values for those pixel coords, and stores
//them, ready for display list generation.
void FeatureSpace::getPixelData(void)
{
	vector<ROI*> theROIs = regionsSet->get_regions();
	Console::write("FS::GpixD\tCount of regions is %d\n", regionsSet->get_regions_count());
	assert(regionsSet != NULL);
	Console::write("FS::GpixD\tGot vector of ROIs\n");
	
    ROI* currentROI;
	
fsTileset = new ImageTileSet(theLOD, mainWindow && mainWindow->imageHandler() ? mainWindow->imageHandler()->get_image_file() : nullptr, tileSize, 128);
	LODfactor = fsTileset->get_LOD_factor();
	LODwidth = fsTileset->get_LOD_width();
	LODheight = fsTileset->get_LOD_height();
	
	if (!onlyROIs) getImageData(); // Populate fsAllPoints, as
	
	Console::write("FS::GpixD\tLOD factor is %d\n", LODfactor);

	#if ENABLE_ROI_POINTS
	if (!theROIs.empty()) //if there are some ROIs in the set
	{
		Console::write("FS::GpixD\tGot past theROIs empty check\n");
		for (int cr = 0; cr < theROIs.size(); cr++)
		{
			fsROIPoints.clear(); // Clear the ROI hash
			Console::write("FS::GpixD\tGot into the ROIs loop\n");
			currentROI = theROIs.at(cr);
			vector<ROIEntity*> theEntities = currentROI->get_entities();
			Console::write("FS::GpixD\tGot vector of entities\n");
			if (!theEntities.empty())
			{
				Console::write("FS::GpixD\tSet our current ROI\n");
				ROIEntity* currentEntity;
				if (currentROI->get_active())
				{
					Console::write("FS::GpixD\tCurrent ROI is active -- checking types\n");
					for(int ce = 0; ce < theEntities.size(); ce++)
					{
						currentEntity = theEntities.at(ce);
						Console::write("FS::GpixD\tSet our current entity\n");
						
						char* theType = (char*)currentEntity->get_type();
						
						if(theType == ROI_POINT) //ROI == point
						{
							Console::write("FS::GpixD\tCurrent type is POINT\n");
							//add data at point to data lists
							getPointData(currentEntity, currentROI);
						}
						else if(theType == ROI_RECT) //ROI == rectangle
						{
							Console::write("FS::GpixD\tCurrent type is RECT\n");
							//add data at all points in rectangle to data lists
							getRectData(currentEntity, currentROI);
						}
						else
						{
							Console::write("FS::GpixD\tCurrent type is POLY\n");
							//add data at all points in rectangle to data lists
							getPolygonData(currentEntity, currentROI);
						}
					}
					int red, green, blue;
					currentROI->get_colour(&red, &green, &blue);
					fsgl->add_points(fsROIPoints, red, green, blue);
				}
			}
			else
			{
				Console::write("FS::GpixD\tthe entities set is empty\n");	
			}
		}
		fsROIPoints.clear(); // Clear the ROI hash
	}
	else
	{
		Console::write("FS::GpixD\tthe ROIs set is empty\n");
	}
	#endif // ENABLE_ROI_POINTS
	
	if (!onlyROIs) fsgl->add_points(fsImagePoints, 255, 255, 255);
	fsImagePoints.clear();

	delete fsTileset;
	
	Console::write("FS::GpixD\tTileset destroyed.\n");
}

// Get entire image into ROI
void FeatureSpace::getImageData(void)
{
	int lastcol = fsTileset->get_columns() - 1;
	int lastrow = fsTileset->get_rows() - 1;
	int lastcoldatawidth = fsTileset->get_last_column_width() / LODfactor;
	int lastrowdataheight = fsTileset->get_last_row_height() / LODfactor;
	unsigned char* tile;
	// We can clip data, to avoid some bad files skewing data averages
//	const unsigned char top_clip = 250;
//	const unsigned char bottom_clip = 5;

	// For all non-last-row non-last-column tiles
	for (int row = 0; row < lastrow; row++) {
		for (int col = 0; col < lastcol; col++) {
			tile = (unsigned char*) fsTileset->get_tile_RGB_LOD(col * tileSize, row * tileSize, band1, band2, band3);
			// Add every point
			for (int pos = 0; pos < tileSize * tileSize * 3; pos = pos + 3) {
//				if (tile[pos] < top_clip && tile[pos+1] < top_clip && tile[pos+2] < top_clip &&
//				    tile[pos] > bottom_clip && tile[pos+1] > bottom_clip && tile[pos+2] > bottom_clip) {
					addToImageFSTable(tile[pos], tile[pos+1], tile[pos+2]);
					numberPoints++;
//				}
			}
			delete[] tile;
		}
	}
	
	// For the last column
	for (int row = 0; row < lastrow; row++) {
		tile = (unsigned char*) fsTileset->get_tile_RGB_LOD(lastcol * tileSize, row * tileSize, band1, band2, band3);
		for(int y = 0; y < tileSize; y++) {
			for(int x = 0; x < lastcoldatawidth; x++) {
				int pos = (y * tileSize + x) * 3;
				addToImageFSTable(tile[pos], tile[pos+1], tile[pos+2]);
				numberPoints++;
			}
		}
		delete[] tile;
	}
	// For the last row
	for (int col = 0; col < lastcol; col++) {
		tile = (unsigned char*) fsTileset->get_tile_RGB_LOD(col * tileSize, lastrow * tileSize, band1, band2, band3);
		for(int y = 0; y < lastrowdataheight; y++) {
			for(int x = 0; x < tileSize; x++) {
				int pos = (y * tileSize + x) * 3;
				addToImageFSTable(tile[pos], tile[pos+1], tile[pos+2]);
				numberPoints++;
			}
		}
		delete[] tile;
	}
	// For the last tile
	tile = (unsigned char*) fsTileset->get_tile_RGB_LOD(lastcol * tileSize, lastrow * tileSize, band1, band2, band3);
	for(int y = 0; y < lastcoldatawidth; y++) {
		for(int x = 0; x < lastrowdataheight; x++) {
			int pos = (y * tileSize + x) * 3;
			addToImageFSTable(tile[pos], tile[pos+1], tile[pos+2]);
			numberPoints++;
		}
	}
	delete[] tile;
}

// -----------------------------------------------------------------------------------------
// getPointData
// -----------------------------------------------------------------------------------------
// Adds a point to the pix data lists
// -----------------------------------------------------------------------------------------
void FeatureSpace::getPointData(ROIEntity* theEntity, ROI* theROI)
{
	vector<coords> theCoords = theEntity->get_points();
	int x = theCoords[0].x;
	int y = theCoords[0].y;
	int fx = x / LODfactor;
	int fy = y / LODfactor;
	int fromTileXOrigin = (fx / tileSize) * tileSize;
	int fromTileYOrigin = (fy / tileSize) * tileSize;
	int tilex = fx % tileSize;
	int tiley = fy % tileSize;
	
	Console::write("tile X orig = %d, tile Y orig = %d\n", fromTileXOrigin, fromTileYOrigin);
	Console::write("fx = %d, fy = %d\n", fx, fy);
	Console::write("x pos in tile = %d, y pos in tile = %d\n", tilex, tiley);
	
	char* grabbedData = fsTileset->get_tile_RGB_LOD(fx, fy, band1, band2, band3);
	
	Console::write("got past tile load\n");
	
	int offset = (tiley * tileSize * 3) + (tilex * 3);
	unsigned char b1 = (unsigned char)grabbedData[offset];
	unsigned char b2 = (unsigned char)grabbedData[offset + 1];
	unsigned char b3 = (unsigned char)grabbedData[offset + 2];
	
	addToROIFSTable(b1, b2, b3);
	delete[] grabbedData;
}

// -----------------------------------------------------------------------------------------
// getRectData
// -----------------------------------------------------------------------------------------
// Gets data for all points inside a given rect
// -----------------------------------------------------------------------------------------

void FeatureSpace::getRectData(ROIEntity* theEntity, ROI* theROI)
{
	vector<coords> theCoords = theEntity->get_points();
	int x1, x2, y1, y2;
	int fx1, fx2, fy1, fy2;
	int startx, starty, endx, endy;
	
	//init our rectangle coords
	x1 = theCoords[0].x;
	y1 = theCoords[0].y;
	x2 = theCoords[1].x;
	y2 = theCoords[1].y;
	
	//swap the coords if p2 points greater than p1
	if (x2 < x1)
	{
		int temp = x1;
		x1 = x2;
		x2 = temp;
	}
	if (y2 < y1)
	{
		int temp = y1;
		y1 = y2;
		y2 = temp;
	}
	
	//get our x and y in terms of the LOD
	fx1 = x1 / LODfactor;
	fy1 = y1 / LODfactor;
	fx2 = x2 / LODfactor;
	fy2 = y2 / LODfactor;
	
	Console::write("FS::Rect - Rect LOD coords are x1 = %d, y1 = %d, x2 = %d, y2 = %d\n", fx1, fy1, fx2, fy2);
	
	//work out the origin points of our start and end tiles
	startx = (fx1 / tileSize) * tileSize;
	starty = (fy1 / tileSize) * tileSize;
	endx = (fx2 / tileSize) * tileSize;
	endy = (fy2 / tileSize) * tileSize;
	
	Console::write("FS::Rect - StartX = %d, StartY = %d, EndX = %d, EndY = %d\n", startx, starty, endx, endy);
	
	//loop through all tiles that have some points we're interested in
	for(int tiley = starty; tiley <= endy; tiley+=tileSize)
	{
		for(int tilex = startx; tilex <= endx; tilex+=tileSize)
		{
			Console::write("FS::Rect - In tile starting at x %d, y %d\n", tilex, tiley);
			int x, y;
			
			char* grabbedData = fsTileset->get_tile_RGB_LOD(tilex, tiley, band1, band2, band3);
			
			for(y = max(tiley, fy1); y < min((fy2 + 1), tiley + tileSize); y++)
			{
				for(x = max(tilex, fx1); x < min((fx2 + 1), tilex + tileSize); x++)
				{
					//Console::write("FS::Rect - Getting point at x %d, y %d\n", x, y);
					int offx = x % tileSize;
					int offy = y % tileSize;
					int offset = (offy * tileSize * 3) + (offx * 3);
					unsigned char b1 = (unsigned char)grabbedData[offset];
					unsigned char b2 = (unsigned char)grabbedData[offset + 1];
					unsigned char b3 = (unsigned char)grabbedData[offset + 2];
					addToROIFSTable(b1, b2, b3);
				}
			}
			delete[] grabbedData;
		}
	}
}

// -----------------------------------------------------------------------------------------
// getPolygonData
// -----------------------------------------------------------------------------------------
// This function creates a list of the coordinates of all points circumscribed by a
// polygon. It does so by recording the x coordinate for any points at a given y coordinate.
// The algorithm generates these boundary points in sorted order from lowest x to highest x,
// and there will always be an even number of these values.
// The algorithm then gets the pixel value of any point x, y, where y is the current y being
// gathered, and x is a value between the pairs of x points stored for that y.
// -----------------------------------------------------------------------------------------

void FeatureSpace::getPolygonData(ROIEntity* theEntity, ROI* theROI)
{	
	int totalPoints = 0;
	int fx1, fx2, fy1, fy2;
	int startx, starty, endx, endy;
	maxy = 0;
	maxx = 0;
	miny = std::numeric_limits<int>::max();
	minx = std::numeric_limits<int>::max();
	yoffset = 0;
	vectorsize = 0;
	
	int time_start, time_end;
	
	getPolygonVertices(theEntity);
	assert(polyPoints.size() > 0);
	
	if (!polyPoints.empty())	//if we've got something to rasterise...
	{
		yoffset = miny;
		vectorsize = (maxy - miny) + 1;
		boundsCoords.resize(vectorsize, new list<int>);
		//pixCoords.resize(vectorsize, new list<int>);
		
		//get the point coordinates for our boundary lines first
		for (int i = 0; i < polyPoints.size() - 1; i++)
		{
			generateBoundaryLine(polyPoints[i].x, polyPoints[i].y, polyPoints[i+1].x, polyPoints[i+1].y);
		}
		generateBoundaryLine(polyPoints[polyPoints.size()-1].x, polyPoints[polyPoints.size()-1].y, polyPoints[0].x, polyPoints[0].y);
		
		//sort our resultant lists
		int sortstart = GetTickCount();
		for (int i = 0; i < vectorsize; i++)
		{
			boundsCoords[i]->sort();
		}
		int sortend = GetTickCount();
		Console::write("FS::GPD Time to sort lists was %f seconds\n\n", (float)(sortend - sortstart) / 1000.0);
		
		//get the point coordinates for all points between those lines, if size not odd
		time_start = GetTickCount();
		
		//get bounding rect
		
		//get our x and y in terms of the LOD
		fx1 = minx;
		fy1 = miny;
		fx2 = maxx;
		fy2 = maxy;
		
		Console::write("FS::GPD - Poly LOD bounds are x1 = %d, y1 = %d, x2 = %d, y2 = %d\n", fx1, fy1, fx2, fy2);
		
		//work out the origin points of our start and end tiles
		startx = (fx1 / tileSize) * tileSize;
		starty = (fy1 / tileSize) * tileSize;
		endx = (fx2 / tileSize) * tileSize;
		endy = (fy2 / tileSize) * tileSize;
		
		Console::write("FS::GPD - StartX = %d, StartY = %d, EndX = %d, EndY = %d\n", startx, starty, endx, endy);
		
		for(int tiley = starty; tiley <= endy; tiley+=tileSize)
		{
			Console::write("FS::GPD - Traversing tiles in row starting at %d\n", tiley);
			for(int tilex = startx; tilex <= endx; tilex+=tileSize)
			{
				int tileendx = tilex + tileSize;
				int tileendy = tiley + tileSize;
				
				Console::write("FS::GPD - In tile starting at x %d, y %d\n", tilex, tiley);
				Console::write("FS::GPD - Current tile bounds: x%d,y%d & x%d,y%d\n", tilex, tiley, tileendx, tileendy);
				char* grabbedData = fsTileset->get_tile_RGB_LOD(tilex, tiley, band1, band2, band3);
				
				int loopstarty = max((tiley - yoffset), 0);
				int loopendy = min(vectorsize, tileendy - yoffset);
				assert(loopstarty >= 0);
				assert(loopendy <= vectorsize);
				
				Console::write("FS::GPD - in this pass, starting from y%d, and going to y%d\n", loopstarty, loopendy);
				
				for (int stepy = loopstarty; stepy < loopendy; stepy++)
				{
					//Console::write("FS::GPD y is %d\n", stepy);
					//iterators for our "point pairs"
					if (boundsCoords[stepy]->size() > 1)
					{
						list<int>::iterator j1 = boundsCoords[stepy]->begin();
						list<int>::iterator j2 = j1;
						//set our second point to the next point in the list
						while(*j1 == *j2)
						{
							j2++;
						}
						if(j2 == boundsCoords[stepy]->end()) j2--;
						
						//loop until we reach the end of the point list
						if ((*j1 == *j2) && (boundsCoords[stepy]->size() == 2))
						{
							int currentx = *j1;
							if(currentx >= tilex && currentx <= tileendx)
							{
								//Console::write("FS::GPD - Getting binary point at x %d, y %d\n", currentx, (stepy + yoffset));
								int offx = currentx % tileSize;
								int offy = (stepy + yoffset) % tileSize;
								int offset = (offy * tileSize * 3) + (offx * 3);
								unsigned char b1 = (unsigned char)grabbedData[offset];
								unsigned char b2 = (unsigned char)grabbedData[offset + 1];
								unsigned char b3 = (unsigned char)grabbedData[offset + 2];
								addToROIFSTable(b1, b2, b3);
								totalPoints++;
							}
						}
						else
						{
							while(true)
							{
		
								for(int stepx = *j1; stepx <= *j2; stepx++)
								{
									int currentx = stepx;
									if(currentx >= tilex && currentx <= tileendx)
									{
										//Console::write("FS::GPD - Getting point at x %d, y %d\n", currentx, (stepy + yoffset));
										int offx = currentx % tileSize;
										int offy = (stepy + yoffset) % tileSize;
										int offset = (offy * tileSize * 3) + (offx * 3);
										unsigned char b1 = (unsigned char)grabbedData[offset];
										unsigned char b2 = (unsigned char)grabbedData[offset + 1];
										unsigned char b3 = (unsigned char)grabbedData[offset + 2];
										addToROIFSTable(b1, b2, b3);
										totalPoints++;
									}
								}
								
								//move j2 on to the next point in the list
								j2++;
								if(j2 == boundsCoords[stepy]->end()) //if we're at the end of the list, break
								{
									break;
								}
								//otherwise, set j1 to be the next point as well, and
								//move j2 on to the next point in the pair
								j1 = j2;
								j2++;
								if(j2 == boundsCoords[stepy]->end()) //if we're at the end of the list, break
								{
									break;
								}
							}
						}
					}
					else //if (boundsCoords[i]->size() == 1)
					{
						int currentx = *boundsCoords[stepy]->begin();
						if(currentx >= tilex && currentx <= tileendx)
						{
							//Console::write("FS::GPD - Getting single point at x %d, y %d\n", currentx, (stepy + yoffset));
							int offx = currentx % tileSize;
							int offy = (stepy + yoffset) % tileSize;
							int offset = (offy * tileSize * 3) + (offx * 3);
							unsigned char b1 = (unsigned char)grabbedData[offset];
							unsigned char b2 = (unsigned char)grabbedData[offset + 1];
							unsigned char b3 = (unsigned char)grabbedData[offset + 2];
							addToROIFSTable(b1, b2, b3);
							totalPoints++;
						}
					}
				}
				
				delete[] grabbedData;
			}
		}
		time_end = GetTickCount();
		Console::write("FS::GPD Time to draw points between pairs was %f seconds\n", (float)(time_end - time_start) / 1000.0);
	}
	
	//write the resultant array
	Console::write("FS::GPD -- Total points in poly = %d\n", totalPoints);
	
	//push the values for this data to our lists of values -- TO DO
	
	//we're finished -- clear our data for the next run
	time_start = GetTickCount();
	for (int i = 0; i < vectorsize; i++)
	{
		delete boundsCoords[i];
		//delete pixCoords[i];
	}
	time_end = GetTickCount();
	Console::write("FS::GPD Time to clear vectors was %f seconds\n", (float)(time_end - time_start) / 1000.0);
	
	boundsCoords.clear();
//	pixCoords.clear();
	maxy = 0; 
	miny = std::numeric_limits<int>::max();
	yoffset = 0;
	vectorsize = 0;
}

// -----------------------------------------------------------------------------------------
// getPolygonVertices
// -----------------------------------------------------------------------------------------
// convert the polygon vertices for the current ROI to vector<myPoint> format
// -----------------------------------------------------------------------------------------

void FeatureSpace::getPolygonVertices(ROIEntity* theEntity)
{
	vector<coords> theCoords = theEntity->get_points();
	coords currentCoords;
	
	//clear our vector of polygon points
	polyPoints.clear();
	
	//make a new point structure, get the data,
	//and push it to the back of the vector
	for(int i = 0; i < theCoords.size(); i++)
	{
		currentCoords = theCoords.at(i);
		Console::write("FS::GetPolyPoints - coord %d = x %d, y %d\n", i, currentCoords.x, currentCoords.y);
		myPoint* thePoint = new myPoint;
		thePoint->x = currentCoords.x / LODfactor;
		thePoint->y = currentCoords.y / LODfactor;
		Console::write("FS::GetPolyPoints - LOD coord %d = x %d, y %d\n", i, thePoint->x, thePoint->y);
		polyPoints.push_back(*thePoint);
		//Console::write("FS:GPV Poly point at X %d, Y %d\n", thePoint->x, thePoint->y);
		
		//determine if this point is a max or min y/x value
		if (thePoint->y > maxy) maxy = thePoint->y;
		if (thePoint->y < miny) miny = thePoint->y;
		if (thePoint->x > maxx) maxx = thePoint->x;
		if (thePoint->x < minx) minx = thePoint->x;
	}
	
	Console::write("FS::GetPolyPoints - maxx = %d, minx = %d\n", maxx, minx);
	Console::write("FS::GetPolyPoints - maxy = %d, miny = %d\n", maxy, miny);
}

// -----------------------------------------------------------------------------------------
// isTurningPoint
// -----------------------------------------------------------------------------------------
// Determine whether a point is a turning point in the y-axis.
// Algorithm by Rowan James, execution by Dafydd Williams
// -----------------------------------------------------------------------------------------

void FeatureSpace::isTurningPoint(int first, int middle, int last)
{
	//Console::write("examining point X %d, Y %d\n", polyPoints[middle].x, polyPoints[middle].y);
	if((polyPoints[first].y > polyPoints[middle].y && polyPoints[last].y > polyPoints[middle].y) || //if both neighbour y greater than middle y
	   (polyPoints[first].y < polyPoints[middle].y && polyPoints[last].y < polyPoints[middle].y)) //or if both neighbour y less than middle y
	{
		//make a new list of int to copy known good points to
		list<int>* tempList = new list<int>;
		
		//Console::write("FS::TP\tpoint is turning point -- attempting to find pixel values matching\n");
		if (boundsCoords[polyPoints[middle].y - yoffset]->size() == 1)
		{
			//Console::write("FS::TP\t\tSingle point - copying to new list\n");
			tempList->push_back(*boundsCoords[polyPoints[middle].y - yoffset]->begin());
		}
		else
		{
			for(list<int>::iterator j = boundsCoords[polyPoints[middle].y - yoffset]->begin(); j != boundsCoords[polyPoints[middle].y - yoffset]->end(); j++)
			{
				if(*j != polyPoints[middle].x)
				{
					//Console::write("FS::TP\t\tCopying to new list where X = %d\n", *j);
					tempList->push_back(*j);
				}
			}
		}
		
		//write our new list to replace our old list
		delete boundsCoords[polyPoints[middle].y - yoffset];
		boundsCoords[polyPoints[middle].y - yoffset] = tempList;
	}
	/*else
	{
		Console::write("FS::TP\tpoint was not turning point\n");
	}*/
}

// -----------------------------------------------------------------------------------------
// generateBoundaryLine
// -----------------------------------------------------------------------------------------
// Generate a line from a to b using the DDA Algorithm.
// Stub code by Tony Gray @ UTas
// -----------------------------------------------------------------------------------------

void FeatureSpace::generateBoundaryLine (int x1, int y1, int x2, int y2)
{
	//Console::write("Line from X1 %d, Y1 %d to X2 %d, Y2 %d\n", x1, y1, x2, y2);
	
	float increment;
	int dx, dy, steps;
	
	//if our second point is less than our first on the x axis, swap the point pairs
	if (x2<x1)
	{
		int temp;
		
		temp = x1;
		x1 = x2;
		x2 = temp;
		temp = y1;
		y1 = y2;
		y2 = temp;
	}
	
	//get our differences between the x coord pair and the y coord pair
	dx = x2 - x1;
	dy = y2 - y1;
	
	//if our x points are identical and our first y is greater than our second y
	//(in other words, if we have a vertical line segment going straight down)
	//swap our y coords only.
	if (x1 == x2 && y1 > y2)
	{
		int temp;
		
		temp = y1;
		y1 = y2;
		y2 = temp;
		//dy = abs(dy);
	}
	
	//We're always stepping by Y in this modified algorithm, so set steps
	steps = abs(dy);
	increment = (float) dx / (float) steps;

	if (dy < 0)
	{
		int temp;
		temp = y1;
		y1 = y2;
		y2 = temp;
		temp = x1;
		x1 = x2;
		x2 = temp;
		increment = increment - (increment * 2);
	}
	//Console::write("We'll start from X %d, Y %d, and go to X %d, Y %d\n", x1, y1, x2, y2);
	float x = (float) x1;
	cout.precision(6);
	//Console::write("After conversion to float, our x starting point is %d\n", x); 
	//Console::write("Our increment is %f\n", increment); 
	if (y1 == y2)
	{
		pushXPixelBounds(x1, y1);
		//plotPixel((short)x1, (short)y1);
		//Console::write("drew a point at X %d, Y %d\n", x1, y1);
		pushXPixelBounds(x2, y1);
		//plotPixel((short)x2, (short)y1);
		//Console::write("drew a point at X %d, Y %d\n", x2, y1);
	}
	else if (x1 == x2)
	{
		if (y1 > y2)
		{
			int temp = y1;
			y1 = y2;
			y2 = temp;
		}
		
		//Console::write("this is a vertical line\n");
		//Console::write("i starts at %d and ends at %d\n", y1, y2); 
		for(int i = y1; i <= y2; i++)
		{
			pushXPixelBounds(x1, i);
			//Console::write("drew a point at X %d, Y %d\n", x1, i);
			//plotPixel((short)x1, (short)i);
		}
	}
	else
	{
		for (int y = y1; y <= y2; y++)
		{
			int rx = int(round(x));
			//draw our pixel, for reference
			//Console::write("drew a point at X %d, Y %d\n", rx, y);
			//plotPixel((short)rx, (short)y);
			
			pushXPixelBounds(rx, y);
			
			x += increment;
		}
	}
}


// -----------------------------------------------------------------------------------------
// pushXPixelBounds
// -----------------------------------------------------------------------------------------
// Push our given X coord into the list of X coords for Y -- tailored for the
// boundary points vector.
// -----------------------------------------------------------------------------------------

void FeatureSpace::pushXPixelBounds(int rx, int y)
{
	//list<int>::iterator i, j;
	
	//Console::write("y - yoffset = %d\n", y - yoffset);
	if (boundsCoords[y - yoffset]->empty())
	{
		//Console::write("\t\tX list for Y = %d is empty\n", y);
		list<int>* tempxlist = new list<int>;
		boundsCoords[y - yoffset] = tempxlist;
		//Console::write("\t\t\tPushed X = %d to front of list for Y = %d\n", rx, y);
	}
	
	boundsCoords[y - yoffset]->push_front(rx);
}


// -----------------------------------------------------------------------------------------
// pushXPixel
// -----------------------------------------------------------------------------------------
// Push our given X coord into the list of X coords for Y.
// -----------------------------------------------------------------------------------------

void FeatureSpace::pushXPixel(int rx, int y)
{
	//list<int>::iterator i, j;
	
	//Console::write("y - yoffset = %d\n", y - yoffset);
	if (pixCoords[y - yoffset]->empty())
	{
		//Console::write("\t\tX list for Y = %d is empty\n", y);
		list<int>* tempxlist = new list<int>;
		pixCoords[y - yoffset] = tempxlist;
		//Console::write("\t\t\tPushed X = %d to front of list for Y = %d\n", rx, y);
	}
	
	pixCoords[y - yoffset]->push_front(rx);
}


// -----------------------------------------------------------------------------------------
// catForHash
// -----------------------------------------------------------------------------------------
// Concatenates three unsigned chars together in preparation for
// passing as a hash key.
// -----------------------------------------------------------------------------------------

unsigned int FeatureSpace::catForHash(unsigned char b1, unsigned char b2, unsigned char b3)
{
	return (unsigned int)b1 + ((unsigned int)b2 << 8) + ((unsigned int)b3 << 16);
}


// -----------------------------------------------------------------------------------------
//addToImageFSTable
// -----------------------------------------------------------------------------------------
//Adds a point to our Image FS hash table.
// -----------------------------------------------------------------------------------------
void FeatureSpace::addToImageFSTable(unsigned char b1, unsigned char b2, unsigned char b3)
{
	unsigned int hash = catForHash(b1, b2, b3);
	fsImagePoints[hash]++;
}


// -----------------------------------------------------------------------------------------
//addToROIFSTable
// -----------------------------------------------------------------------------------------
//Adds a point to current ROI FS hash table
// -----------------------------------------------------------------------------------------
void FeatureSpace::addToROIFSTable(unsigned char b1, unsigned char b2, unsigned char b3)
{
	unsigned int hash = catForHash(b1, b2, b3);
	if (onlyROIs) {
       	fsROIPoints[hash]++;
	} else {
		fsROIPoints[hash] += fsImagePoints[hash];
		fsImagePoints.erase(hash);
	}
}


// create feature space window 
void FeatureSpace::Create() {
    // RECT rect;  // 绉婚櫎Windows鐗瑰畾鐨勪唬鐮?

    // get position of overview window for alignment   
    // GetWindowRect(overviewWindow.GetHandle(),&rect);  // 鏇挎崲涓篞t鏂瑰紡

    // create feature space window
    // const char *title=makeMessage("Feature Space ",numFeatureSpaces+1);
    // if (!CreateWin(0, "Parbat3D Feature Window", title,
    //      WS_OVERLAPPEDWINDOW|WS_SYSMENU|WS_CAPTION|WS_SIZEBOX|WS_MAXIMIZEBOX|WS_MINIMIZEBOX|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
    //      rect.right, rect.top, FEATURE_WINDOW_WIDTH, FEATURE_WINDOW_HEIGHT, NULL, NULL))
    //     return false;  // 淇敼涓篞t鏂瑰紡
    // delete(title);

    // 浣跨敤Qt鏂瑰紡鍒涘缓绐楀彛
    setWindowTitle(QStringLiteral("Feature Space"));
    resize(FEATURE_WINDOW_WIDTH, FEATURE_WINDOW_HEIGHT);
      
    // create child windows
    // glContainer=new GLContainer(GetHandle(),this,0,0,FEATURE_WINDOW_WIDTH,FEATURE_WINDOW_HEIGHT-TOOLBAR_HEIGHT);  // 鏇挎崲涓篞t鏂瑰紡
    glContainer = new GLContainer(this, this, 0, 0, FEATURE_WINDOW_WIDTH, FEATURE_WINDOW_HEIGHT-TOOLBAR_HEIGHT);
    glContainer->setupUI();

    // 构造时若已有布局则直接加入 GLView；否则在 init 中创建
    if (!m_glView) {
        QVBoxLayout *lyt = new QVBoxLayout(glContainer);
        lyt->setContentsMargins(0,0,0,0);
        lyt->setSpacing(0);
        m_glView = new GLView(glContainer);
        m_glView->setMinimumSize(4,4);
        m_glView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        lyt->addWidget(m_glView);
    }

        // handle events for the window
    // prevProc=SetWindowProcedure(&WindowProcedure);  // 绉婚櫎Windows鐗瑰畾鐨勪唬鐮?
    
    // return true;  // 淇敼涓篞t鏂瑰紡
}
 
// resize GLContainer to fit the feature space window
void FeatureSpace::OnResize()
{   
    QRect rect;
    int x,y,width,height;
    
    if (this) {
        rect.setLeft(0);
        rect.setTop(0);
        rect.setRight(this->width());
        rect.setBottom(this->height());
    }
    
    x = rect.left();
    y = rect.top();
    width = rect.width();
    height = rect.height();

    if (glContainer) {
        glContainer->setGeometry(x, y, width, height);
    }

    if (fsgl!=NULL)
        fsgl->resize();
		
    if (m_glView) {
        m_glView->update();
    } else if (glContainer) {
        glContainer->repaint();
    }
}

// Qt resize event: forward to base and trigger layout/GL resize
void FeatureSpace::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    OnResize();
}


// handle left mouse button down event
void FeatureSpace::OnGLContainerLeftMouseDown(int x,int y)
{
	// record inital mouse position for rotating/zooming
	prev_mouse_x=x;
	prev_mouse_y=y;
	Console::write("FeatureSpace::OnMouseDown\n");
}

// handle mouse move event
void FeatureSpace::OnGLContainerMouseMove(int virtualKeys,int x,int y)
{
//	Console::write("FeatureSpace::OnMouseMove\n");
	int x_diff = x - prev_mouse_x;
	int y_diff = y - prev_mouse_y;
//	Console::write("x_diff=%d y_diff=%d \n",x_diff,y_diff);
	// checked if left mouse button is down
	if ((virtualKeys&MK_LBUTTON) && !(virtualKeys&MK_RBUTTON))
	{
		fsgl->rot_cam(x_diff * 0.8f, y_diff * 0.5f);
	}
	// if right mouse button down
	if ((virtualKeys&MK_RBUTTON) && !(virtualKeys&MK_LBUTTON))
	{
		float nx = (float)x_diff/250.0;
		float ny = (float)y_diff/250.0;
		fsgl->translate_cam(nx, -ny);
	} 
	// check both mouse buttons down
	if ((virtualKeys&MK_RBUTTON) && (virtualKeys&MK_LBUTTON))
	{
		fsgl->zoom_cam((float)y_diff / 100.0f);
	}
	
	// Operations are cumulative, so re-set distance
	prev_mouse_x=x;
	prev_mouse_y=y;	
}


// handle key presses on the feature space window
void FeatureSpace::OnKeyPress(int virtualKey)
{
	// check whether control is currently down
	// bool control_pressed=(bool)(GetKeyState(VK_CONTROL)&128);  // 鏇挎崲涓篞t鏂瑰紡

	// check whether shift is currently down
	// bool shift_pressed=(bool)(GetKeyState(VK_SHIFT)&128);  // 鏇挎崲涓篞t鏂瑰紡	
	
	switch (virtualKey)
	{
		
		case /*VK_UP*/ Qt::Key_Up:
			// if (control_pressed)
			// {
			// 	//Rotate(0, 5 * fsgl->rads_to_deg / 2.0);
			// 	fsgl->rot_cam(0, 5);
			// }
			// else if (shift_pressed)
			// {
			// 	fsgl->translate_cam(0, 0.1);
			// }
			// else
				fsgl->translate_cam(0, 0.01);
			break;
			
		case /*VK_DOWN*/ Qt::Key_Down:
			// if (control_pressed)
			// {
			// 	//Rotate(0, -5 * fsgl->rads_to_deg / 2.0);
			// 	fsgl->rot_cam(0, -5);
			// }
			// else if (shift_pressed)
			// {
			// 	fsgl->translate_cam(0, -0.1);
			// }
			// else
				fsgl->translate_cam(0, -0.01);
			break;
			
		case /*VK_LEFT*/ Qt::Key_Left:
			// if (control_pressed)
			// {
			// 	//Rotate(5 * fsgl->rads_to_deg / 2.0, 0);
			// 	fsgl->rot_cam(5, 0);
			// }
			// else if (shift_pressed)
			// {
			// 	fsgl->translate_cam(0.1, 0);
			// }
			// else			
				fsgl->translate_cam(0.01, 0);
			break;
			
		case /*VK_RIGHT*/ Qt::Key_Right:
			// if (control_pressed)
			// {
			// 	//Rotate(-5 * fsgl->rads_to_deg / 2.0, 0);
			// 	fsgl->rot_cam(-5, 0);
			// }	
			// else if (shift_pressed)
			// {
			// 	fsgl->translate_cam(-0.1, 0);
			// }					
			// else
				fsgl->translate_cam(-0.01, 0);
			break;
			
		case /*VK_PRIOR*/ Qt::Key_PageUp:	// page-up key
			fsgl->zoom_cam(0.1f);
			break;
			
		case /*VK_NEXT*/ Qt::Key_PageDown:	// page-down key
			fsgl->zoom_cam(-0.1f);
			break;

		case Qt::Key_Space:
			fsgl->toggle_smooth();
			break;
						
	}
}


#include "PchApp.h"

#include "ImageTab.h"

#include "config.h"
#include "utils/StringUtils.h"
#include "imagery/ImageProperties.h"

// #include "Window.h"  // 移除对Window.h的依赖
#include "MainWindow.h"
#include "imagery/ImageHandler.h"
extern MainWindow* mainWindow;
#include "ScrollBox.h"
#include "ToolTab.h"
#include "ToolWindow.h"


int ImageTab::GetContainerHeight()
{
    return 0;
}

void ImageTab::Create(QWidget *parent)
{
    // ToolTab::Create(parent,parentRect);  // 修改为Qt方式
    
    const int iNumItems = 5;
	/* add the image property information under the image tab */
	if (!mainWindow || !mainWindow->imageHandler()) {
		return;
	}
	ImageProperties* ip = mainWindow->imageHandler()->get_image_properties();
	std::string leader;
	int ipItems= iNumItems;
	std::string n[iNumItems];
	std::string v[iNumItems];
	
	/* If the filename is too long to be displayed, truncate it.
	Later on, a roll-over tooltip should be implemented to bring
	up the full name.*/
	std::string fullname = ip->getFileName();
	std::string fname, bname, finalname;
	if (fullname.length() > 25) {
		fname = fullname.substr(0, 12);
		bname = fullname.substr(fullname.length()-12, fullname.length()-1);
		finalname = fname + "" + bname;
	} else
	    finalname = fullname;
	    
	std::string drivername = ip->getDriverLongName();
	std::string finaldrivname;
	if (drivername.length() > 25) {
		fname = drivername.substr(0, 12);
		bname = drivername.substr(drivername.length()-12, drivername.length()-1);
		finaldrivname = fname + "" + bname;
	} else
	    finaldrivname = drivername;
	
	    
    n[0]="File Name"; v[0]=makeString(leader, finalname);
    n[1]="File Type"; v[1]=makeString(leader, finaldrivname);
    n[2]="Width"; v[2]=makeString(leader, ip->getWidth());
    n[3]="Height"; v[3]=makeString(leader, ip->getHeight());
    n[4]="Bands"; v[4]=makeString(leader, ip->getNumBands());
	
	// 使用Qt组件替代Windows API
	// HWND hstatic;
	// for (int i=0; i<ipItems; i++) {
	// 	hstatic=CreateWindowEx(0, szStaticControl, (char*) n[i].c_str(),
	// 		WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE, 20, 40+(i*20), 50, 18,
	// 		GetHandle(), NULL, Window::GetAppInstance(), NULL);
	// 	SetFont(hstatic,Window::FONT_NORMAL);
		
	// 	hstatic=CreateWindowEx(0, szStaticControl, (char*) v[i].c_str(),
	// 		WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE, 78, 40+(i*20), 160, 18,
	// 		GetHandle(), NULL, Window::GetAppInstance(), NULL);
	// 	SetFont(hstatic,Window::FONT_NORMAL);
    // }
    
    // 这里应该添加Qt组件创建代码
}

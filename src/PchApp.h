#ifndef PCH_APP_H
#define PCH_APP_H

/*
	This header provides program-wide declarations and imports.  It should only
	include system/third-party headers, as changes will require re-building all
	modules.
	
	Each .cpp file in the project should #include this file before any other
	includes, declarations, or code.
*/ 

// We use a fair bit of stuff from the C/C++ standard libraries
#include <cassert>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <cstdarg>
#include <cstdlib>
#include <cstring>

// C++ standard containers
#include <string>
#include <vector>
#include <list>
#include <queue>
#include <deque>
#include <memory>
#include <limits>
#include <algorithm>
#include <sstream>
#include <iostream>   
#include <fstream>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>  // for glm::value_ptr
#include <glm/gtc/matrix_transform.hpp>

// GDAL includes before Windows headers to avoid conflicts
#include <gdal_priv.h>

// Windows headers
#ifdef _WIN32
    #include <windows.h>
    #include <commctrl.h>
    #include <commdlg.h>
    #include <shellapi.h>
	#include <io.h>
	#include <fcntl.h>
#else
	#include <sys/types.h>
#endif

// OpenGL - Include Windows headers before OpenGL to avoid conflicts

// Ensure GLAD's header is included before any OpenGL headers
#include <glad/glad.h>

// // And Win32-API
// #define WINVER 0x0500
// #define _WIN32_IE 0x600
// #define _WIN32_WINNT 0x501

// Qt includes (after GDAL, Windows headers, and GLAD)
#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QScrollBar>
#include <QAbstractSlider>
#include <QSizePolicy>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QTabWidget>
#include <QListWidget>
#include <QGroupBox>
#include <QScrollArea>
#include <QProgressBar>
#include <QLineEdit>
#include <QCheckBox>
#include <QString>
#include <QVector>
#include <QDir>
#include <QTimer>
#include <QKeyEvent>
#include <QImage>
#include <QPixmap>
#include <QIcon>
#include <QColor>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QRect>
#include <QPoint>
#include <QCursor>
#include <QOpenGLWidget>
//#include <QOpenGLFunctions>
// #include <QGLWidget>  // Removed in Qt6, replaced with QOpenGLWidget
#include <QStandardPaths>
#include <QScreen>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

#endif // include guard
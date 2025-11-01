#ifndef _PARBAT_TOOLTAB_H
#define _PARBAT_TOOLTAB_H

#include "PchApp.h"
#include "ScrollBox.h"

// Qt includes
#include <QWidget>
#include <QLabel>
#include <QRect>

class ToolWindow; // forward declaration

class ToolTab : public QWidget
{
	Q_OBJECT

protected:
	ScrollBox scrollBox;
	ToolWindow* mToolWindow = nullptr;
public:
	QWidget* headingWidget;

	virtual int Create(QWidget* parent, QRect* parentRect);
	virtual const char* GetTabName() { return nullptr; };
	virtual const char* GetTabHeading() { return nullptr; };
	virtual int GetContainerHeight() { return 0; };
	virtual void setupUI();

	void setToolWindow(ToolWindow* tw) { mToolWindow = tw; }
	ToolWindow* toolWindow() const { return mToolWindow; }
};

#endif

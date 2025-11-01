#ifndef _PARBAT_ROIWINDOW_H
#define _PARBAT_ROIWINDOW_H

#include <QDialog>
#include <QListWidget>
#include <QToolBar>
#include <QAction>
#include <QIcon>
#include <QColor>
#include <QVector>
#include <QString>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>

#include "imagery/ROI.h"

// class ROIWindow : public QWidget
class ROIWindow : public QDialog
{
    Q_OBJECT

public:
    ROIWindow(QWidget *parent = nullptr);
    virtual ~ROIWindow();

    int Create(QWidget *parent = nullptr);
    
    /**
        Deletes all the Regions of Interest from the list and regionsSet.
        This is called when a new image is opened so that any ROIs from the
        previous image will be removed.
    */
    void deleteAllROI();
    
    /**
        Contains the procedure for adding a new entity to the set of Regions
        of Interest. If no region is selected in the list then a new region
        is created, with the entity inside it. The second parameter
        specifies the type of entity to create.
    */
    void newEntity(ROIWindow*, const char*);
    
    /**
        Make the buttons active or inactive depending on whether an entity
        is currently being created. This prevents the user from trying to
        create a new entity while they are editing one or removing the ROI
        that the entity is being added to.
    */
    void updateButtons(ROIWindow*);
    
    int getWidth() const;
    int getHeight() const;

private slots:
    void onOpenROI();
    void onSaveROI();
    void onNewROI();
    void onDeleteROI();
    void onPolySelection();
    void onRectSelection();
    void onSinglePointSelection();
    void onListItemChanged();
    void onChangeROIColour();

private:
    void setupUI();
    void setupActions();
    void setupToolBars();
    
    /**
        Returns the number of ROI checkboxes that have been checked.
    */
    int getROICheckedCount();
    
    /**
        Returns the index of the selected item in the list. Returns -1 if
        nothing is selected.
    */
    int getSelectedItemIndex();
    
    /**
        Returns the text of the specified item in the ROI list.
    */
    QString getItemText(int i);
    
    /**
        Returns the text of the selected item in the list. The text is the
        name of the ROI that the list item represents. Returns a blank
        string ("") if nothing is selected.
    */
    QString getSelectedItemText();
    
    /**
        Creates a new Region of Interest and, if specified, creates a new
        entity inside the new region. The second parameter accepts a string
        specifying the type of entity to add to the region. If the second
        parameter is left blank then no new entity is added.
    */
    void newROI(ROIWindow*, const char*);
    
    /**
        Contains the procedure for loading a set of Regions of Interest
        from a file. This includes opening a dialog box, getting the
        filename and loading the set from the file.
    */
    void loadROI(ROIWindow*);
    
    /**
        Contains the procedure for saving a set of Regions of Interest to a
        file. This includes opening a dialog box, getting the filename and
        saving the set to the file.
    */
    void saveROI(ROIWindow*);
    
    /**
        Removes an ROI from the set. Removes the ROI from the set, the item
        from the list and the ROIs colour picker. Finally, moves all the
        other colour pickers up one position.
    */
    void deleteROI(ROIWindow*);
    
    void updateROIList(ROIWindow*);
    void addNewRoiToList(ROI *rCur, int newId);
    
    // UI组件
    QWidget *centralWidget;
    QListWidget *roiListWidget;
    QToolBar *roiToolBar;
    
    // 动作
    QAction *openAction;
    QAction *saveAction;
    QAction *newAction;
    QAction *deleteAction;
    QAction *polyAction;
    QAction *rectAction;
    QAction *singlePointAction;
    
    // 图标
    QIcon openIcon;
    QIcon saveIcon;
    QIcon newIcon;
    QIcon deleteIcon;
    QIcon polyIcon;
    QIcon rectIcon;
    QIcon singlePointIcon;
    
    // 颜色相关
    QVector<QWidget*> roiColourButtonList;
    QColor customColours[16];
    
    static QString *editingROIName;
    
    static const int WIDTH = 250;
    static const int HEIGHT = 300;
};

#endif
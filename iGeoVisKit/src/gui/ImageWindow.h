#ifndef _PARBAT_IMAGEWINDOW_H
#define _PARBAT_IMAGEWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QImage>
#include <QPixmap>
#include <QKeyEvent>

#include "imagery/ImageHandler.h"

class ImageWindow : public QWidget
{
    Q_OBJECT

public:
    ImageWindow(QWidget *parent = nullptr);
    virtual ~ImageWindow();

    virtual int Create(QWidget* parent = nullptr);
    
    void scrollImage(bool vertical, int amount);
    void updateImageScrollbar();
    void updateImageWindowTitle();

    // 作为 GLView 的父容器，直接使用自身（移除 DisplayWindow）
    
    int getWidth() const;
    int getHeight() const;

public slots:
    void zoomIn();
    void zoomOut();
    void setFitToWindow(bool enabled);
    void onHorizontalScroll(int value);
    void onVerticalScroll(int value);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void setupUI();
    void loadImage(const QString &fileName);
    void scrollImageX(int scrollMsg);
    void scrollImageY(int scrollMsg);
    void zoomImage(float nlevels);
    void onKeyDown(int virtualKey);
    
    // UI组件
    QScrollArea *scrollArea;
    QLabel *imageLabel;
    QScrollBar *horizontalScrollBar;
    QScrollBar *verticalScrollBar;
    qreal scaleFactor;
    QImage currentImage;
    QPixmap currentPixmap;
    bool m_fitToWindow = false;
};

#endif
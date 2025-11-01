#include "PchApp.h"

#include "MainWindow.h"
#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QDockWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include "OverviewWindow.h"
#include "ToolWindow.h"
#include "ROIWindow.h"
#include "PrefsWindow.h"
#include "ImageWindow.h"
#include "ContrastWindow.h"
#include "ContrastAdvWindow.h"
#include "utils/Console.h"
#include "imagery/ROISet.h"
#include "ProgressStatusWidget.h"
#include "imagery/Renderer.h"
#include "imagery/ImageHandler.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 鍒涘缓缁熶竴鐨?Renderer 瀹炰緥
    renderer_ = std::make_shared<Renderer>();
    
    setupActions();
    setupMenus();
    
    // 璁剧疆绐楀彛鏍囬
    setWindowTitle("Parbat3D");
    
    // 鍒涘缓涓ぎ Tab 瀹瑰櫒
    centralTabs_ = new QTabWidget(this);
    setCentralWidget(centralTabs_);

    // 娣诲姞鈥淚mage鈥濋〉锛氬祵鍏ョ幇鏈?ImageWindow 浣滀负涓績瑙嗗浘锛岄伩鍏嶉《灞傜獥鍙ｅ脊鍑?
    imageTabPage_ = new QWidget(centralTabs_);
    {
        QVBoxLayout *imageLayout = new QVBoxLayout(imageTabPage_);
        imageLayout->setContentsMargins(0,0,0,0);
        imageWindow_ = new ImageWindow(this);
        imageWindow_->Create(this);
        imageLayout->addWidget(imageWindow_);
    }
    centralTabs_->addTab(imageTabPage_, QStringLiteral("Image"));
    // ImageWindow 宸插祵鍏ヤ腑澶爣绛鹃〉

    // 宸︿晶锛氭枃浠舵爲 Dock
    filesDock_ = new QDockWidget(QStringLiteral("Images"), this);
    filesTree_ = new QTreeWidget(filesDock_);
    filesTree_->setHeaderLabel(QStringLiteral("Opened Images"));
    filesDock_->setWidget(filesTree_);
    addDockWidget(Qt::LeftDockWidgetArea, filesDock_);
    connect(filesTree_, &QTreeWidget::itemActivated, this, &MainWindow::onFileItemActivated);

    // 宸︿晶搴曢儴锛歄verview Dock锛堜笌鏂囦欢鏍戝瀭鐩村垎鍓诧級
    overviewDock_ = new QDockWidget(QStringLiteral("Overview"), this);
    overviewWidget_ = new OverviewWindow(this);
    overviewWidget_->Create(this);
    overviewDock_->setWidget(overviewWidget_);
    addDockWidget(Qt::LeftDockWidgetArea, overviewDock_);
    splitDockWidget(filesDock_, overviewDock_, Qt::Vertical);

    // 鍙充晶锛歍ools Dock
    toolsDock_ = new QDockWidget(QStringLiteral("Tools"), this);
    toolsWidget_ = new ToolWindow(this);
    toolsWidget_->Create(this);
    toolsDock_->setWidget(toolsWidget_);
    addDockWidget(Qt::RightDockWidgetArea, toolsDock_);

    // ROI 闈炴ā鎬佸璇濇锛堥粯璁ら殣钘忥級
    roiWindow_ = new ROIWindow(this);
    roiWindow_->Create(this);
    roiWindow_->hide();

    // 姒傝涓庡伐鍏风獥鍙ｅ凡浣滀负 Dock 绠＄悊
    
    // 璁剧疆绐楀彛澶у皬
    resize(800, 600);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupActions()
{
    // 鏂囦欢鑿滃崟鍔ㄤ綔
    openAction_ = new QAction("&Open", this);
    openAction_->setShortcut(QKeySequence::Open);
    connect(openAction_, &QAction::triggered, this, &MainWindow::openFile);
    
    saveAction_ = new QAction("&Save", this);
    saveAction_->setShortcut(QKeySequence::Save);
    
    // 杩炴帴 ROI 鑿滃崟椤癸紝鐢ㄤ簬鏄剧ず/闅愯棌闈炴ā鎬?ROI 瀵硅瘽妗?
    roiWindowAction_ = new QAction("ROI Window", this);
    roiWindowAction_->setCheckable(true);
    connect(roiWindowAction_, &QAction::toggled, [this](bool checked){
        if (!roiWindow_) return;
        if (checked) roiWindow_->show(); else roiWindow_->hide();
    });

    saveAsAction_ = new QAction("Save &As...", this);
    saveAsAction_->setShortcut(QKeySequence::SaveAs);
    
    closeAction_ = new QAction("&Close", this);
    closeAction_->setShortcut(QKeySequence::Close);
    
    exitAction_ = new QAction("E&xit", this);
    exitAction_->setShortcut(QKeySequence::Quit);
    connect(exitAction_, &QAction::triggered, qApp, &QApplication::quit);
    
    // 绐楀彛鑿滃崟鍔ㄤ綔
    imageWindowAction_ = new QAction("&Image Window", this);
    toolsWindowAction_ = new QAction("&Tools Window", this);
    // roiWindowAction_ 宸插湪涓婃柟鍒涘缓骞惰繛鎺?
    prefsWindowAction_ = new QAction("&Preferences Window", this);
    contrastWindowAction_ = new QAction("&Contrast Window", this);
    contrastAdvWindowAction_ = new QAction("&Advanced Contrast Window", this);

    // 缁熶竴瀵硅瘽妗嗗叆鍙ｏ細鍔ㄤ綔瑙﹀彂鍗虫樉绀猴紙鎯版€у垱寤猴級
    connect(prefsWindowAction_, &QAction::triggered, [this]{ if (this->prefsWindow()) this->prefsWindow()->show(); });
    connect(contrastWindowAction_, &QAction::triggered, [this]{ if (this->contrastWindow()) this->contrastWindow()->show(); });
    connect(contrastAdvWindowAction_, &QAction::triggered, [this]{ if (this->contrastAdvWindow()) this->contrastAdvWindow()->show(); });
    
    // 甯姪鑿滃崟鍔ㄤ綔
    helpContentsAction_ = new QAction("&Help Contents", this);
    aboutAction_ = new QAction("&About", this);

    // 瑙嗗浘鑿滃崟鍔ㄤ綔锛堢敱 MainWindow 绠＄悊锛屼綔鐢ㄤ簬宓屽叆寮?ImageWindow锛?
    zoomInAction_ = new QAction("Zoom &In (25%)", this);
    zoomOutAction_ = new QAction("Zoom &Out (25%)", this);
    fitToWindowAction_ = new QAction("&Fit to Window", this);
    fitToWindowAction_->setCheckable(true);
    // 鍒濆绂佺敤锛屽緟鎴愬姛鍔犺浇鍥惧儚鍚庡惎鐢?
    zoomInAction_->setEnabled(false);
    zoomOutAction_->setEnabled(false);
    fitToWindowAction_->setEnabled(false);
    // 杩炴帴鍒?ImageWindow 鐨勬Ы鍑芥暟
    connect(zoomInAction_, &QAction::triggered, [this]{ if (imageWindow_) imageWindow_->zoomIn(); });
    connect(zoomOutAction_, &QAction::triggered, [this]{ if (imageWindow_) imageWindow_->zoomOut(); });
    connect(fitToWindowAction_, &QAction::toggled, [this](bool checked){ if (imageWindow_) imageWindow_->setFitToWindow(checked); });
}

void MainWindow::setupMenus()
{
    // 鏂囦欢鑿滃崟
    fileMenu_ = menuBar()->addMenu("&File");
    fileMenu_->addAction(openAction_);
    fileMenu_->addAction(saveAction_);
    fileMenu_->addAction(saveAsAction_);
    fileMenu_->addSeparator();
    fileMenu_->addAction(closeAction_);
    fileMenu_->addSeparator();
    fileMenu_->addAction(exitAction_);
    
    // 绐楀彛鑿滃崟
    windowMenu_ = menuBar()->addMenu("&Window");
    windowMenu_->addAction(imageWindowAction_);
    windowMenu_->addAction(toolsWindowAction_);
    windowMenu_->addAction(roiWindowAction_);
    windowMenu_->addAction(prefsWindowAction_);
    windowMenu_->addAction(contrastWindowAction_);
    windowMenu_->addAction(contrastAdvWindowAction_);
    
    // 甯姪鑿滃崟
    helpMenu_ = menuBar()->addMenu("&Help");
    helpMenu_->addAction(helpContentsAction_);
    helpMenu_->addAction(aboutAction_);

    // 瑙嗗浘鑿滃崟锛堟斁澶?缂╁皬/閫傞厤绐楀彛锛?
    viewMenu_ = menuBar()->addMenu("&View");
    viewMenu_->addAction(zoomInAction_);
    viewMenu_->addAction(zoomOutAction_);
    viewMenu_->addSeparator();
    viewMenu_->addAction(fitToWindowAction_);

    // 缁熶竴鍏ュ彛鍒板伐鍏锋爮
    toolsBar_ = addToolBar("Tools");
    toolsBar_->addAction(roiWindowAction_);
    toolsBar_->addAction(contrastWindowAction_);
    toolsBar_->addAction(contrastAdvWindowAction_);
    toolsBar_->addAction(prefsWindowAction_);
}

void MainWindow::openFile()
{
    // 閫夋嫨鏂囦欢鍚庯細鍔犲叆宸︿晶鏍戝苟閫氳繃鍏ㄥ眬娴佺▼鍔犺浇
    QString fileName = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("Please choose a file"),
        QString(),
        QStringLiteral("All Supported Images (*.ecw *.jpg *.tif *.j2k *.jp2);;ERMapper Compressed Wavelets (*.ecw);;JPEG (*.jpg);;JPEG 2000 (*.j2k *.jp2);;TIFF / GeoTIFF (*.tif);;All Files (*.*)")
    );
    if (fileName.isEmpty()) return;

    // 娣诲姞鍒版爲锛堜笉閲嶅锛?
    bool exists = false;
    for (int i = 0; i < filesTree_->topLevelItemCount(); ++i) {
        QTreeWidgetItem* it = filesTree_->topLevelItem(i);
        if (it && it->data(0, Qt::UserRole).toString() == fileName) { exists = true; break; }
    }
    if (!exists) {
        QTreeWidgetItem* item = new QTreeWidgetItem(filesTree_);
        item->setText(0, QFileInfo(fileName).fileName());
        item->setData(0, Qt::UserRole, fileName);
        filesTree_->addTopLevelItem(item);
    }

    // 閫氳繃閲嶈浇鐨勫叏灞€鍑芥暟鍔犺浇鎸囧畾鏂囦欢
    loadFile(fileName);
}

void MainWindow::RestoreAll()
{
    // 鎭㈠鎵€鏈夌獥鍙ｅ埌鍘熷鐘舵€?
}

void MainWindow::MinimizeAll()
{
    // 闅愯棌鎵€鏈夌獥鍙?
}

void MainWindow::DestroyAll()
{
    // 閿€姣佹墍鏈夌獥鍙?
}

void MainWindow::DisableAll()
{
    // 绂佺敤鎵€鏈夌獥鍙?
}

void MainWindow::DisableAll(QWidget*)
{
    // 绂佺敤闄ょ壒瀹氱獥鍙ｅ鐨勬墍鏈夌獥鍙?
}

void MainWindow::EnableAll()
{
    // 鍚敤鎵€鏈夌獥鍙?
}

void MainWindow::onFileItemActivated(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column);
    if (!item) return;
    const QString fileName = item->data(0, Qt::UserRole).toString();
    if (!fileName.isEmpty()) {
        loadFile(fileName);
    }
}

// 鎯版€ц闂櫒瀹炵幇锛氫粎鍦ㄧ涓€娆¤闂椂鍒涘缓
PrefsWindow* MainWindow::prefsWindow()
{
    if (!prefsWindow_) {
        prefsWindow_ = new PrefsWindow(this);
        prefsWindow_->Create(this);
    }
    return prefsWindow_;
}

ContrastWindow* MainWindow::contrastWindow()
{
    if (!contrastWindow_) {
        // 瀵规瘮搴︾獥鍙ｅ巻鍙蹭笂浠?ImageWindow 浣滀负鐖剁獥鍙?
        contrastWindow_ = new ContrastWindow(imageWindow_);
        contrastWindow_->Create(imageWindow_);
    }
    return contrastWindow_;
}

ContrastAdvWindow* MainWindow::contrastAdvWindow()
{
    if (!contrastAdvWindow_) {
        contrastAdvWindow_ = new ContrastAdvWindow(this);
        contrastAdvWindow_->Create(this);
    }
    return contrastAdvWindow_;
}

ProgressStatusWidget* MainWindow::progressWidget()
{
    if (!progressWidget_) {
        progressWidget_ = new ProgressStatusWidget(this);
        if (statusBar()) {
            statusBar()->addPermanentWidget(progressWidget_, 0);
        }
        progressWidget_->setVisible(false);
    }
    return progressWidget_;
}

void MainWindow::loadFile(const QString& fileName) {
    if (fileName.isEmpty()) return;

    // 娓呯悊涓婁竴娆＄殑鎵撳紑鐘舵€侊紙淇濇寔鍗曡鍥俱€佷絾鍏佽鍒楄〃涓繚瀛樺涓」锛?
    closeFile();

    // 鏂囦欢瀵硅瘽妗嗗叧闂悗锛岀‘淇濅富绐楀彛淇濇寔鍓嶅彴鍙涓庢縺娲?
    ensureVisibleAndActive();
    forceRefreshUI();

    // 灏嗘枃浠跺悕淇濆瓨涓?C 椋庢牸瀛楃涓蹭緵 GDAL 浣跨敤
    QByteArray utf8Bytes = fileName.toUtf8();
    m_filename = std::string(utf8Bytes.constData());

    // 纭繚骞舵樉绀虹姸鎬佹爮杩涘害鎺т欢锛堢敱 MainWindow 缁熶竴绠＄悊锛?
    ProgressStatusWidget* pw = this->progressWidget();
    if (pw) {
        pw->start(100, true);
        QApplication::processEvents(QEventLoop::AllEvents);
    }

    // 鐩稿叧绐楀彛鍧囧凡鍦?MainWindow 鏋勯€犳垨鎯版€у垱寤猴紝鏃犻渶閲嶅鍒涘缓

    // 鏋勫缓 GDAL 鍥惧儚澶勭悊绠＄嚎
    try {
        QWidget* overviewParent = nullptr;
        QWidget* imageParent = nullptr;

        // 淇濇姢鎬ч€夋嫨鐖剁獥鍙ｏ紝閬垮厤绌烘寚閽堟柇瑷€
        if (this->overviewWindow()) {
            // 灏嗘瑙堢殑 OpenGL 瀹瑰櫒浼犲叆
            overviewParent = this->overviewWindow()->displayWidget();
        } else if (overviewDock_) {
            overviewParent = overviewDock_->widget();
        } else {
            overviewParent = this;
        }

        if (this->imageWindow()) {
            // 鐩存帴浣跨敤 ImageWindow 鑷韩浣滀负 OpenGL 鐖舵帶浠?
            imageParent = this->imageWindow();
        } else if (imageTabPage_) {
            imageParent = imageTabPage_;
        } else {
            imageParent = this;
        }

        // 鍔犺浇杩囩▼鍙兘杈冮噸锛屾彁鍓嶅鐞嗕竴娆′簨浠讹紝閬垮厤涓荤獥鍙ｂ€滄秷澶扁€濋敊瑙?
        QApplication::processEvents(QEventLoop::AllEvents);

        m_imageHandler = std::make_unique<ImageHandler>(
            /* overview parent */ overviewParent,
            /* image parent */ imageParent,
            /* filename */ m_filename.c_str(),
            /* ROI set */ regionsSet,
            /* renderer */ this->getRenderer());
    }
    catch (...) {
        m_imageHandler.reset();
    }

    // 楠岃瘉鍔犺浇鏄惁鎴愬姛
    if (!m_imageHandler || m_imageHandler->get_image_properties() == nullptr) {
        if (pw) pw->end();
        QMessageBox::critical(this, "Parbat3D Error", "Failed to open image file. Please check GDAL support and file path.");
        return;
    }

    // 鏇存柊绐楀彛鏍囬骞舵樉绀虹浉鍏崇獥鍙?
    if (this->imageWindow()) {
        this->imageWindow()->setWindowTitle(QStringLiteral("Image Window - ") + QFileInfo(fileName).fileName());
    }
    // ImageWindow 宸插祵鍏ヤ腑澶爣绛鹃〉锛屾棤闇€鍗曠嫭鏄剧ず椤跺眰绐楀彛
    if (this->overviewWindow()) this->overviewWindow()->show();
    if (this->toolWindow()) {
        this->toolWindow()->show();
        // 打开影像后刷新 DisplayTab 的波段下拉框
        this->toolWindow()->refreshBands();
    }
    QApplication::processEvents(QEventLoop::AllEvents);
    // ROI 闈炴ā鎬佸璇濇榛樿闅愯棌锛屾寜闇€閫氳繃鑿滃崟鏄剧ず
    if (this->roiWindow()) this->roiWindow()->hide();

    // 缁撴潫杩涘害骞惰Е鍙戦娆￠噸缁?
    if (pw) pw->end();
    // 纭繚绐楀彛鍜孏L瑙嗗浘鍦ㄩ娆＄粯鍒跺墠鏀跺埌涓€娆″竷灞€涓庣粯鍒朵簨浠?
    QApplication::processEvents(QEventLoop::AllEvents);
    // 鍒濇璁剧疆 GL 瑙嗗浘绐楀彛灏哄锛岄伩鍏嶉娆＄粯鍒朵负闆跺昂瀵稿鑷撮粦灞?
    m_imageHandler->resize_image_window();
    m_imageHandler->redraw();

    // 鍐嶆纭繚绐楀彛澶勪簬婵€娲荤姸鎬佸苟寮哄埗鍒锋柊锛岄伩鍏嶅嚭鐜扮櫧灞忔垨鑿滃崟鏍忎笉缁樺埗
    ensureVisibleAndActive();
    forceRefreshUI();

    // 鏇存柊涓昏彍鍗曚腑涓庢枃浠?绐楀彛鐩稿叧鐨勫姩浣滅姸鎬?
    if (closeAction_) closeAction_->setEnabled(true);
    if (imageWindowAction_) imageWindowAction_->setEnabled(true);
    if (toolsWindowAction_) toolsWindowAction_->setEnabled(true);
    if (roiWindowAction_) roiWindowAction_->setEnabled(true);
    if (contrastWindowAction_) contrastWindowAction_->setEnabled(true);
    if (contrastAdvWindowAction_) contrastAdvWindowAction_->setEnabled(true);
    if (zoomInAction_) zoomInAction_->setEnabled(true);
    if (zoomOutAction_) zoomOutAction_->setEnabled(true);
    if (fitToWindowAction_) fitToWindowAction_->setEnabled(true);
}

void MainWindow::loadFile() {
    // 鍏煎鏃у叆鍙ｏ細寮圭獥閫夋嫨锛屽啀璋冪敤閲嶈浇
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Please choose a file",
        QString(),
        "All Supported Images (*.ecw *.jpg *.tif *.j2k *.jp2);;ERMapper Compressed Wavelets (*.ecw);;JPEG (*.jpg);;JPEG 2000 (*.j2k *.jp2);;TIFF / GeoTIFF (*.tif);;All Files (*.*)"
    );
    if (fileName.isEmpty()) return;
    loadFile(fileName);
}

void MainWindow::closeFile() {
    // deallocate variables

    m_filename.clear();

    // 使用类成员持有 ImageHandler，直接重置释放
    m_imageHandler.reset();

    // destroy tool window
    // if (toolWindow.GetHandle()!=NULL)
    //     toolWindow.Destroy();

    // hide image window
    // if (imageWindow.GetHandle()!=NULL)
    //     imageWindow.Hide();

    // destroy the roi window
    // if (roiWindow.GetHandle()!=NULL) {
    //     roiWindow.Destroy();       // roiWindow.Hide(); seems to cause the window to alternate save positions
    //     roiWindow.deleteAllROI();  // remove all the ROI from the list
    // }

    // destroy contrast window
    // if (contrastWindow.GetHandle()!=NULL) {
    //	contrastWindow.Destroy();
    // }


    // disable menu items
    // EnableMenuItem(overviewWindow.hMainMenu,IDM_IMAGEWINDOW,true);
    // EnableMenuItem(overviewWindow.hMainMenu,IDM_TOOLSWINDOW,true);
    // EnableMenuItem(overviewWindow.hMainMenu,IDM_ROIWINDOW,true);
    // EnableMenuItem(overviewWindow.hMainMenu,IDM_FILECLOSE,true);    
    // EnableMenuItem(overviewWindow.hMainMenu,IDM_CONTSWINDOW,true);    


    // repaint main window
    // overviewWindow.overviewWindowDisplay.Repaint();

    Console::write("closeFile() done\n");
}

void MainWindow::ensureVisibleAndActive()
{
    // 鑻ョ獥鍙ｈ鏈€灏忓寲鎴栨湭婵€娲伙紝鏄惧紡鎭㈠骞剁疆鍓?
    if (isMinimized()) showNormal();
    if (!isVisible()) show();
    raise();
    activateWindow();
    // 绔嬪嵆澶勭悊涓€娆′簨浠堕槦鍒楋紝纭繚鐘舵€佺敓鏁?
    QApplication::processEvents(QEventLoop::AllEvents);
}

void MainWindow::forceRefreshUI()
{
    // 瀵瑰叧閿尯鍩熻Е鍙戝埛鏂颁互閬垮厤缁勫悎鍣ㄦ湭鏇存柊瀵艰嚧鐨勭櫧灞?
    if (menuBar()) menuBar()->update();
    if (statusBar()) statusBar()->update();
    if (centralWidget()) centralWidget()->update();
    if (centralWidget()) centralWidget()->repaint();
    // 鍐嶅鐞嗕簨浠讹紝鎺ㄥ姩缁樺埗鎻愪氦
    QApplication::processEvents(QEventLoop::AllEvents);
}

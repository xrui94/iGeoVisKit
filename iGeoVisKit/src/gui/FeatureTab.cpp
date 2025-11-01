#include "PchApp.h"
#include "FeatureTab.h"

#include "config.h"
#include "ToolWindow.h"
#include "imagery/ROISet.h"

// Qt includes
#include <QApplication>
#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

extern ROISet *regionsSet;

int FeatureTab::GetContainerHeight()
{
    // 在Qt实现中，容器高度由布局自动管理，这里返回0即可
    return 0;
}

int FeatureTab::Create(QWidget *parent, QRect *parentRect)
{
    Q_UNUSED(parent);
    Q_UNUSED(parentRect);
    
    // ToolTab::Create(parent,parentRect);
    // prevProc=SetWindowProcedure(&WindowProcedure);
    
    setupUI();
    
    return 1;
}

void FeatureTab::setupUI()
{
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 创建粒度控制组
    QGroupBox *granularityGroup = new QGroupBox(QStringLiteral("Granularity"), this);
    QVBoxLayout *granularityLayout = new QVBoxLayout(granularityGroup);
    
    htitle = new QLabel(QStringLiteral("Granularity        No. Points: 0"), this);
    granularityLayout->addWidget(htitle);
    
    hTrackbar = new QSlider(Qt::Horizontal, this);
    hTrackbar->setRange(1, 10);
    hTrackbar->setTickPosition(QSlider::TicksBelow);
    hTrackbar->setTickInterval(1);
    hTrackbar->setPageStep(1);
    hTrackbar->setValue(1);
    granularityLayout->addWidget(hTrackbar);
    
    mainLayout->addWidget(granularityGroup);
    
    // 创建颜色通道组框
    QGroupBox *xGroup = new QGroupBox(QStringLiteral("X"), this);
    QGroupBox *yGroup = new QGroupBox(QStringLiteral("Y"), this);
    QGroupBox *zGroup = new QGroupBox(QStringLiteral("Z"), this);
    
    // 创建单选按钮
    int bands = 0;
    ToolWindow* tw = qobject_cast<ToolWindow*>(parent());
    if (tw) bands = tw->bands;
    for (int i = 0; i < bands; i++) {
        xRadiobuttons[i] = new QRadioButton(this);
        yRadiobuttons[i] = new QRadioButton(this);
        zRadiobuttons[i] = new QRadioButton(this);
        
        QString bandName;
        if (i > 0) {
            bandName = QStringLiteral("Band %1").arg(i);
        } else {
            bandName = QStringLiteral("NONE");
        }
        
        QLabel *bandLabel = new QLabel(bandName, this);
        
        // 连接信号槽
        connect(xRadiobuttons[i], &QRadioButton::toggled, this, &FeatureTab::onXRadioToggled);
        connect(yRadiobuttons[i], &QRadioButton::toggled, this, &FeatureTab::onYRadioToggled);
        connect(zRadiobuttons[i], &QRadioButton::toggled, this, &FeatureTab::onZRadioToggled);
    }
    
    hROIOnly = new QCheckBox(QStringLiteral("ROIs Only"), this);
    
    hGenerate = new QPushButton(QStringLiteral("Generate"), this);
    connect(hGenerate, &QPushButton::clicked, this, &FeatureTab::onGenerateClicked);
    
    mainLayout->addWidget(xGroup);
    mainLayout->addWidget(yGroup);
    mainLayout->addWidget(zGroup);
    mainLayout->addWidget(hROIOnly);
    mainLayout->addWidget(hGenerate);
    
    // 连接滑块信号
    connect(hTrackbar, &QSlider::valueChanged, this, &FeatureTab::onSliderValueChanged);
}

void FeatureTab::onSliderValueChanged(int value)
{
    Q_UNUSED(value);
    // 处理滑块值变化
    const long points = guessPoints(value);
    // Add points to title
    QString titleAndPoints = QStringLiteral("Granularity        No. Points: %1").arg(points);
    htitle->setText(titleAndPoints);
}

void FeatureTab::onXRadioToggled(bool checked)
{
    Q_UNUSED(checked);
    // 处理X单选按钮切换
}

void FeatureTab::onYRadioToggled(bool checked)
{
    Q_UNUSED(checked);
    // 处理Y单选按钮切换
}

void FeatureTab::onZRadioToggled(bool checked)
{
    Q_UNUSED(checked);
    // 处理Z单选按钮切换
}

void FeatureTab::onGenerateClicked()
{
    // 处理生成按钮点击
    // find out which bands are selected
    int x = 0, y = 0, z = 0;
    // for (int i=0; i<toolWindow->bands; i++)
    // {
    //     if (xRadiobuttons[i]->isChecked())
    //         x = i;
    //     
    //     if (yRadiobuttons[i]->isChecked())
    //         y = i;
    //     
    //     if (zRadiobuttons[i]->isChecked())
    //         z = i;
    // }
    // 
    // bool rois_only_state = hROIOnly->isChecked();
    // 
    // int GranState = hTrackbar->value();
    // OnGenerateClicked(GranState, rois_only_state, x, y, z);
}

int FeatureTab::guessPoints(int slider_pos)
{
    // ImageProperties* im_prop = image_handler->get_image_properties();
    int my_guess, granularity = 1;
    
    // Console::write("guessPoints - slider_pos = %d, ",slider_pos);
    slider_pos--;
    while (slider_pos) {
        granularity *= 4;
        slider_pos--;
    }

    // Console::write("granularity = %d, width = %d, height = %d\n", granularity, im_prop->getWidth(), im_prop->getHeight());
    // my_guess = im_prop->getWidth() * im_prop->getHeight() / granularity;
    my_guess = 1000; // 临时值
    return my_guess;
}

void FeatureTab::OnUserMessage()
{
    // 处理用户消息
}
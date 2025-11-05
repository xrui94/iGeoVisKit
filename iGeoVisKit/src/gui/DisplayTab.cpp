#include "PchApp.h"
#include "DisplayTab.h"

#include "config.h"
#include "ScrollBox.h"
#include "ToolTab.h"
#include "ToolWindow.h"
#include "MainWindow.h"
#include "imagery/ImageHandler.h"
#include "opengl/ImageViewport.h"
#include "gui/ImageWindow.h"

extern MainWindow* mainWindow;

// Qt includes
#include <QApplication>
#include <QWidget>
#include <QGroupBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QSizePolicy>
#include <algorithm>

int DisplayTab::GetContainerHeight()
{
	return 0;
}

int DisplayTab::Create(QWidget* parent, QRect* parentRect)
{
	Q_UNUSED(parent);
	Q_UNUSED(parentRect);
	return 1;
}

void DisplayTab::setupUI()
{
	// 先调用基类以创建标题等
	ToolTab::setupUI();

	QVBoxLayout* outer = qobject_cast<QVBoxLayout*>(layout());
	if (!outer) {
		outer = new QVBoxLayout(this);
	}

	// “Channel Selection” GroupBox（不铺满 Display Tab） - 扁平化视觉并移除组内标题
	QGroupBox* groupBox = new QGroupBox(QString(), this);
	groupBox->setFlat(true);
	groupBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	groupBox->setMaximumWidth(220);
	QVBoxLayout* groupLayout = new QVBoxLayout(groupBox);
	groupLayout->setContentsMargins(8, 8, 8, 8);
	groupLayout->setSpacing(6);

	// 纵向排列：R/G/B 标签 + 对应下拉框
	comboR = new QComboBox(groupBox);
	comboG = new QComboBox(groupBox);
	comboB = new QComboBox(groupBox);

	groupLayout->addWidget(new QLabel("R"));
	groupLayout->addWidget(comboR);
	groupLayout->addSpacing(8);
	groupLayout->addWidget(new QLabel("G"));
	groupLayout->addWidget(comboG);
	groupLayout->addSpacing(8);
	groupLayout->addWidget(new QLabel("B"));
	groupLayout->addWidget(comboB);

	// 拉伸模式选择
	groupLayout->addSpacing(10);
	groupLayout->addWidget(new QLabel(QStringLiteral("拉伸模式")));
	comboStretch = new QComboBox(groupBox);
	comboStretch->addItem(QStringLiteral("不拉伸"), (int)ImageGL::StretchNone);
	comboStretch->addItem(QStringLiteral("2–98%"), (int)ImageGL::StretchPercentile2_98);
	comboStretch->addItem(QStringLiteral("1–99%"), (int)ImageGL::StretchPercentile1_99);
	comboStretch->addItem(QStringLiteral("5–95%"), (int)ImageGL::StretchPercentile5_95);
	groupLayout->addWidget(comboStretch);

	// 初次填充（依据 ToolWindow.bands）
	int bands = 0;
	if (toolWindow()) bands = toolWindow()->bands;
	if (bands <= 0) bands = 1;
	refreshBands(bands);

	updateButton = new QPushButton(QStringLiteral("Update"), this);
	connect(updateButton, &QPushButton::clicked, this, &DisplayTab::onUpdateClicked);
	groupLayout->addSpacing(10);
	groupLayout->addWidget(updateButton);

	// 添加到外层并保留空间用于后续“Curves”组
	outer->addWidget(groupBox, 0, Qt::AlignTop | Qt::AlignLeft);
	outer->addStretch(1);
}

void DisplayTab::onUpdateClicked()
{
	int r = comboR ? comboR->currentData().toInt() : 0;
	int g = comboG ? comboG->currentData().toInt() : 0;
	int b = comboB ? comboB->currentData().toInt() : 0;
	int stretchVal = comboStretch ? comboStretch->currentData().toInt() : (int)ImageGL::StretchPercentile2_98;

	if (mainWindow && mainWindow->imageHandler() && mainWindow->imageHandler()->get_image_viewport()) {
		mainWindow->imageHandler()->get_image_viewport()->set_display_bands(r, g, b);
		// 同步拉伸模式
		mainWindow->imageWindow()->setStretchMode(static_cast<ImageGL::StretchMode>(stretchVal));
	}
}

void DisplayTab::refreshBands(int totalBandsIncludingNone)
{
	if (!comboR || !comboG || !comboB) return;

	// 清空并重建条目
	comboR->clear();
	comboG->clear();
	comboB->clear();

	const int realBands = std::max(0, totalBandsIncludingNone - 1);
	comboR->addItem(QStringLiteral("NONE"), 0);
	comboG->addItem(QStringLiteral("NONE"), 0);
	comboB->addItem(QStringLiteral("NONE"), 0);
	for (int i = 1; i <= realBands; ++i) {
		const QString name = QStringLiteral("Band %1").arg(i);
		comboR->addItem(name, i);
		comboG->addItem(name, i);
		comboB->addItem(name, i);
	}

	// 默认值：优先视口当前值，否则 1/2/3
	int defR = 1, defG = 2, defB = 3;
	if (mainWindow && mainWindow->imageHandler() && mainWindow->imageHandler()->get_image_viewport()) {
		int r = defR, g = defG, b = defB;
		mainWindow->imageHandler()->get_image_viewport()->get_display_bands(&r, &g, &b);
		defR = r; defG = g; defB = b;
	}
	auto setByVal = [](QComboBox* cb, int v) { int idx = cb->findData(v); if (idx >= 0) cb->setCurrentIndex(idx); };
	setByVal(comboR, std::min(defR, std::max(0, realBands)));
	setByVal(comboG, std::min(defG, std::max(0, realBands)));
	setByVal(comboB, std::min(defB, std::max(0, realBands)));

	// 启用/禁用：1->禁用G/B；2->禁用B；>=3全部可用
	comboR->setEnabled(realBands >= 1);
	comboG->setEnabled(realBands >= 2);
	comboB->setEnabled(realBands >= 3);
}

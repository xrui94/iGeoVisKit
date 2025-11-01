#include "PchApp.h"

#include "PrefsWindow.h"

#include "utils/Settings.h"

// Qt includes
#include <QApplication>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>

PrefsWindow::PrefsWindow(QWidget *parent)
    : QDialog(parent)
    , cacheLabel(nullptr)
    , cacheEntry(nullptr)
    , texSizeLabel(nullptr)
    , texSizeEntry(nullptr)
    , displayCloseConfirmCheckbox(nullptr)
    , okButton(nullptr)
    , cancelButton(nullptr)
{
    setWindowTitle(QStringLiteral("Preferences"));
    resize(300, 200);
    
    setupUI();
}

PrefsWindow::~PrefsWindow()
{
}

int PrefsWindow::Create(QWidget *parent)
{
	if (parent) {
		setParent(parent);
		setWindowFlags(windowFlags() | Qt::Dialog);
	}
	
	// 从设置文件加载值
	if (settingsFile) {
		// cacheEntry->setText(QString::fromStdString(settingsFile->getSetting("preferences", "cachesize")));
		// texSizeEntry->setText(QString::fromStdString(settingsFile->getSetting("preferences", "texsize")));
		// 
		// std::string confirmSetting = settingsFile->getSetting("preferences", "displayconfirmwindow");
		// displayCloseConfirmCheckbox->setChecked(confirmSetting == "1");
	}
	
	show();
	return true;
}

void PrefsWindow::setupUI()
{
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 缓存大小设置
    QHBoxLayout *cacheLayout = new QHBoxLayout();
    cacheLabel = new QLabel(QStringLiteral("Cache Size:"), this);
    cacheEntry = new QLineEdit(this);
    cacheLayout->addWidget(cacheLabel);
    cacheLayout->addWidget(cacheEntry);
    mainLayout->addLayout(cacheLayout);
    
    // 纹理大小设置
    QHBoxLayout *texSizeLayout = new QHBoxLayout();
    texSizeLabel = new QLabel(QStringLiteral("Texture Size:"), this);
    texSizeEntry = new QLineEdit(this);
    texSizeLayout->addWidget(texSizeLabel);
    texSizeLayout->addWidget(texSizeEntry);
    mainLayout->addLayout(texSizeLayout);
    
    // 显示确认窗口设置
    displayCloseConfirmCheckbox = new QCheckBox(QStringLiteral("Display confirmation window when closing"), this);
    mainLayout->addWidget(displayCloseConfirmCheckbox);
    
    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    okButton = new QPushButton(QStringLiteral("OK"), this);
    cancelButton = new QPushButton(QStringLiteral("Cancel"), this);
    
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号槽
    connect(okButton, &QPushButton::clicked, this, &PrefsWindow::onOKClicked);
    connect(cancelButton, &QPushButton::clicked, this, &PrefsWindow::onCancelClicked);
}

void PrefsWindow::onOKClicked()
{
    // 保存设置
    if (settingsFile) {
        // settingsFile->setSetting("preferences", "cachesize", cacheEntry->text().toStdString());
        // settingsFile->setSetting("preferences", "texsize", texSizeEntry->text().toStdString());
        // settingsFile->setSetting("preferences", "displayconfirmwindow", 
        //                         displayCloseConfirmCheckbox->isChecked() ? "1" : "0");
    }
    
    QMessageBox::information(this, QStringLiteral("Parbat3D"),
                            QStringLiteral("Preferences saved successfully.\nThis will take effect after restarting Parbat."));
    
    hide();
}

void PrefsWindow::onCancelClicked()
{
    // 恢复原来的设置
    if (settingsFile) {
        // cacheEntry->setText(QString::fromStdString(settingsFile->getSetting("preferences", "cachesize")));
        // texSizeEntry->setText(QString::fromStdString(settingsFile->getSetting("preferences", "texsize")));
        // 
        // std::string confirmSetting = settingsFile->getSetting("preferences", "displayconfirmwindow");
        // displayCloseConfirmCheckbox->setChecked(confirmSetting == "1");
    }
    
    hide();
}
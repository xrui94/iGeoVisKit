#ifndef _PARBAT_PREFSWINDOW_H
#define _PARBAT_PREFSWINDOW_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>

class PrefsWindow : public QDialog
{
    Q_OBJECT

public:
    PrefsWindow(QWidget *parent = nullptr);
    virtual ~PrefsWindow();

    int Create(QWidget *parent = nullptr);

private slots:
    void onOKClicked();
    void onCancelClicked();

private:
    void setupUI();
    
    QLabel *cacheLabel;
    QLineEdit *cacheEntry;
    QLabel *texSizeLabel;
    QLineEdit *texSizeEntry;
    QLabel *displayCloseConfirmLabel;
    QCheckBox *displayCloseConfirmCheckbox;
    QPushButton *okButton;
    QPushButton *cancelButton;
};

#endif
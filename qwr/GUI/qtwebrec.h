/****************************************************************************
** Form interface generated from reading ui file 'qtwebrec.ui'
**
** Created: So Apr 2 17:58:13 2006
**      by: The User Interface Compiler ($Id: qtwebrec.h,v 1.1 2006/08/09 12:27:54 yorn Exp $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef QTWEBREC_H
#define QTWEBREC_H

#include <qvariant.h>
#include <qpixmap.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class videoWidget;
class QLabel;
class QComboBox;
class QPushButton;
class QRadioButton;
class QLineEdit;

class qtWebRec : public QWidget
{
    Q_OBJECT

public:
    qtWebRec( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~qtWebRec();

    videoWidget* videoWidget1;
    QLabel* PluginLabel;
    QComboBox* AudCodecCombo;
    QComboBox* vidCodecCombo;
    QLabel* textLabel2_2;
    QPushButton* FileButton;
    QRadioButton* preview;
    QPushButton* playButton;
    QLabel* textLabel1;
    QLineEdit* fileNameDisplay;
    QLabel* textLabel1_2;
    QPushButton* recButton;
    QLineEdit* streamurl;
    QPushButton* streamButton;

public slots:
    virtual void play(bool);
    virtual void record(bool);
    virtual void selectFile(const QString&);
    virtual void videoSelect(int);
    virtual void togglePreview(bool);
    virtual void audioSelect(int);
    virtual void stream(bool);
    virtual void selectStream(const QString&);

protected:

protected slots:
    virtual void languageChange();

private:
    QPixmap image0;

};

#endif // QTWEBREC_H

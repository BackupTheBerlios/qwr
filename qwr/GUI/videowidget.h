/*
 * abstract video widget class
 * Copyright (C) 2005-2006 Sulejman Mundzic 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#ifndef VIDEO_WIDGET_H
#define VIDEO_WIDGET_H

#include <qwidget.h>
#include <qsocketnotifier.h>
#include "qDescConnection.h"

extern "C"
{
#include <sys/shm.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
}
//#include <X11/Intrinsic.h>
#include <X11/extensions/Xvlib.h>

#include"pixmapFormat.h"
//class VideoLayer;


class videoWidget : public QWidget
{
    Q_OBJECT
public:
    videoWidget( QWidget *parent=0, const char *name=0 );
    virtual ~videoWidget(){}

    void setConnector(QDescConnection* connection);

public slots:
    void videoDevMessageReader(int);

protected:
    virtual void resizeEvent( QResizeEvent * );
    virtual void paintEvent( QPaintEvent * );
    QSizePolicy sizePolicy() const;

private:
    QSocketNotifier* videoNotifier;

    void    visualPropertiesIdentify();
    void    findPixmapFormat();
    void    initXvImage();
    XImage* createX11Image(int width, int height);
    void    destroyX11image();
    void    putFrame();

    void    ratio_fixup(int *width, int *height, int *xoff, int *yoff);

    QDescConnection* connection;

    // structurs required for image displaying
    Display*          _dpy;
    unsigned int      _win;
    GC                _gc;
    XVisualInfo*      _vinfo;
    XShmSegmentInfo*  _shm;

    int               _winWidth;
    int               _winHeight;
    int               _wx;
    int               _wy;
    int               _ww;
    int               _wh;
    int               _ratio_x;
    int               _ratio_y;

    // ximage structures
    bool                       _no_mitshm;
    XImage*                    _ximage;
    unsigned int               _imgPixelSize;
    PixmapFormat               _imgFormat;

    /*
    bool              _status;
    int               _format;
    */
};

#endif // VIDEO_WIDGET_H

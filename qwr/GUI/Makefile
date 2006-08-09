SRC=
OBJ= event.o connection.o timev.o scheduler.o simpleConnection.o \
     qThreadConnection.o qDescConnection.o task.o \
     pixmapFormat.o videoCapture.o v4l2Capture.o v4lCapture.o webrecBackend.o \
     videoDisplayMsg.o jackAudioDevice.o theoraCodec.o convert.o codecInterface.o ringbuffer.o
STATLIBS= ../ext/ffmpeg/libavformat/libavformat.a ../ext/ffmpeg/libavcodec/libavcodec.a  ../ext/ffmpeg/libavutil/libavutil.a 
QTSRC = videowidget.cpp webrec.cpp moc_videowidget.cpp moc_webrec.cpp 
QTOBJ = qTask.o qDescConnection.o qThreadConnection.o

UIC_FORMS = qtwebrec.ui 	

UIC_SRC = $(patsubst %.ui,%.cpp,$(UIC_FORMS)) $(patsubst %.ui, moc_%.cpp, $(UIC_FORMS))
HDR = $(patsubst %.ui, %.h, $(UIC_FORMS))
QTSRC1 = $(UIC_SRC) $(SRC) $(QTSRC)
QTOBJ1 = $(patsubst %.cpp, %.o, $(notdir $(QTSRC1)))

CXX=g++
UIC = /usr/lib/qt3/bin/uic
MOC = /usr/lib/qt3/bin/moc

FLAGS= -O0 -g -Wall -pthread -I/usr/local/include -I/usr/include/cc++2 -I../esystem/ -I../backend -I/usr/lib/qt3/include \
       -I../ext/ffmpeg/libavcodec -I../ext/ffmpeg/libavutil -I../ext/ffmpeg/libavformat -DQT_THREAD_SUPPORT \
       -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE -DWITH_QT 
LDFLAGS = -L/usr/local/lib -L/usr/lib -ldl -ljack -logg -ltheora -lvorbis -lvorbisenc -lvorbisfile
VPATH=../esystem ../backend

STRIP=strip
INSTALL=install

all: QTWebRec

%.o:%.cpp
	$(CXX) $(FLAGS) $< -c

%.h: %.ui
	$(UIC) $< -o $@

moc_%.cpp: %.h
	$(MOC) $< -o $@

%.cpp: %.ui %.h
	$(UIC) -impl $*.h $< -o $@

QTWebRec: $(QTOBJ1) $(OBJ) $(QTOBJ) qtwebrecMain.cpp 
	$(CXX) $(FLAGS) $(LDFLAGS) $^ -L/usr/lib/qt3/lib -o $@ $(STATLIBS) -lqt-mt -ljpeg -lm -L/usr/X11R6/lib -lXv -lXt -lX11

clean:
	@rm -f *.o *~ *.d QTWebRec

.PHONY: all QTWebRec clean

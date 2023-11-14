
QT += widgets

TEMPLATE = app
TARGET = rrcmp

# You can make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# Please consult the documentation of the deprecated API in order to know
# how to port your code away from it.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += .
QT += core gui

win32 {
	QT += printsupport
        include( .\QWT_win.pri )
        CONFIG+= c++11 gui console
#        CONFIG+= c++11 gui
        LIBS += -L"C:/opencv/x64/mingw/bin" -lopencv_core455 -lopencv_highgui455 -lopencv_imgcodecs455 -lopencv_imgproc455
#        LIBS += -L"C:/opencv/build/x64/vc16/bin" -lopencv_world481 -lopencv_world481d -lopencv_videoio_ffmpeg481_64 -lopencv_videoio_msmf481_64 -lopencv_videoio_msmf481_64d
#       INCLUDEPATH += C:\opencv\build\include\opencv2 C:\opencv\build\include
        INCLUDEPATH += C:\opencv\include C:\opencv\include\opencv2
        target.path=.\
	INSTALLS += target
}

unix {
	greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
	CONFIG += c++17

	# Default rules for deployment.
	qnx: target.path = /tmp/$${TARGET}/bin
	else: unix:!android: target.path = /opt/$${TARGET}/bin
	!isEmpty(target.path): INSTALLS += target
	
	TRANSLATIONS += \
		RRcomparator_en_GB.ts
	CONFIG += lrelease
	CONFIG += embed_translations

        CONFIG += link_pkgconfig
	PKGCONFIG += opencv4
}

# Input
HEADERS += rrcmp.h 
FORMS += rrcmp.ui
SOURCES += rrcmp.cpp main.cpp 



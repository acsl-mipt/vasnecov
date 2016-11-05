#-------------------------------------------------
#
# Project created by QtCreator 2012-02-15T11:50:47
#
#-------------------------------------------------

QT       += opengl

TARGET = Vasnecov
TEMPLATE = lib
CONFIG += staticlib

LIBS += -lGLU

OBJECTS_DIR = obj
MOC_DIR = moc

QMAKE_CXXFLAGS += -Wall -Wextra -Woverloaded-virtual -Weffc++

HEADERS += \
    version.h \
    configuration.h \
    types.h \
	vasnecov.h \
    vasnecovworld.h \
    vasnecovuniverse.h \
    vasnecovscene.h \
    vasnecovmesh.h \
    vasnecovfigure.h \
    vasnecovlabel.h \
    vasnecovlamp.h \
    vasnecovproduct.h \
    vasnecovelement.h \
    vasnecovmaterial.h \
    vasnecovmatrix4x4.h \
    vasnecovpipeline.h \
    technologist.h \
    vasnecovtexture.h \
    elementlist.h \
    coreobject.h

SOURCES += \
	vasnecov.cpp \
    vasnecovworld.cpp \
    vasnecovuniverse.cpp \
    vasnecovscene.cpp \
    vasnecovmesh.cpp \
    vasnecovfigure.cpp \
    vasnecovlabel.cpp \
    vasnecovlamp.cpp \
    vasnecovproduct.cpp \
    vasnecovelement.cpp \
    vasnecovmaterial.cpp \
    vasnecovmatrix4x4.cpp \
    vasnecovpipeline.cpp \
    vasnecovtexture.cpp \
    technologist.cpp

target.path = /usr/local/lib

inc_target.path = /usr/local/include
inc_target.files = $$TARGET

incd_target.path = /usr/local/include/lib$$TARGET
incd_target.files += $$HEADERS

#share_target.path = /usr/local/share/lib$$TARGET
#share_target.files += share/loading.png

INSTALLS += target \
			inc_target \
            incd_target

RESOURCES += \
    resources.qrc

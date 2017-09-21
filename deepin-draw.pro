QT += core gui  printsupport svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

  qtHaveModule(opengl): QT += opengl

CONFIG += c++11 link_pkgconfig
PKGCONFIG += dtkwidget libexif
LIBS += -lfreeimage

TARGET = deepin-draw
TEMPLATE = app

isEmpty(PREFIX){
    PREFIX = /usr
}

HEADERS += \
    application.h

SOURCES += main.cpp \
    application.cpp

include(utils/utils.pri)
include(controller/controller.pri)
include(widgets/widgets.pri)
include(drawshape/drawshape.pri)
include(frame/frame.pri)

RESOURCES +=

BINDIR = $$PREFIX/bin
APPSHAREDIR = $$PREFIX/share/deepin-draw
MANDIR = $$PREFIX/share/dman/deepin-draw
MANICONDIR = $$PREFIX/share/icons/hicolor/scalable/apps
APPICONDIR = $$PREFIX/share/icons/deepin/apps/scalable

DEFINES += APPSHAREDIR=\\\"$$APPSHAREDIR\\\"

target.path = $$BINDIR

desktop.path = $${PREFIX}/share/applications/
desktop.files =  deepin-draw.desktop

manual.path = $$MANDIR
#manual.files = doc/*
manual_icon.path = $$MANICONDIR
manual_icon.files = logo/deepin-draw.svg

app_icon.path = $$APPICONDIR
app_icon.files = logo/deepin-draw.svg

# Automating generation .qm files from .ts files
#CONFIG(release, debug|release) {
#    !system($$PWD/generate_translations.sh): error("Failed to generate translation")
#}

#TRANSLATIONS += \
#   translations/deepin-draw.ts\
#    translations/deepin-draw_zh_CN.ts

#translations.path = $$APPSHAREDIR/translations
#translations.files = translations/*.qm
exists(app_icon.files) {
    message("app_icon exists")
}

INSTALLS = target desktop  app_icon manual_icon #manual   translations

DISTFILES += \
    logo/deepin-draw-16.svg \
    logo/deepin-draw-32.svg \
    logo/deepin-draw-64.svg \
    logo/deepin-draw-96.svg \
    logo/deepin-draw-128.svg \
    logo/deepin-draw-256.svg

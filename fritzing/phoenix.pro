# /*******************************************************************
#
# Part of the Fritzing project - http://fritzing.org
# Copyright (c) 2007-08 Fachhochschule Potsdam - http://fh-potsdam.de
#
# Fritzing is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Fritzing is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Fritzing.  If not, see <http://www.gnu.org/licenses/>.
#
# ********************************************************************
#
# $Revision$:
# $Author$:
# $Date$
#
#********************************************************************/

CONFIG += debug_and_release
win32::CONFIG -= embed_manifest_exe
HEADERS += \
	src/lib/quazip/crypt.h \
	src/lib/quazip/ioapi.h \
	src/lib/quazip/quazip.h \
	src/lib/quazip/quazipfile.h \
	src/lib/quazip/quazipfileinfo.h \
	src/lib/quazip/quazipnewinfo.h \
	src/lib/quazip/unzip.h \
	src/lib/quazip/zip.h \
    src/referencemodel/sqlitereferencemodel.h \
    src/referencemodel/referencemodel.h \
    src/referencemodel/daos.h \
    src/partsbinpalette/simpleeditablelabelwidget.h \
    src/partsbinpalette/partsbinpalettewidget.h \
    src/partsbinpalette/partsbinview.h \
    src/partsbinpalette/partsbinlistview.h \
    src/partsbinpalette/partsbiniconview.h \
    src/partsbinpalette/graphicsflowlayout.h \
    src/partsbinpalette/svgiconwidget.h \
    src/partseditor/editabledatewidget.h \
    src/partseditor/abstractconnectorinfowidget.h \
    src/partseditor/mismatchingconnectorwidget.h \
    src/partseditor/partseditorabstractviewimage.h \
    src/partseditor/partseditorconnectorviewimagewidget.h \
    src/partseditor/partseditorconnectoritem.h \
    src/partseditor/singleconnectorinfowidget.h \
    src/partseditor/partconnectorswidget.h \
    src/partseditor/connectorsviewswidget.h \
    src/partseditor/connectorsinfowidget.h \
    src/partseditor/hashpopulatewidget.h \
    src/partseditor/editablelinewidget.h \
    src/partseditor/editabletextwidget.h \
    src/partseditor/abstracteditablelabelwidget.h \
    src/partseditor/partseditormainwindow.h \
    src/partseditor/partsymbolswidget.h \
    src/partseditor/editablelabel.h \
    src/partseditor/partseditorviewimagewidget.h \
    src/partseditor/partspecificationswidget.h \
    src/aboutbox.h \
    src/autorouter1.h \
    src/bettertimer.h \
    src/bettertriggeraction.h \
    src/bus.h \
    src/busstuff.h \
    src/busconnectoritem.h \
    src/commands.h \
    src/connector.h \
    src/connectoritem.h \
    src/connectorstuff.h \
    src/connectorviewthing.h \
    src/console.h \
    src/debugdialog.h \
    src/eventeater.h \
    src/fdockwidget.h \
    src/filedealingwidget.h \
    src/fritzingwindow.h \
    src/fsplashscreen.h \
    src/ftabwidget.h \
    src/graphicssvglineitem.h \
    src/groupitem.h \
    src/htmlinfoview.h \
    src/infographicsview.h \
    src/itembase.h \
    src/itemdrag.h \
    src/layerattributes.h \
    src/layerkinpaletteitem.h \
    src/mainwindow.h \
    src/miniview.h \
    src/miniviewcontainer.h \
    src/misc.h \
    src/modelbase.h \
    src/modelpart.h \
    src/modelpartstuff.h \
    src/paletteitem.h \
    src/paletteitembase.h \
    src/palettemodel.h \
    src/partseditor/addremovelistwidget.h \
    src/partseditor/connectorswidget.h \
    src/partseditor/mainpartseditorwindow.h \
    src/partseditor/partinfowidget.h \
    src/partseditor/partseditorpaletteitem.h \
    src/partseditor/partseditorsketchwidget.h \
    src/partseditor/pcbxml.h \
    src/partseditor/svgdomdocument.h \
    src/partseditor/svgview.h \
    src/partinstancestuff.h \
    src/rendererviewthing.h \
    src/sketchareawidget.h \
    src/sketchmodel.h \
    src/sketchwidget.h \
    src/svgfilesplitter.h \
    src/version.h \
    src/viewgeometry.h \
    src/viewlayer.h \
    src/viewthing.h \
    src/virtualwire.h \
    src/waitpushundostack.h \
    src/wire.h \
    src/zoomcombobox.h
ICON = resources/images/fritzing_icon.icns
QT += core \
    gui \
    svg \
    xml \
    network \
    webkit \
    sql
RC_FILE = fritzing.rc
RESOURCES += phoenixresources.qrc
SOURCES += \
	src/lib/quazip/ioapi.c \
	src/lib/quazip/quazip.cpp \
	src/lib/quazip/quazipfile.cpp \
	src/lib/quazip/quazipnewinfo.cpp \
	src/lib/quazip/unzip.c \
	src/lib/quazip/zip.c \
    src/referencemodel/sqlitereferencemodel.cpp \
    src/partsbinpalette/simpleeditablelabelwidget.cpp \
    src/partsbinpalette/partsbinpalettewidget.cpp \
    src/partsbinpalette/partsbinview.cpp \
    src/partsbinpalette/partsbinlistview.cpp \
    src/partsbinpalette/partsbiniconview.cpp \
    src/partsbinpalette/graphicsflowlayout.cpp \
    src/partsbinpalette/svgiconwidget.cpp \
    src/partseditor/editabledatewidget.cpp \
    src/partseditor/abstractconnectorinfowidget.cpp \
    src/partseditor/mismatchingconnectorwidget.cpp \
    src/partseditor/partseditorabstractviewimage.cpp \
    src/partseditor/partseditorconnectorviewimagewidget.cpp \
    src/partseditor/partseditorconnectoritem.cpp \
    src/partseditor/singleconnectorinfowidget.cpp \
    src/partseditor/partconnectorswidget.cpp \
    src/partseditor/connectorsviewswidget.cpp \
    src/partseditor/connectorsinfowidget.cpp \
    src/partseditor/hashpopulatewidget.cpp \
    src/partseditor/editablelinewidget.cpp \
    src/partseditor/editabletextwidget.cpp \
    src/partseditor/abstracteditablelabelwidget.cpp \
    src/partseditor/partseditormainwindow.cpp \
    src/partseditor/partsymbolswidget.cpp \
    src/partseditor/editablelabel.cpp \
    src/partseditor/partseditorviewimagewidget.cpp \
    src/partseditor/partspecificationswidget.cpp \
    src/aboutbox.cpp \
    src/autorouter1.cpp \
    src/bettertriggeraction.cpp \
    src/bettertimer.cpp \
    src/bus.cpp \
    src/busstuff.cpp \
    src/busconnectoritem.cpp \
    src/commands.cpp \
    src/connector.cpp \
    src/connectoritem.cpp \
    src/connectorstuff.cpp \
    src/connectorviewthing.cpp \
    src/console.cpp \
    src/debugdialog.cpp \
    src/eventeater.cpp \
    src/fdockwidget.cpp \
    src/filedealingwidget.cpp \
    src/fritzingwindow.cpp \
    src/fsplashscreen.cpp \
    src/ftabwidget.cpp \
    src/graphicssvglineitem.cpp \
    src/groupitem.cpp \
    src/htmlinfoview.cpp \
    src/infographicsview.cpp \
    src/itembase.cpp \
    src/itemdrag.cpp \
    src/layerattributes.cpp \
    src/layerkinpaletteitem.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/mainwindow_menu.cpp \
    src/miniview.cpp \
    src/miniviewcontainer.cpp \
    src/misc.cpp \
    src/modelbase.cpp \
    src/modelpart.cpp \
    src/modelpartstuff.cpp \
    src/paletteitem.cpp \
    src/paletteitembase.cpp \
    src/palettemodel.cpp \
    src/partseditor/addremovelistwidget.cpp \
    src/partseditor/connectorswidget.cpp \
    src/partseditor/mainpartseditorwindow.cpp \
    src/partseditor/mainpartseditorwindow_menu.cpp \
    src/partseditor/partinfowidget.cpp \
    src/partseditor/partseditorpaletteitem.cpp \
    src/partseditor/partseditorsketchwidget.cpp \
    src/partseditor/pcbxml.cpp \
    src/partseditor/svgdomdocument.cpp \
    src/partseditor/svgview.cpp \
    src/partinstancestuff.cpp \
    src/rendererviewthing.cpp \
    src/sketchareawidget.cpp \
    src/sketchmodel.cpp \
    src/sketchwidget.cpp \
    src/svgfilesplitter.cpp \
    src/version.cpp \
    src/viewgeometry.cpp \
    src/viewlayer.cpp \
    src/virtualwire.cpp \
    src/wire.cpp \
    src/waitpushundostack.cpp \
    src/zoomcombobox.cpp
TARGET = Fritzing
TEMPLATE = app

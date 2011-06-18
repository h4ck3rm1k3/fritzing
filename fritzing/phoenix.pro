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
win32 {
# release build using msvc 2010 needs to use Multi-threaded (/MT) for the code generation/runtime library option
# release build using msvc 2010 needs to add msvcrt.lib;%(IgnoreSpecificDefaultLibraries) to the linker/no default libraries option
	CONFIG -= embed_manifest_exe
        INCLUDEPATH += $$[QT_INSTALL_PREFIX]/src/3rdparty/zlib
        #INCLUDEPATH += C:/QtSDK/QtSources/4.7.2/src/3rdparty/zlib
	DEFINES += _CRT_SECURE_NO_DEPRECATE
    LIBS += setupapi.lib
}
macx {
	MOC_DIR = build/moc
        CONFIG += x86 x86_64 # ppc x86_64
	QMAKE_INFO_PLIST = FritzingInfo.plist
	#DEFINES += QT_NO_DEBUG   		# uncomment this for xcode
        LIBS += /usr/lib/libz.dylib
        LIBS += /System/Library/Frameworks/CoreFoundation.framework/Versions/A/CoreFoundation
        LIBS += /System/Library/Frameworks/Carbon.framework/Carbon
        LIBS += /System/Library/Frameworks/IOKit.framework/Versions/A/IOKit
}
unix {
    !macx { # unix is defined on mac
        HARDWARE_PLATFORM = $$system(uname -m)
        contains( HARDWARE_PLATFORM, x86_64 ) {
            DEFINES += LINUX_64
        } else {
            DEFINES += LINUX_32
	}
	LIBS += -lz
    }
}
		
ICON = resources/images/fritzing_icon.icns
QT += core gui svg xml network sql #opengl

RC_FILE = fritzing.rc
RESOURCES += phoenixresources.qrc
	include(pri/kitchensink.pri)
	include(pri/quazip.pri)
	include(pri/partsbinpalette.pri)
	include(pri/partseditor.pri)
	include(pri/referencemodel.pri)
	include(pri/svg.pri)
	include(pri/help.pri)
	include(pri/itemselection.pri)
	include(pri/version.pri)
	include(pri/eagle.pri)
	include(pri/utils.pri)
	include(pri/viewswitcher.pri)
	include(pri/navigator.pri)
	include(pri/items.pri)
	include(pri/autoroute.pri)
	include(pri/dialogs.pri)
	include(pri/connectors.pri)
	include(pri/infoview.pri)
	include(pri/model.pri)
	include(pri/sketch.pri)
	include(pri/translations.pri)
	include(pri/program.pri)
	include(pri/qtlockedfile.pri)
	include(pri/ff.pri)	
	include(pri/qtsysteminfo.pri)
TARGET = Fritzing
TEMPLATE = app

	

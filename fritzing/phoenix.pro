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
	CONFIG -= embed_manifest_exe
	INCLUDEPATH += $$[QT_INSTALL_PREFIX]/src/3rdparty/zlib
	DEFINES += _CRT_SECURE_NO_DEPRECATE
}
macx {
	CONFIG += x86 ppc
	QMAKE_INFO_PLIST = FritzingInfo.plist
	DEFINES += QT_NO_DEBUG
}
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
include(kitchensink.pri)
include(quazip.pri)
include(partsbinpalette.pri)
include(partseditor.pri)
include(referencemodel.pri)
include(svg.pri)
include(help.pri)
include(labels.pri)
TARGET = Fritzing
TEMPLATE = app
TRANSLATIONS += translations/fritzing_de.ts \
	translations/fritzing_fr.ts \
	translations/fritzing_en.ts \
	translations/fritzing_es.ts

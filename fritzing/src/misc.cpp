/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-08 Fachhochschule Potsdam - http://fh-potsdam.de

Fritzing is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Fritzing is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Fritzing.  If not, see <http://www.gnu.org/licenses/>.

********************************************************************

$Revision$:
$Author$:
$Date$

********************************************************************/

// misc Fritzing utility functions
#include "misc.h"
#include "debugdialog.h"
#include <QDesktopServices>
#include <QCoreApplication>
#include <QDir>


QString & makeGrammaticalNumber(int count, QString & singular, QString & plural) {
	if (count == 1) return singular;

	return plural;
}

// finds the user parts folder based on local desktop (OS) defaults
QString getUserPartsFolder(){
	return QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/UserCreatedParts";
}

// finds a subfolder of the application directory searching backward up the tree
QDir * getApplicationSubFolder(QString search) {
	QString path = QCoreApplication::applicationDirPath();
    path += "/" + search;
	//DebugDialog::debug(QObject::tr("path %1").arg(path) );
    QDir* dir= new QDir(path);
    while (!dir->exists()) {
    	// if we're running from the debug or release folder, go up one to find things
    	dir->cdUp();
		dir->cdUp();
    	if (dir->isRoot()) return NULL;   // didn't find the parts folder

		dir->setPath(dir->absolutePath() + "/" + search);
   	}

   	return dir;
}

QString getApplicationSubFolderPath(QString search) {
	QDir * dir = getApplicationSubFolder(search);
	QString result = dir->path();
	delete dir;
	return result;
}

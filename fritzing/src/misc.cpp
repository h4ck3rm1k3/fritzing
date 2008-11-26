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


const QString& makeGrammaticalNumber(int count, const QString& singular, const QString& plural) {
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

qreal convertToInches(const QString & s, bool * ok) {
	QString string = s;
	qreal divisor = 1.0;
	if (string.endsWith("cm", Qt::CaseInsensitive)) {
		divisor = 2.54;
		string.chop(2);
	}
	else if (string.endsWith("mm", Qt::CaseInsensitive)) {
		divisor = 25.4;
		string.chop(2);
	}
	else if (string.endsWith("in", Qt::CaseInsensitive)) {
		divisor = 1.0;
		string.chop(2);
	}
	else {
		if (ok) *ok = false;
		return 0;
	}

	bool fine;
	qreal result = string.toDouble(&fine);
	if (!fine) {
		if (ok) *ok = false;
		return 0;
	}

	if (ok) *ok = true;
	return result / divisor;
}
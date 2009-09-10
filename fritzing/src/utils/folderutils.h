/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2009 Fachhochschule Potsdam - http://fh-potsdam.de

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

#ifndef FOLDERUTILS_H
#define FOLDERUTILS_H

#include <QString>
#include <QDir>
#include <QStringList>

#include "misc.h"

class FolderUtils
{

public:
	static QString getUserPartsFolder();
	static QDir *getApplicationSubFolder(QString);
	static QString getApplicationSubFolderPath(QString);
	static QString getUserDataStorePath(QString folder = ___emptyString___);
	static const QStringList & getUserDataStoreFolders();
	static bool createFolderAnCdIntoIt(QDir &dir, QString newFolder);
	static bool setApplicationPath(const QString & path);
	static const QString getLibraryPath();
	static void cleanup();
	static bool containsText(const QString &filepath, const QString &searchText);


protected:
	FolderUtils();
	~FolderUtils();
	const QStringList & userDataStoreFolders();
	bool setApplicationPath2(const QString & path);
	const QString applicationDirPath();
	const QString libraryPath();

protected:
	static FolderUtils* singleton;

protected:
	QStringList m_folders;
	QString m_appPath;
};

#endif

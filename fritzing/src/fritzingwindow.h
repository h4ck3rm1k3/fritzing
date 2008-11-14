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



#ifndef FRITZINGWINDOW_H_
#define FRITZINGWINDOW_H_

#include <QMainWindow>
#include <QDir>

#include "waitpushundostack.h"

#include "lib/quazip/quazip.h"
#include "lib/quazip/quazipfile.h"

class FritzingWindow : public QMainWindow {
	Q_OBJECT
	public:
		FritzingWindow(const QString &untitledFileName, int &untitledFileCount, QString fileExt, QWidget * parent = 0, Qt::WFlags f = 0);

	protected slots:
		bool save();
		virtual bool saveAs();
		void undoStackCleanChanged(bool isClean);

	protected:
		void setTitle();
		virtual const QString fileExtension() = 0;
		virtual const QString untitledFileName() = 0;
		virtual int &untitledFileCount() = 0;
		virtual const QString defaultSaveFolder() = 0;

		virtual void saveAsAux(const QString & fileName) = 0;
		bool beforeClosing(); // returns true if close, false if cancel

		bool createFolderAnCdIntoIt(QDir &dir, QString newFolder);
		void rmdir(const QString &dirPath);
		void rmdir(QDir & dir);
		bool createZipAndSaveTo(const QDir &dirToCompress, const QString &filename);
		bool unzipTo(const QString &filepath, const QString &dirToDecompress);

	protected:
		class WaitPushUndoStack * m_undoStack;
		QString m_fileName;

	public:
		static bool isEmptyFileName(const QString &filename, const QString &unsavedFilename);
		static bool alreadyHasExtension(const QString &fileName);
		static QString getExtFromFileDialog(const QString &extOpt);

	public:
		static QString QtFunkyPlaceholder;  // this is some weird hack Qt uses in window titles as a placeholder to set the modified state
		static const QString FritzingExtension;
		static const QString CoreBinLocation;
		static void replicateDir(QDir srcDir, QDir targDir);
		static QString getRandText();
};

#endif /* FRITZINGWINDOW_H_ */

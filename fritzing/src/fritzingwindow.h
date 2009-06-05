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



#ifndef FRITZINGWINDOW_H_
#define FRITZINGWINDOW_H_

#include <QMainWindow>
#include <QDir>
#include <QStatusBar>

#include "waitpushundostack.h"
#include "utils/misc.h"

class FritzingWindow : public QMainWindow {
	Q_OBJECT
	public:
		FritzingWindow(const QString &untitledFileName, int &untitledFileCount, QString fileExt, QWidget * parent = 0, Qt::WFlags f = 0);
		const QString &fileName() {
			return m_fileName;
		}

	signals:
		void readOnlyChanged(bool isReadOnly);

	protected slots:
		virtual bool save();
		virtual bool saveAs();
		void undoStackCleanChanged(bool isClean);

	protected:
		void setTitle();
		virtual const QString fritzingTitle();
		virtual const QString fileExtension() = 0;
		virtual const QString untitledFileName() = 0;
		virtual int &untitledFileCount() = 0;
		virtual const QString defaultSaveFolder() = 0;

		virtual void saveAsAux(const QString & fileName) = 0;
		bool beforeClosing(bool showCancel=true); // returns true if close, false if cancel

		void createCloseAction();

		void setReadOnly(bool readOnly);

	protected:
		class WaitPushUndoStack * m_undoStack;
		QString m_fileName;
		bool m_readOnly;
		QAction *m_closeAct;
		QDir m_tempDir;
		QStatusBar *m_statusBar;

	public:
		// TODO: these probably belong in some separate file i/o class
		static bool isEmptyFileName(const QString &filename, const QString &unsavedFilename);
		static bool alreadyHasExtension(const QString &fileName, const QString &extension=___emptyString___);
		static QString getExtFromFileDialog(const QString &extOpt);
		static void rmdir(const QString &dirPath);
		static void rmdir(QDir & dir);
		static bool createZipAndSaveTo(const QDir &dirToCompress, const QString &filename);
		static bool unzipTo(const QString &filepath, const QString &dirToDecompress);
		static void replicateDir(QDir srcDir, QDir targDir);
		static QString getRandText();

	public:
		virtual void notClosableForAWhile() {}

	public:
		static QString ReadOnlyPlaceholder;
		static const QString CoreBinLocation;
};

#endif /* FRITZINGWINDOW_H_ */

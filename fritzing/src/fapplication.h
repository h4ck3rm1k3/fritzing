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

#ifndef FAPPLICATION_H
#define FAPPLICATION_H

#include <QApplication>
#include <QTranslator>
#include <QPixmap>
#include <QFileDialog>

class FApplication :
	public QApplication
{
	Q_OBJECT

public:
	FApplication(int & argc, char ** argv);
	~FApplication(void);

public:
	int startup(int & argc, char ** argv);
	void finish();
	void preloadSlowParts();

public:
	static QString getOpenFileName( QWidget * parent = 0, const QString & caption = QString(), const QString & dir = QString(), const QString & filter = QString(), QString * selectedFilter = 0, QFileDialog::Options options = 0 );
	static QString getSaveFileName( QWidget * parent = 0, const QString & caption = QString(), const QString & dir = QString(), const QString & filter = QString(), QString * selectedFilter = 0, QFileDialog::Options options = 0 );
	static void setOpenSaveFolder(const QString& path);
	static const QString openSaveFolder();
	static bool spaceBarIsPressed();

signals:
	void spaceBarIsPressedSignal(bool);

public slots:
	void preferences();
	void checkForUpdates();
	void checkForUpdates(bool atUserRequest);
	void enableCheckUpdates(bool enabled);
	void createUserDataStoreFolderStructure();

protected:
    bool eventFilter(QObject *obj, QEvent *event);
	bool event(QEvent *event);
	bool findTranslator(const QString & translationsPath);
	void loadNew(QString path);
	void loadOne(class MainWindow *, QString path, int loaded);
	void initSplash(class FSplashScreen & splash, int & progressIndex, QPixmap & pixmap);

protected:
	static bool m_spaceBarIsPressed;
	static bool m_mousePressed;
	static QString m_openSaveFolder;
	static QTranslator m_translator;
	static class ReferenceModel * m_referenceModel;
	static class PaletteModel * m_paletteBinModel;
	static bool m_started;
	static QList<QString> m_filesToLoad;
	static QString m_libPath;
	static QString m_translationPath;
	static class UpdateDialog * m_updateDialog;


};


#endif

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
#include <QPointer>
#include <QWidget>
#include <QMutex>
#include <QMultiHash>
#include <QTimer>

class FApplication :
	public QApplication
{
	Q_OBJECT

public:
	FApplication(int & argc, char ** argv);
	~FApplication(void);

public:
	int startup(bool firstRun);
	void finish();
	void preloadSlowParts();

public:
	static bool spaceBarIsPressed();
	static bool runAsService();

signals:
	void spaceBarIsPressedSignal(bool);

public slots:
	void preferences();
	void checkForUpdates();
	void checkForUpdates(bool atUserRequest);
	void enableCheckUpdates(bool enabled);
	void createUserDataStoreFolderStructure();
	void changeActivation(bool activate, QWidget * originator);
	void updateActivation();
	void topLevelWidgetDestroyed(QObject *);
	void closeAllWindows2();
	void loadedPart(int loaded, int total);
	void externalProcessSlot(QString & name, QString & path, QStringList & args);

protected:
    bool eventFilter(QObject *obj, QEvent *event);
	bool event(QEvent *event);
	bool findTranslator(const QString & translationsPath);
	void loadNew(QString path);
	void loadOne(class MainWindow *, QString path, int loaded);
	void initSplash(class FSplashScreen & splash, QPixmap & pixmap);
	void registerFont(const QString &fontFile, bool reallyRegister);
	void clearModels();
    void copyBin(const QString & source, const QString & dest);

protected:
	bool m_spaceBarIsPressed;
	bool m_mousePressed;
	QTranslator m_translator;
	class ReferenceModel * m_referenceModel;
	class PaletteModel * m_paletteBinModel;
	bool m_started;
	QStringList m_filesToLoad;
	QString m_libPath;
	QString m_translationPath;
	class UpdateDialog * m_updateDialog;
	QTimer m_activationTimer;
	QPointer<class MainWindow> m_lastTopmostWindow;
	QList<QWidget *> m_orderedTopLevelWidgets;
	QStringList m_arguments;
	QStringList m_externalProcessArgs;
	QString m_externalProcessName;
	QString m_externalProcessPath;
	bool m_runAsService;
	int m_progressIndex;
	class FSplashScreen * m_splash;

public:
	static QSet<QString> InstalledFonts;
	static QMultiHash<QString, QString> InstalledFontsNameMapper;   // family name to filename; SVG files seem to have to use filename
	static int RestartNeeded;

};


#endif

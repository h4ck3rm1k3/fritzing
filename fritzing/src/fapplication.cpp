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

#include "fapplication.h"
#include "debugdialog.h"
#include "misc.h"
#include "mainwindow.h"
#include "fsplashscreen.h"
#include "version.h"
#include "prefsdialog.h"
#include "help/helper.h"
#include "partseditor/partseditormainwindow.h"
#include "layerattributes.h"
#include "fsvgrenderer.h"
#include "versionchecker.h"

// dependency injection :P
#include "referencemodel/sqlitereferencemodel.h"
#define CurrentReferenceModel SqliteReferenceModel

#include <QKeyEvent>
#include <QFileInfo>
#include <QDesktopServices>
#include <QLocale>
#include <QFileOpenEvent>
#include <QThread>
#include <QMessageBox>

bool FApplication::m_spaceBarIsPressed = false;
QString FApplication::m_openSaveFolder = ___emptyString___;
QTranslator FApplication::m_translator;
ReferenceModel * FApplication::m_referenceModel = NULL;
PaletteModel * FApplication::m_paletteBinModel = NULL;
bool FApplication::m_started = false;
QList<QString> FApplication::m_filesToLoad;
QString FApplication::m_libPath;
VersionChecker * FApplication::m_versionChecker = NULL;

static int kBottomOfAlpha = 206;

#ifdef Q_WS_WIN
#ifndef QT_NO_DEBUG
#define WIN_DEBUG
#endif
#endif

#ifdef LINUX_32
#define PLATFORM_NAME "linux-32bit"
#endif
#ifdef LINUX_64
#define PLATFORM_NAME "linux-64bit"
#endif
#ifdef Q_WS_WIN
#define PLATFORM_NAME "windows"
#endif
#ifdef Q_WS_MAC
#define PLATFORM_NAME "mac"
#endif

//////////////////////////

class DoOnceThread : public QThread
{
public:
	DoOnceThread();

	void run();
};


DoOnceThread::DoOnceThread() {
}

void DoOnceThread::run()
{
	static_cast<FApplication *>(qApp)->preloadSlowParts();
}

///////////////////////////

class CheckVersionThread : public QThread
{
public:

	void run();
};

void CheckVersionThread::run() {
	static_cast<FApplication *>(qApp)->checkVersion();
}


//////////////////////////

FApplication::FApplication( int & argc, char ** argv) : QApplication(argc, argv)
{
	m_started = false;

	QCoreApplication::setOrganizationName("Fritzing");
	QCoreApplication::setOrganizationDomain("fritzing.org");
	QCoreApplication::setApplicationName("Fritzing");

	installEventFilter(this);

#ifdef Q_WS_MAC
	QString lib = "/../lib";
#else
	QString lib = "/lib";
#endif

	m_libPath = applicationDirPath() + lib;   // applicationDirPath() doesn't work until after QApplication is instantiated
	addLibraryPath(m_libPath);							// tell app where to search for plugins (jpeg export and sql lite)

	// !!! translator must be installed before any widgets are created !!!
	findTranslator(m_libPath);

	Q_INIT_RESOURCE(phoenixresources);
}

FApplication::~FApplication(void)
{
}

bool FApplication::spaceBarIsPressed() {
	return m_spaceBarIsPressed;
}

void FApplication::setOpenSaveFolder(const QString& path) {
	QFileInfo fileInfo(path);
	if(fileInfo.isFile()) {
		m_openSaveFolder = path;
	} else {
		m_openSaveFolder = fileInfo.path().remove(fileInfo.fileName());
	}
}

const QString FApplication::openSaveFolder() {
	if(m_openSaveFolder == ___emptyString___) {
		DebugDialog::debug(QString("default save location: %1").arg(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)));
		return QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
	} else {
		return m_openSaveFolder;
	}
}

bool FApplication::eventFilter(QObject *obj, QEvent *event)
{
	// check whether the space bar is down.

	Q_UNUSED(obj);

	switch (event->type()) {
		case QEvent::KeyPress:
			{
				QKeyEvent * kevent = static_cast<QKeyEvent *>(event);
				if (!kevent->isAutoRepeat() && (kevent->key() == Qt::Key_Space)) {
					m_spaceBarIsPressed = true;
					emit spaceBarIsPressedSignal(true);
				}
			}
			break;
		case QEvent::KeyRelease:
			{
				QKeyEvent * kevent = static_cast<QKeyEvent *>(event);
				if (!kevent->isAutoRepeat() && (kevent->key() == Qt::Key_Space)) {
					m_spaceBarIsPressed = false;
					emit spaceBarIsPressedSignal(false);
				}
			}
			break;
		default:
			break;
	}

	return false;
}


bool FApplication::event(QEvent *event)
{
    switch (event->type()) {
		case QEvent::FileOpen:
			{
				QString path = static_cast<QFileOpenEvent *>(event)->file();
				if (m_started) {
					loadNew(path);
				}
				else {
					m_filesToLoad.append(path);
				}

			}
			return true;
		default:
			return QApplication::event(event);
    }
}

bool FApplication::findTranslator(const QString & libPath) {
	QSettings settings;
	QString suffix = settings.value("language").toString();
	if (suffix.isEmpty()) {
		suffix = QLocale::system().name();	   // Returns the language and country of this locale as a string of the form "language_country", where language is a lowercase, two-letter ISO 639 language code, and country is an uppercase, two-letter ISO 3166 country code.
	}

    bool loaded = m_translator.load(QString("fritzing_") + suffix, libPath + "/translations");
	if (loaded) {
		this->installTranslator(&m_translator);
	}

	return loaded;
}

int FApplication::startup(int & argc, char ** argv)
{
	int progressIndex;
    QPixmap pixmap(":/resources/images/splash_2010.png");
	FSplashScreen splash(pixmap);
	processEvents();								// seems to need this (sometimes?) to display the splash screen

	initSplash(splash, progressIndex, pixmap);
	processEvents();

	// DebugDialog::debug("Data Location: "+QDesktopServices::storageLocation(QDesktopServices::DataLocation));

	// so we can use ViewGeometry in a Qt::QueueConnection signal
	qRegisterMetaType<ViewGeometry>("ViewGeometry");

	MainWindow::initExportConstants();
	FSvgRenderer::calcPrinterScale();
	Wire::initNames();
    ItemBase::initNames();
    ViewLayer::initNames();
    ModelPart::initNames();
    Connector::initNames();
    ZoomComboBox::loadFactors();
	Helper::initText();
	PartsEditorMainWindow::initText();

	m_versionChecker = new VersionChecker();
	m_versionChecker->setUrl(QString("http://fritzing.org/download/feed/atom/%1/").arg(PLATFORM_NAME));
	connect(m_versionChecker, SIGNAL(releasesAvailable()), this, SLOT(notifyReleasesAvailable()));
	CheckVersionThread checkVersionThread;
	checkVersionThread.start();
	
	splash.showProgress(progressIndex, 0.1);
	processEvents();

#ifdef Q_WS_WIN
	// associate .fz file with fritzing app on windows (xp only--vista is different)
	// TODO: don't change settings if they're already set?
	// TODO: only do this at install time?
	QSettings settings1("HKEY_CLASSES_ROOT\\Fritzing", QSettings::NativeFormat);
	settings1.setValue(".", "Fritzing Application");
	QSettings settings2("HKEY_CLASSES_ROOT\\.fz", QSettings::NativeFormat);
	settings2.setValue(".", "Fritzing");
	QSettings settings3("HKEY_CLASSES_ROOT\\.fzb", QSettings::NativeFormat);
	settings3.setValue(".", "Fritzing");
	QSettings settings4("HKEY_CLASSES_ROOT\\.fzp", QSettings::NativeFormat);
	settings4.setValue(".", "Fritzing");
	QSettings settings5("HKEY_CLASSES_ROOT\\.fzz", QSettings::NativeFormat);
	settings5.setValue(".", "Fritzing");
	QSettings settings6("HKEY_CLASSES_ROOT\\Fritzing\\shell\\open\\command", QSettings::NativeFormat);
	settings6.setValue(".", QString("\"%1\" \"%2\"")
					   .arg(QDir::toNativeSeparators(QApplication::applicationFilePath()))
					   .arg("%1") );
#endif


	m_referenceModel = new CurrentReferenceModel();
	m_paletteBinModel = new PaletteModel(true, false);

	QSettings settings;
	QString prevVersion = settings.value("version").toString();
	QString currVersion = Version::versionString();
	if(prevVersion != currVersion) {
		settings.clear();
	}

	splash.showProgress(progressIndex, 0.2);
	processEvents();

	QString binToOpen = settings.value("lastBin").toString();
	binToOpen = binToOpen.isNull() || binToOpen.isEmpty() ? MainWindow::CoreBinLocation : binToOpen;

	if (!m_paletteBinModel->load(binToOpen, m_referenceModel)) {
		if(binToOpen == MainWindow::CoreBinLocation
		   || !m_paletteBinModel->load(MainWindow::CoreBinLocation, m_referenceModel)) {
			// TODO: we're really screwed, what now?
			QMessageBox::warning(NULL, QObject::tr("Fritzing"), QObject::tr("Friting cannot load the parts bin"));
			return -1;
		}
	}

	splash.showProgress(progressIndex, 0.4);
	processEvents();

	DebugDialog::debug("starting thread");
	QMutex mutex;
	mutex.lock();
	DoOnceThread doOnceThread;
	doOnceThread.start();
	while (!doOnceThread.isFinished()) {
		processEvents();
		mutex.tryLock(10);							// always fails, but effectively sleeps for 10 ms
	}
	mutex.unlock();

	splash.showProgress(progressIndex, 0.7);

	DebugDialog::debug("ending thread");

	// our MainWindows use WA_DeleteOnClose so this has to be added to the heap (via new) rather than the stack (for local vars)
	MainWindow * mainWindow = new MainWindow(m_paletteBinModel, m_referenceModel);

#ifndef WIN_DEBUG
	// not sure why, but calling showProgress after the main window is instantiated seems to cause a deadlock in windows debug mode
	// thought it might be the splashscreen calling QApplication::flush() but eliminating that didn't remove the deadlock
	splash.showProgress(progressIndex, 0.9);
	processEvents();
#endif

	int loaded = 0;
	for(int i=1; i < argc; i++) {
		loadOne(mainWindow, argv[i], loaded++);
	}

	DebugDialog::debug("after argc");

	foreach (QString filename, m_filesToLoad) {
		loadOne(mainWindow, filename, loaded++);
	}

	DebugDialog::debug("after m_files");

	if (loaded == 0)
	{
		if(!settings.value("lastOpenSketch").isNull()) {
			QString lastSketchPath = settings.value("lastOpenSketch").toString();
			if(QFileInfo(lastSketchPath).exists()) {
				settings.remove("lastOpenSketch");				// clear the preference, in case the load crashes
				mainWindow->load(lastSketchPath);
				settings.setValue("lastOpenSketch", lastSketchPath);	// the load works, so restore the preference
			} else {
				settings.remove("lastOpenSketch");
			}
		}
	}

	DebugDialog::debug("after last open sketch");

	m_started = true;

#ifndef WIN_DEBUG
	// not sure why, but calling showProgress after the main window is instantiated seems to cause a deadlock in windows debug mode
	splash.showProgress(progressIndex, 0.99);
	processEvents();
#endif

	mainWindow->show();

	/*
	 QDate now = QDate::currentDate();
	 QDate over = QDate(2009, 1, 7);
	 if (now < over) {
		 QString path = getApplicationSubFolderPath("examples") + "/Fritzmas/treeduino.fz";
		 QFileInfo tree(path);
		 if (tree.exists()) {
			 MainWindow * treeduino = new MainWindow(paletteBinModel, referenceModel);
			 treeduino->load(path, false);
			 treeduino->show();
		}
	 }
	 */

	splash.finish(mainWindow);

	return 0;
}

void FApplication::finish()
{
    delete m_paletteBinModel;
    delete m_referenceModel;

	QString currVersion = Version::versionString();
	QSettings settings;
    settings.setValue("version", currVersion);
}

void FApplication::loadNew(QString path) {
	MainWindow * mw = new MainWindow(m_paletteBinModel, m_referenceModel);
	if (!mw->loadWhich(path, false)) {
		mw->close();
	}
}

void FApplication::loadOne(MainWindow * mw, QString path, int loaded) {
	if (loaded == 0) {
		mw->loadWhich(path);
	}
	else {
		loadNew(path);
	}
}

void FApplication::preferences() {
	QDir dir(m_libPath + "/translations");
	QStringList nameFilters;
	nameFilters << "*.qm";
    QFileInfoList list = dir.entryInfoList(nameFilters, QDir::Files | QDir::NoSymLinks);
	QSettings settings;
	QString language = settings.value("language").toString();
	if (language.isEmpty()) {
		language = QLocale::system().name();
	}

	PrefsDialog prefsDialog(language, list, NULL);			// TODO: use the topmost MainWindow as parent
	if (QDialog::Accepted == prefsDialog.exec()) {
		if (prefsDialog.cleared()) {
			settings.clear();
		}
		else {
			QString name = prefsDialog.name();
			settings.setValue("language", name);
		}
	}
}

void FApplication::initSplash(FSplashScreen & splash, int & progressIndex, QPixmap & pixmap) {
	QPixmap logo(":/resources/images/fhp_logo_small.png");
	QPixmap progress(":/resources/images/splash_progressbar.png");

	progressIndex = splash.showPixmap(progress, QPoint(0, pixmap.height() - progress.height()));
	splash.showProgress(progressIndex, 0);

	// put this above the progress indicator
	splash.showPixmap(logo, QPoint(5, pixmap.height() - 12));

	QColor w(0xea, 0xf4, 0xed);
	QRect r1(45, kBottomOfAlpha, pixmap.width() - 45, 20);
	QString msg1 = QObject::tr("<font face='Lucida Grande, Tahoma, Sans Serif' size='2' color='#eaf4ed'>"
							   "&#169; 2007-%1 Fachhochschule Potsdam"
							   "</font>")
	.arg(Version::year());
	splash.showMessage(msg1, r1, Qt::AlignLeft | Qt::AlignTop, w);

	QRect r2(0, kBottomOfAlpha, pixmap.width() - 12, 20);
	QString msg2 = QObject::tr("<font face='Lucida Grande, Tahoma, Sans Serif' size='2' color='#eaf4ed'>"
							   "Version %1.%2.%3 (%4%5)"
							   "</font>")
	.arg(Version::majorVersion())
	.arg(Version::minorVersion())
	.arg(Version::minorSubVersion())
	.arg(Version::modifier())
	.arg(Version::shortDate());
	splash.showMessage(msg2, r2, Qt::AlignRight | Qt::AlignTop, w);
    splash.show();
}

void FApplication::preloadSlowParts() {
	// loads the part into a renderer and sets up its connectors
	// so this doesn't have to happen the first time the part is dragged into the sketch

	//QTime t;
	//t.start();
	DebugDialog::debug(QString("preload slow parts"));
	ModelPart * modelPart = m_paletteBinModel->retrieveModelPart(ItemBase::breadboardModuleIDName);
	LayerAttributes layerAttributes;
	FSvgRenderer * renderer = PaletteItemBase::setUpImage(modelPart, ItemBase::BreadboardView, ViewLayer::BreadboardBreadboard, layerAttributes);
	DebugDialog::debug(QString("preload set up image"));
	foreach (Connector * connector, modelPart->connectors().values()) {
		if (connector == NULL) continue;

		QRectF connectorRect;
		QPointF terminalPoint;
		connector->setUpConnector(renderer, ItemBase::BreadboardView, ViewLayer::BreadboardBreadboard, connectorRect, terminalPoint, false);
		DebugDialog::debug(QString("preload set up connector %1").arg(connector->connectorStuffID()));
	}
	//DebugDialog::debug(QString("preload slow parts elapsed (1) %1").arg(t.elapsed()) );
	DebugDialog::debug(QString("preload slow parts done") );
}

void FApplication::checkVersion() {
	if (m_versionChecker) {
		m_versionChecker->fetch();
	}
}

void FApplication::notifyReleasesAvailable() {
	// put up a modal dialog?

	DebugDialog::debug("got releases available");
}

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
#include "utils/misc.h"
#include "mainwindow.h"
#include "fsplashscreen.h"
#include "version/version.h"
#include "prefsdialog.h"
#include "help/helper.h"
#include "partseditor/partseditormainwindow.h"
#include "layerattributes.h"
#include "fsvgrenderer.h"
#include "version/versionchecker.h"
#include "version/updatedialog.h"
#include "itemdrag.h"
#include "viewswitcher/viewswitcher.h"
#include "items/wire.h"
#include "partsbinpalette/binmanager/binmanager.h"
#include "help/tipsandtricks.h"

// dependency injection :P
#include "referencemodel/sqlitereferencemodel.h"
#define CurrentReferenceModel SqliteReferenceModel

#include <QSettings>
#include <QKeyEvent>
#include <QFileInfo>
#include <QDesktopServices>
#include <QLocale>
#include <QFileOpenEvent>
#include <QThread>
#include <QMessageBox>
#include <QTextStream>
#include <QFontDatabase>

bool FApplication::m_spaceBarIsPressed = false;
bool FApplication::m_mousePressed = false;
QString FApplication::m_openSaveFolder = ___emptyString___;
QTranslator FApplication::m_translator;
ReferenceModel * FApplication::m_referenceModel = NULL;
PaletteModel * FApplication::m_paletteBinModel = NULL;
bool FApplication::m_started = false;
QList<QString> FApplication::m_filesToLoad;
QString FApplication::m_libPath;
QString FApplication::m_translationPath;
UpdateDialog * FApplication::m_updateDialog = NULL;
QSet<QString> FApplication::InstalledFonts;
QPointer<MainWindow> FApplication::m_lastTopmostWindow = NULL;
QTimer FApplication::m_activationTimer;
QList<QWidget *> FApplication::m_orderedTopLevelWidgets;


static int kBottomOfAlpha = 204;

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

//////////////////////////

FApplication::FApplication( int & argc, char ** argv) : QApplication(argc, argv)
{
	m_started = false;
	m_updateDialog = NULL;
	m_lastTopmostWindow = NULL;

	connect(&m_activationTimer, SIGNAL(timeout()), this, SLOT(updateActivation()));
	m_activationTimer.setInterval(10);
	m_activationTimer.setSingleShot(true);

	QCoreApplication::setOrganizationName("Fritzing");
	QCoreApplication::setOrganizationDomain("fritzing.org");
	QCoreApplication::setApplicationName("Fritzing");

	installEventFilter(this);

#ifdef Q_WS_MAC
	QString lib = "/../lib";
#else
	QString lib = "/lib";
#endif

	m_libPath = QDir::cleanPath(applicationDirPath() + lib);		// applicationDirPath() doesn't work until after QApplication is instantiated
	addLibraryPath(m_libPath);					// tell app where to search for plugins (jpeg export and sql lite)

	/*QFile file("libpath.txt");
	if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QTextStream out(&file);
		out << m_libPath;
		file.close();
	}*/

	// !!! translator must be installed before any widgets are created !!!
	m_translationPath = m_libPath + "/translations";
	bool loaded = findTranslator(m_translationPath);
	if (!loaded) {
		m_translationPath = getApplicationSubFolderPath("translations");
		loaded = findTranslator(m_translationPath);
	}

	Q_INIT_RESOURCE(phoenixresources);
}

FApplication::~FApplication(void)
{

	if (m_paletteBinModel) {
		m_paletteBinModel->clearPartHash();
		delete m_paletteBinModel;
	}
	if (m_referenceModel) {
		m_referenceModel->clearPartHash();
		delete m_referenceModel;
	}
	if (m_updateDialog) {
		delete m_updateDialog;
	}

	FSvgRenderer::cleanup();
	ViewIdentifierClass::cleanup();
	ViewLayer::cleanup();
	ItemBase::cleanup();
	Wire::cleanup();
	DebugDialog::cleanup();
	ViewSwitcher::cleanup();
	ItemDrag::cleanup();
	Version::cleanup();
	TipsAndTricks::cleanup();

}

bool FApplication::spaceBarIsPressed() {
	return m_spaceBarIsPressed;
}

void FApplication::setOpenSaveFolder(const QString& path) {
	QFileInfo fileInfo(path);
	if(fileInfo.isDir()) {
		m_openSaveFolder = path;
	} else {
		m_openSaveFolder = fileInfo.path().remove(fileInfo.fileName());
	}
	QSettings settings;
	settings.setValue("openSaveFolder", m_openSaveFolder);
}

const QString FApplication::openSaveFolder() {
	if(m_openSaveFolder == ___emptyString___) {
		QSettings settings;
		QString tempFolder = settings.value("openSaveFolder").toString();
		if (!tempFolder.isEmpty()) {
			QFileInfo fileInfo(tempFolder);
			if (fileInfo.exists()) {
				m_openSaveFolder = tempFolder;
				return m_openSaveFolder;
			}
			else {
				settings.remove("openSaveFolder");
			}
		}

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
		case QEvent::MouseButtonPress:
			m_mousePressed = true;
			break;
		case QEvent::MouseButtonRelease:
			m_mousePressed = false;
			break;
		case QEvent::KeyPress:
			{
				if (!m_mousePressed) {
					QKeyEvent * kevent = static_cast<QKeyEvent *>(event);
					if (!kevent->isAutoRepeat() && (kevent->key() == Qt::Key_Space)) {
						m_spaceBarIsPressed = true;
						emit spaceBarIsPressedSignal(true);
					}
				}
			}
			break;
		case QEvent::KeyRelease:
			{
				if (m_spaceBarIsPressed) {
					QKeyEvent * kevent = static_cast<QKeyEvent *>(event);
					if (!kevent->isAutoRepeat() && (kevent->key() == Qt::Key_Space)) {
						m_spaceBarIsPressed = false;
						emit spaceBarIsPressedSignal(false);
					}
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

bool FApplication::findTranslator(const QString & translationsPath) {
	QSettings settings;
	QString suffix = settings.value("language").toString();
	if (suffix.isEmpty()) {
		suffix = QLocale::system().name();	   // Returns the language and country of this locale as a string of the form "language_country", where language is a lowercase, two-letter ISO 639 language code, and country is an uppercase, two-letter ISO 3166 country code.
	}
	else {
		QLocale::setDefault(QLocale(suffix));
	}

    bool loaded = m_translator.load(QString("fritzing_") + suffix, translationsPath);
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
	ViewIdentifierClass::initNames();
	Wire::initNames();
    ItemBase::initNames();
    ViewLayer::initNames();
    ModelPart::initNames();
    Connector::initNames();
    SketchWidget::init();
    ZoomComboBox::loadFactors();
	Helper::initText();
	PartsEditorMainWindow::initText();
	BinManager::MyPartsBinLocation = getUserDataStorePath("bins")+"/my_parts.fzb";
	BinManager::MyPartsBinTemplateLocation =":/resources/bins/my_parts.fzb";

	QList<int> fontIds;
	registerFont(":/resources/fonts/DroidSans.ttf", fontIds);
	registerFont(":/resources/fonts/DroidSans-Bold.ttf", fontIds);
	registerFont(":/resources/fonts/ocra10.ttf", fontIds);

	foreach(int id, fontIds) {
		foreach(QString ff, QFontDatabase::applicationFontFamilies(id)) {
			InstalledFonts << ff;
			DebugDialog::debug("installing font family: "+ff);
		}
	}

	
	QFontDatabase database;
	QStringList families = database.families (  );
	foreach (QString string, families) {
		DebugDialog::debug(string);			// should print out the name of the font you loaded
	}
	

	splash.showProgress(progressIndex, 0.085);
	processEvents();

#ifdef Q_WS_WIN
	// associate .fz file with fritzing app on windows (xp only--vista is different)
	// TODO: don't change settings if they're already set?
	// TODO: only do this at install time?
	QSettings settings1("HKEY_CLASSES_ROOT\\Fritzing", QSettings::NativeFormat);
	settings1.setValue(".", "Fritzing Application");
	foreach (QString extension, fritzingExtensions()) {
		QSettings settings2("HKEY_CLASSES_ROOT\\" + extension, QSettings::NativeFormat);
		settings2.setValue(".", "Fritzing");
	}
	QSettings settings3("HKEY_CLASSES_ROOT\\Fritzing\\shell\\open\\command", QSettings::NativeFormat);
	settings3.setValue(".", QString("\"%1\" \"%2\"")
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

	splash.showProgress(progressIndex, 0.1);
	createUserDataStoreFolderStructure();


	splash.showProgress(progressIndex, 0.2);
	processEvents();

	QString binToOpen = settings.value("lastBin").toString();
	binToOpen = binToOpen.isNull() || binToOpen.isEmpty()? MainWindow::CoreBinLocation: binToOpen;

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

	QTime t;
	t.start();
	//DebugDialog::debug("starting thread");
	QMutex mutex;
	mutex.lock();
	DoOnceThread doOnceThread;
	doOnceThread.start();
	while (!doOnceThread.isFinished()) {
		processEvents();
		mutex.tryLock(10);							// always fails, but effectively sleeps for 10 ms
	}
	mutex.unlock();
	//DebugDialog::debug(QString("ending thread %1").arg(t.elapsed()));

	splash.showProgress(progressIndex, 0.65);


	m_updateDialog = new UpdateDialog();
	connect(m_updateDialog, SIGNAL(enableAgainSignal(bool)), this, SLOT(enableCheckUpdates(bool)));
	checkForUpdates(false);

	splash.showProgress(progressIndex, 0.70);

	// our MainWindows use WA_DeleteOnClose so this has to be added to the heap (via new) rather than the stack (for local vars)
	MainWindow * mainWindow = MainWindow::newMainWindow(m_paletteBinModel, m_referenceModel, "", false);

	splash.showProgress(progressIndex, 0.9);
	processEvents();

	int loaded = 0;
	for(int i=1; i < argc; i++) {
		loadOne(mainWindow, argv[i], loaded++);
	}

	//DebugDialog::debug("after argc");

	foreach (QString filename, m_filesToLoad) {
		loadOne(mainWindow, filename, loaded++);
	}

	//DebugDialog::debug("after m_files");

	if (loaded == 0)
	{
		if(!settings.value("lastOpenSketch").isNull()) {
			QString lastSketchPath = settings.value("lastOpenSketch").toString();
			if(QFileInfo(lastSketchPath).exists()) {
				settings.remove("lastOpenSketch");				// clear the preference, in case the load crashes
				mainWindow->showFileProgressDialog(lastSketchPath);
				mainWindow->load(lastSketchPath);
				loaded++;
				settings.setValue("lastOpenSketch", lastSketchPath);	// the load works, so restore the preference
			} else {
				settings.remove("lastOpenSketch");
			}
		}
	}

	//DebugDialog::debug("after last open sketch");

	m_started = true;

//#ifndef WIN_DEBUG
	// not sure why, but calling showProgress after the main window is instantiated seems to cause a deadlock in windows debug mode
	splash.showProgress(progressIndex, 0.99);
	processEvents();
//#endif

	if (!loaded) {
		mainWindow->addBoard();
	}

	mainWindow->show();

	/*
	 QDate now = QDate::currentDate();
	 QDate over = QDate(2009, 1, 7);
	 if (now < over) {
		 QString path = getApplicationSubFolderPath("examples") + "/Fritzmas/treeduino.fz";
		 QFileInfo tree(path);
		 if (tree.exists()) {
			 MainWindow * treeduino = MainWindow::newMainWindow(paletteBinModel, referenceModel, false);
			 treeduino->load(path, false);
			 treeduino->show();
		}
	 }
	 */

	splash.finish(mainWindow);

	mainWindow->clearFileProgressDialog();

	if(prevVersion != ___emptyString___
	   && Version::greaterThan(prevVersion,Version::FirstVersionWithDetachedUserData))
	{
		QMessageBox messageBox( 
			QMessageBox::Question, 
			tr("Import files from previous version?"), 
			tr("Do you want to import parts and bins that you have created with earlier versions of Fritzing?\n"
			   "\nNote: You can import them later using the \"Help\" > \"Import parts and bins "
			   "from old version...\" menu action."),
			   QMessageBox::Ok | QMessageBox::Cancel, 
			   mainWindow, 
			   Qt::Sheet );
	    messageBox.setButtonText(QMessageBox::Ok, tr("Import"));
		messageBox.setButtonText(QMessageBox::Cancel, tr("Do not import now"));
		messageBox.setDefaultButton(QMessageBox::Cancel);
		QMessageBox::StandardButton answer = (QMessageBox::StandardButton) messageBox.exec();

		/*

		QMessageBox::StandardButton answer = QMessageBox::question(
			mainWindow,
			tr("Import files from previous version?"),
			tr("Do you want to import parts and bins that you have created with earlier versions of Fritzing?\n"
			   "\nNote: You can also import them later through the \"Help\" > \"Import parts and bins "
			   "from old version...\" menu action."),
			QMessageBox::Ok | QMessageBox::Cancel,
			QMessageBox::Ok
		);

		*/


		if(answer == QMessageBox::Ok) {
			mainWindow->importFilesFromPrevInstall();
		}
	}

	return 0;
}

void FApplication::registerFont(const QString &fontFile, QList<int> &fontIds) {
	int id = QFontDatabase::addApplicationFont(fontFile);
	if(id > -1) {
		fontIds << id;
	}
}

void FApplication::finish()
{
	QString currVersion = Version::versionString();
	QSettings settings;
    settings.setValue("version", currVersion);
}

void FApplication::loadNew(QString path) {
	MainWindow * mw = MainWindow::newMainWindow(m_paletteBinModel, m_referenceModel, path, true);
	if (!mw->loadWhich(path, false)) {
		mw->close();
	}
	mw->clearFileProgressDialog();
}

void FApplication::loadOne(MainWindow * mw, QString path, int loaded) {
	if (loaded == 0) {
		mw->showFileProgressDialog(path);
		mw->loadWhich(path);
	}
	else {
		loadNew(path);
	}
}

void FApplication::preferences() {
	QDir dir(m_translationPath);
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
	//DebugDialog::debug(QString("preload slow parts"));
	ModelPart * modelPart = m_paletteBinModel->retrieveModelPart(ItemBase::breadboardModuleIDName);
	if (modelPart == NULL) {
		// something is badly wrong--parts folder is missing, for example
		return;
	}

	LayerAttributes layerAttributes;
	FSvgRenderer * renderer = ItemBase::setUpImage(modelPart, ViewIdentifierClass::BreadboardView, ViewLayer::BreadboardBreadboard, layerAttributes);
	//DebugDialog::debug(QString("preload set up image"));
	foreach (Connector * connector, modelPart->connectors().values()) {
		if (connector == NULL) continue;

		QRectF connectorRect;
		QPointF terminalPoint;
		qreal radius, strokeWidth;
		connector->setUpConnector(renderer, modelPart->moduleID(), ViewIdentifierClass::BreadboardView, ViewLayer::BreadboardBreadboard, connectorRect, terminalPoint, radius, strokeWidth, false);
		//DebugDialog::debug(QString("preload set up connector %1").arg(connector->connectorSharedID()));
	}
	//DebugDialog::debug(QString("preload slow parts elapsed (1) %1").arg(t.elapsed()) );
	//DebugDialog::debug(QString("preload slow parts done") );
}

void FApplication::checkForUpdates() {
	checkForUpdates(true);
}

void FApplication::checkForUpdates(bool atUserRequest)
{
	if (atUserRequest) {
		enableCheckUpdates(false);
	}

	VersionChecker * versionChecker = new VersionChecker();

	if (!atUserRequest) {
		// if I've already been notified about these updates, don't bug me again
		QSettings settings;
		QString lastMainVersionChecked = settings.value("lastMainVersionChecked").toString();
		if (!lastMainVersionChecked.isEmpty()) {
			versionChecker->ignore(lastMainVersionChecked, false);
		}
		QString lastInterimVersionChecked = settings.value("lastInterimVersionChecked").toString();
		if (!lastInterimVersionChecked.isEmpty()) {
			versionChecker->ignore(lastInterimVersionChecked, true);
		}
	}

	versionChecker->setUrl(QString("http://fritzing.org/download/feed/atom/%1/").arg(PLATFORM_NAME));
	m_updateDialog->setAtUserRequest(atUserRequest);
	m_updateDialog->setVersionChecker(versionChecker);

	if (atUserRequest) {
		m_updateDialog->show();
	}
}

void FApplication::enableCheckUpdates(bool enabled)
{
    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        MainWindow *mainWindow = qobject_cast<MainWindow *>(widget);
		if (mainWindow) {
			mainWindow->enableCheckUpdates(enabled);
		}
	}
}


QString FApplication::getOpenFileName( QWidget * parent, const QString & caption, const QString & dir, const QString & filter, QString * selectedFilter, QFileDialog::Options options )
{
	QString result = QFileDialog::getOpenFileName(parent, caption, dir, filter, selectedFilter, options);
	if (!result.isNull()) {
		setOpenSaveFolder(result);
	}
	return result;
}

QString FApplication::getSaveFileName( QWidget * parent, const QString & caption, const QString & dir, const QString & filter, QString * selectedFilter, QFileDialog::Options options )
{
	//DebugDialog::debug(QString("getopenfilename %1 %2 %3 %4").arg(caption).arg(dir).arg(filter).arg(*selectedFilter));
	QString result = QFileDialog::getSaveFileName(parent, caption, dir, filter, selectedFilter, options);
	if (!result.isNull()) {
		setOpenSaveFolder(result);
	}
	return result;
}

void FApplication::createUserDataStoreFolderStructure() {
	// make sure that the folder structure for parts and bins, exists
	QString userDataStorePath = getUserDataStorePath();
	QDir dataStore(userDataStorePath);
	QStringList dataFolders = getUserDataStoreFolders();
	foreach(QString folder, dataFolders) {
		if(!QFileInfo(dataStore.absolutePath()+folder).exists()) {
			QString folderaux = folder.startsWith("/")? folder.remove(0,1): folder;
			dataStore.mkpath(folder);
		}
	}

	if(!QFileInfo(BinManager::MyPartsBinLocation).exists()) {
		// this copy action, is not working on windows, because is a resources file
		if(!QFile(BinManager::MyPartsBinTemplateLocation).copy(BinManager::MyPartsBinLocation)) {
#ifdef Q_WS_WIN // may not be needed from qt 4.5.2 on
			DebugDialog::debug("Failed to copy a file from the resources");
			QApplication::processEvents();
			QDir binsFolder = QFileInfo(BinManager::MyPartsBinLocation).dir().absolutePath();
			QStringList binFiles = binsFolder.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
			foreach(QString binName, binFiles) {
				if(binName.startsWith("qt_temp.")) {
					QString filePath = binsFolder.absoluteFilePath(binName);
					bool success = QFile(filePath).rename(BinManager::MyPartsBinLocation);
					Q_UNUSED(success);
					break;
				}
			}
#endif
		}
	}
}

void FApplication::changeActivation(bool activate, QWidget * originator) {
	if (!activate) return;

	MainWindow * mainWindow = qobject_cast<MainWindow *>(originator);
	if (mainWindow == NULL) {
		mainWindow = qobject_cast<MainWindow *>(originator->parent());
	}
	if (mainWindow == NULL) return;

	m_orderedTopLevelWidgets.removeOne(mainWindow);
	m_orderedTopLevelWidgets.push_front(mainWindow);

	m_activationTimer.stop();
	m_activationTimer.start();
}

void FApplication::updateActivation() {
	DebugDialog::debug("updating activation");

	MainWindow * prior = m_lastTopmostWindow; 
	m_lastTopmostWindow = NULL;
	if (m_orderedTopLevelWidgets.count() > 0) {
		m_lastTopmostWindow = qobject_cast<MainWindow *>(m_orderedTopLevelWidgets.at(0));
	}

	if (prior == m_lastTopmostWindow) return;

	DebugDialog::debug(QString("last:%1, new:%2").arg((long) prior, 0, 16).arg((long) m_lastTopmostWindow.data(), 0, 16));

	if (prior != NULL) {			
		prior->saveDocks();
	}
	if (m_lastTopmostWindow != NULL) {
		m_lastTopmostWindow->restoreDocks();
		DebugDialog::debug("restoring active window");
	}
}

void FApplication::topLevelWidgetDestroyed(QObject * object) {
	QWidget * widget = qobject_cast<QWidget *>(object);
	if (widget) {
		m_orderedTopLevelWidgets.removeOne(widget);
	}
}



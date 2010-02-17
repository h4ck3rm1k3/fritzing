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
#include "dialogs/prefsdialog.h"
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
#include "utils/folderutils.h"
#include "dialogs/translatorlistmodel.h"
#include "partsbinpalette/svgiconwidget.h"
#include "items/moduleidnames.h"
#include "partsbinpalette/searchlineedit.h"
#include "utils/ratsnestcolors.h"

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

int FApplication::RestartNeeded = 9999;
QSet<QString> FApplication::InstalledFonts;
QMultiHash<QString, QString> FApplication::InstalledFontsNameMapper;   // family name to filename; SVG files seem to have to use filename

static const qreal LoadProgressStart = 0.085;
static const qreal LoadProgressEnd = 0.6;

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
	MainWindow::RestartNeeded = RestartNeeded;
	m_spaceBarIsPressed = false;
	m_mousePressed = false;
	m_referenceModel = NULL;
	m_paletteBinModel = NULL;
	m_started = false;
	m_updateDialog = NULL;
	m_lastTopmostWindow = NULL;
	m_runAsService = false;

	m_arguments = arguments();
	QList<int> toRemove;
	for (int i = 0; i < m_arguments.length() - 1; i++) {
		if ((m_arguments[i].compare("-f", Qt::CaseInsensitive) == 0) ||
			(m_arguments[i].compare("-folder", Qt::CaseInsensitive) == 0)||
			(m_arguments[i].compare("--folder", Qt::CaseInsensitive) == 0))
		{
			FolderUtils::setApplicationPath(m_arguments[i + 1]);
			// delete these so we don't try to process them as files later
			toRemove << i;
			toRemove << i + 1;
		}

		if ((m_arguments[i].compare("-g", Qt::CaseInsensitive) == 0) ||
			(m_arguments[i].compare("-gerber", Qt::CaseInsensitive) == 0)||
			(m_arguments[i].compare("--gerber", Qt::CaseInsensitive) == 0)) {
			m_runAsService = true;
			toRemove << i;
		}

		if ((m_arguments[i].compare("-go", Qt::CaseInsensitive) == 0)) {
			m_runAsService = true;
			toRemove << i;
			toRemove << i + 1;
			m_gerberOutputFolder = m_arguments[i + 1];
		}


		if (m_arguments[i].compare("-ep", Qt::CaseInsensitive) == 0) {
			m_externalProcessPath = m_arguments[i + 1];
			toRemove << i;
			toRemove << (i + 1);
		}

		if (m_arguments[i].compare("-eparg", Qt::CaseInsensitive) == 0) {
			m_externalProcessArgs << m_arguments[i + 1];
			toRemove << i;
			toRemove << (i + 1);
		}

		if (m_arguments[i].compare("-epname", Qt::CaseInsensitive) == 0) {
			m_externalProcessName = m_arguments[i + 1];
			toRemove << i;
			toRemove << (i + 1);
		}

	}

	while (toRemove.count() > 0) {
		int ix = toRemove.takeLast();
		m_arguments.removeAt(ix);
	}

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

	// tell app where to search for plugins (jpeg export and sql lite)
	m_libPath = FolderUtils::getLibraryPath();
	addLibraryPath(m_libPath);	

	/*QFile file("libpath.txt");
	if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QTextStream out(&file);
		out << m_libPath;
		file.close();
	}*/

	// !!! translator must be installed before any widgets are created !!!
	m_translationPath = FolderUtils::getApplicationSubFolderPath("translations");

	bool loaded = findTranslator(m_translationPath);
	Q_UNUSED(loaded);

	Q_INIT_RESOURCE(phoenixresources);

	qRegisterMetaType<ViewGeometry>("ViewGeometry");

	MainWindow::initExportConstants();
	FSvgRenderer::calcPrinterScale();
	ViewIdentifierClass::initNames();
	Wire::initNames();
	ItemBase::initNames();
	ViewLayer::initNames();
	Connector::initNames();
	Helper::initText();
	PartsEditorMainWindow::initText();
	BinManager::initNames();
	PaletteModel::initNames();
	RatsnestColors::initNames();

}

FApplication::~FApplication(void)
{

	clearModels();

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
	TranslatorListModel::cleanup();
	FolderUtils::cleanup();
	SearchLineEdit::cleanup();
	RatsnestColors::cleanup();
}

void FApplication::clearModels() {
	if (m_paletteBinModel) {
		m_paletteBinModel->clearPartHash();
		delete m_paletteBinModel;
	}
	if (m_referenceModel) {
		m_referenceModel->clearPartHash();
		delete m_referenceModel;
	}
}

bool FApplication::spaceBarIsPressed() {
	return ((FApplication *) qApp)->m_spaceBarIsPressed;
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

void FApplication::registerFonts() {
	registerFont(":/resources/fonts/DroidSans.ttf", true);
	registerFont(":/resources/fonts/DroidSans-Bold.ttf", false);
	//registerFont(":/resources/fonts/ocra10.ttf", true);
	registerFont(":/resources/fonts/OCRA.otf", true);

	/*
		QFontDatabase database;
		QStringList families = database.families (  );
		foreach (QString string, families) {
			DebugDialog::debug(string);			// should print out the name of the fonts you loaded
		}
	*/
}

void FApplication::loadReferenceModel() {
	m_referenceModel = new CurrentReferenceModel();	
	ItemBase::setReferenceModel(m_referenceModel);
	connect(m_referenceModel, SIGNAL(loadedPart(int, int)), this, SLOT(loadedPart(int, int)));
	m_referenceModel->loadAll();								// this is very slow
	//DebugDialog::debug("after new current reference model");
	m_paletteBinModel = new PaletteModel(true, false);
	//DebugDialog::debug("after new palette model");
}

bool FApplication::loadBin(QString binToOpen) {
	binToOpen = binToOpen.isNull() || binToOpen.isEmpty()? BinManager::CorePartsBinLocation: binToOpen;

	if (!m_paletteBinModel->load(binToOpen, m_referenceModel)) {
		if(binToOpen == BinManager::CorePartsBinLocation
		   || !m_paletteBinModel->load(BinManager::CorePartsBinLocation, m_referenceModel)) {
			return false;
		}
	}

	return true;
}

MainWindow * FApplication::loadWindows(bool showProgress, int & loaded) {
	// our MainWindows use WA_DeleteOnClose so this has to be added to the heap (via new) rather than the stack (for local vars)
	MainWindow * mainWindow = MainWindow::newMainWindow(m_paletteBinModel, m_referenceModel, "", false);   // this is also slow

	if (showProgress) {
		m_splash->showProgress(m_progressIndex, 0.9);
		processEvents();
	}

	loaded = 0;
	for (int i = 1; i < m_arguments.length(); i++) {
		QFileInfo fileinfo(m_arguments[i]);
		if (fileinfo.exists() && !fileinfo.isDir()) {
			loadOne(mainWindow, m_arguments[i], loaded++);
		}
	}

	//DebugDialog::debug("after argc");

	return mainWindow;
}

int FApplication::serviceStartup() {

	if (m_gerberOutputFolder.isEmpty()) {
		return -1;
	}

	registerFonts();
	loadReferenceModel();
	createUserDataStoreFolderStructure();
	if (!loadBin("")) {
		return -1;
	}

	int loaded = 0;
	MainWindow * mainWindow = loadWindows(false, loaded);
	m_started = true;

	if (loaded == 0) {
		return -1;
	}

	mainWindow->exportToGerber(m_gerberOutputFolder, NULL);

	return 0;
}

int FApplication::startup(bool firstRun)
{
	QPixmap pixmap(":/resources/images/splash_2010.png");
	FSplashScreen splash(pixmap);
	m_splash = &splash;
	processEvents();								// seems to need this (sometimes?) to display the splash screen

	initSplash(splash, pixmap);
	processEvents();

	// DebugDialog::debug("Data Location: "+QDesktopServices::storageLocation(QDesktopServices::DataLocation));

	if(firstRun) {		
		registerFonts();

		splash.showProgress(m_progressIndex, LoadProgressStart);
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

	} 
	else 
	{
		clearModels();
		FSvgRenderer::cleanup();
	}

	loadReferenceModel();

	QSettings settings;
	QString prevVersion = settings.value("version").toString();
	QString currVersion = Version::versionString();
	if(prevVersion != currVersion) {
		settings.clear();
	}

	splash.showProgress(m_progressIndex, LoadProgressEnd);

	createUserDataStoreFolderStructure();

	//DebugDialog::debug("after createUserDataStoreFolderStructure");

	splash.showProgress(m_progressIndex, 0.65);
	processEvents();

	if (!loadBin(settings.value("lastBin").toString())) {
			// TODO: we're really screwed, what now?
		QMessageBox::warning(NULL, QObject::tr("Fritzing"), QObject::tr("Friting cannot load the parts bin"));
		return -1;
	}

	splash.showProgress(m_progressIndex, 0.8);
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

	splash.showProgress(m_progressIndex, 0.85);


	m_updateDialog = new UpdateDialog();
	connect(m_updateDialog, SIGNAL(enableAgainSignal(bool)), this, SLOT(enableCheckUpdates(bool)));
	checkForUpdates(false);

	splash.showProgress(m_progressIndex, 0.875);

	int loaded = 0;
	MainWindow * mainWindow = loadWindows(true, loaded);
	foreach (QString filename, m_filesToLoad) {
		loadOne(mainWindow, filename, loaded++);
	}

	//DebugDialog::debug("after m_files");

	if (loaded == 0 || !firstRun)
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

	splash.showProgress(m_progressIndex, 0.99);
	processEvents();

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

void FApplication::registerFont(const QString &fontFile, bool reallyRegister) {
	int id = QFontDatabase::addApplicationFont(fontFile);
	if(id > -1 && reallyRegister) {
		QStringList familyNames = QFontDatabase::applicationFontFamilies(id);
		QFileInfo finfo(fontFile);
		foreach (QString family, familyNames) {
			InstalledFontsNameMapper.insert(family, finfo.baseName());
			InstalledFonts << family;
			DebugDialog::debug("registering font family: "+family);
		}
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
			QHash<QString, QString> hash = prefsDialog.settings();
			foreach (QString key, hash.keys()) {
				settings.setValue(key, hash.value(key));
				if (key.compare("connectedColor") == 0) {
					QColor c(hash.value(key));
					ItemBase::setConnectedColor(c);
					foreach (QWidget * widget, m_orderedTopLevelWidgets) {
						((MainWindow *) widget)->redrawSketch();
					}
				}
				else if (key.compare("unconnectedColor") == 0) {
					QColor c(hash.value(key));
					ItemBase::setUnconnectedColor(c);
					foreach (QWidget * widget, m_orderedTopLevelWidgets) {
						((MainWindow *) widget)->redrawSketch();
					}
				}
			}
		}
	}
}

void FApplication::initSplash(FSplashScreen & splash, QPixmap & pixmap) {
	QPixmap logo(":/resources/images/fhp_logo_small.png");
	QPixmap progress(":/resources/images/splash_progressbar.png");

	m_progressIndex = splash.showPixmap(progress, QPoint(0, pixmap.height() - progress.height()));
	splash.showProgress(m_progressIndex, 0);

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
	ModelPart * modelPart = m_paletteBinModel->retrieveModelPart(ModuleIDNames::breadboardModuleIDName);
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


void FApplication::createUserDataStoreFolderStructure() {
	// make sure that the folder structure for parts and bins, exists
	QString userDataStorePath = FolderUtils::getUserDataStorePath();
	QDir dataStore(userDataStorePath);
	QStringList dataFolders = FolderUtils::getUserDataStoreFolders();
	foreach(QString folder, dataFolders) {
		if(!QFileInfo(dataStore.absolutePath()+folder).exists()) {
			QString folderaux = folder.startsWith("/")? folder.remove(0,1): folder;
			dataStore.mkpath(folder);
		}
	}

    copyBin(BinManager::MyPartsBinLocation, BinManager::MyPartsBinTemplateLocation);
    copyBin(BinManager::SearchBinLocation, BinManager::SearchBinTemplateLocation);

}

void FApplication::copyBin(const QString & dest, const QString & source) {
    if(QFileInfo(dest).exists()) return;

    // this copy action, is not working on windows, because is a resources file
    if(!QFile(source).copy(dest)) {
#ifdef Q_WS_WIN // may not be needed from qt 4.5.2 on
        DebugDialog::debug("Failed to copy a file from the resources");
        QApplication::processEvents();
        QDir binsFolder = QFileInfo(dest).dir().absolutePath();
        QStringList binFiles = binsFolder.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
        foreach(QString binName, binFiles) {
            if(binName.startsWith("qt_temp.")) {
                QString filePath = binsFolder.absoluteFilePath(binName);
                bool success = QFile(filePath).rename(dest);
                Q_UNUSED(success);
                break;
            }
        }
#endif
    }
    QFlags<QFile::Permission> ps = QFile::permissions(dest);
    QFile::setPermissions(
        dest,
        QFile::WriteOwner | QFile::WriteUser | ps
#ifdef Q_WS_WIN
        | QFile::WriteOther | QFile::WriteGroup
#endif

    );
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
	// DebugDialog::debug("updating activation");

	MainWindow * prior = m_lastTopmostWindow; 
	m_lastTopmostWindow = NULL;
	if (m_orderedTopLevelWidgets.count() > 0) {
		m_lastTopmostWindow = qobject_cast<MainWindow *>(m_orderedTopLevelWidgets.at(0));
	}

	if (prior == m_lastTopmostWindow) return;

	//DebugDialog::debug(QString("last:%1, new:%2").arg((long) prior, 0, 16).arg((long) m_lastTopmostWindow.data(), 0, 16));

	if (prior != NULL) {			
		prior->saveDocks();
	}
	if (m_lastTopmostWindow != NULL) {
		m_lastTopmostWindow->restoreDocks();
		//DebugDialog::debug("restoring active window");
	}
}

void FApplication::topLevelWidgetDestroyed(QObject * object) {
	QWidget * widget = qobject_cast<QWidget *>(object);
	if (widget) {
		m_orderedTopLevelWidgets.removeOne(widget);
	}
}

void FApplication::closeAllWindows2() {
/*
Ok, near as I can tell, here's what's going on.  When you quit fritzing, the function 
QApplication::closeAllWindows() is invoked.  This goes through the top-level window 
list in random order and calls close() on each window, until some window says "no".  
The QGraphicsProxyWidgets must contain top-level windows, and at least on the mac, their response to close() 
seems to be setVisible(false).  The random order explains why different icons 
disappear, or sometimes none at all.  

So the hack for now is to call the windows in non-random order.

Eventually, maybe the SvgIconWidget class could be rewritten so that it's not using QGraphicsProxyWidget, 
which is really not intended for hundreds of widgets.
*/


// this code modified from QApplication::closeAllWindows()


    bool did_close = true;
    QWidget *w;
    while((w = activeModalWidget()) && did_close) {
        if(!w->isVisible())
            break;
        did_close = w->close();
    }
	if (!did_close) return;

    QWidgetList list = QApplication::topLevelWidgets();
    for (int i = 0; did_close && i < list.size(); ++i) {
        w = list.at(i);
        FritzingWindow *fWindow = qobject_cast<FritzingWindow *>(w);
		if (fWindow == NULL) continue;

        if (w->isVisible() && w->windowType() != Qt::Desktop) {
            did_close = w->close();
            list = QApplication::topLevelWidgets();
            i = -1;
        }
    }
	if (!did_close) return;

    list = QApplication::topLevelWidgets();
    for (int i = 0; did_close && i < list.size(); ++i) {
        w = list.at(i);
        if (w->isVisible() && w->windowType() != Qt::Desktop) {
            did_close = w->close();
            list = QApplication::topLevelWidgets();
            i = -1;
        }
    }

}

bool FApplication::runAsService() {
	return ((FApplication *) qApp)->m_runAsService;
}

void FApplication::loadedPart(int loaded, int total) {
	if (total == 0) return;
	if (m_splash == NULL) return;



	m_splash->showProgress(m_progressIndex, LoadProgressStart + ((LoadProgressEnd - LoadProgressStart) * loaded / (double) total));
}

void FApplication::externalProcessSlot(QString &name, QString &path, QStringList &args) {
	name = m_externalProcessName;
	path = m_externalProcessPath;
	args = m_externalProcessArgs;
}

bool FApplication::notify(QObject *receiver, QEvent *e)
{
    try {
        return QApplication::notify(receiver, e);
    }
    catch (...) {
        QMessageBox::critical(NULL, tr("Fritzing failure"), tr("Fritzing caught an exception from %1 in event %2").arg(receiver->objectName()).arg(e->type()));
    }
    // save files here
    qFatal("Exiting due to exception");
    return false;
}


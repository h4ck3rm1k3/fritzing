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



#include <QApplication>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QSplashScreen>
#include <QMessageBox>
#include <QSettings>
#include <QThread>
#include <QMutex>

#include "mainwindow.h"
#include "debugdialog.h"
#include "version.h"
#include "fsplashscreen.h"
#include "fapplication.h"

// dependency injection :P
#include "referencemodel/sqlitereferencemodel.h"
#define CurrentReferenceModel SqliteReferenceModel

#define kBottomOfAlpha 206


class DoOnceThread : public QThread
 {
 public:
	 DoOnceThread(MainWindow *);

     void run();

 protected:
	MainWindow * m_mainWindow;
 };


DoOnceThread::DoOnceThread(MainWindow * mainWindow) {
	m_mainWindow = mainWindow;
}

 void DoOnceThread::run()
{
	m_mainWindow->doOnce();
}


int main(int argc, char *argv[])
{

	FApplication app(argc, argv);

	#ifdef Q_WS_MAC
		QString lib = "/../lib";
	#else
		QString lib = "/lib";
	#endif

	QString libPath = QCoreApplication::applicationDirPath() + lib;   // applicationDirPath() doesn't work until after QApplication is instantiated
	DebugDialog::debug(QString("libpath: %1").arg(libPath) );
	QCoreApplication::addLibraryPath(libPath);							// tell app where to search for plugins (jpeg export and sql lite)

    Q_INIT_RESOURCE(phoenixresources);

    QPixmap pixmap(":/resources/images/splash_2010.png");
    FSplashScreen splash(pixmap);

	QPixmap logo(":/resources/images/fhp_logo_small.png");



	QColor w(0xea, 0xf4, 0xed);
	QRect r1(45, kBottomOfAlpha, pixmap.width() - 45, 20);
	splash.showMessage(QObject::tr("<font face='Lucida Grande, Tahoma, Sans Serif' size='2' color='#eaf4ed'>"
								   "&#169; 2007-%1 Fachhochschule Potsdam"
								   "</font>")
									.arg(Version::year()),
		r1, Qt::AlignLeft | Qt::AlignTop, w);
	splash.showPixmap(logo, QPoint(5, pixmap.height() - 12));

	QRect r2(0, kBottomOfAlpha, pixmap.width() - 12, 20);
	splash.showMessage(QObject::tr("<font face='Lucida Grande, Tahoma, Sans Serif' size='2' color='#eaf4ed'>"
								   "Version %1.%2 (%3%4)"
								   "</font>")
									.arg(Version::majorVersion())
									.arg(Version::minorVersion())
									.arg(Version::modifier())
									.arg(Version::shortDate()),
		r2, Qt::AlignRight | Qt::AlignTop, w);
    splash.show();
	QApplication::processEvents();			// seems to need this (sometimes?) to display the splash screen

	QCoreApplication::setOrganizationName("Fritzing");
	QCoreApplication::setOrganizationDomain("fritzing.org");
	QCoreApplication::setApplicationName("Fritzing");
	// DebugDialog::debug("Data Location: "+QDesktopServices::storageLocation(QDesktopServices::DataLocation));

	// so we can use ViewGeometry in a Qt::QueueConnection signal
	qRegisterMetaType<ViewGeometry>("ViewGeometry");

	MainWindow::initExportConstants();
	Wire::initNames();
    ItemBase::initNames();
    ViewLayer::initNames();
    ModelPart::initNames();
    Connector::initNames();
    ZoomComboBox::loadFactors();

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
	QSettings settings5("HKEY_CLASSES_ROOT\\Fritzing\\shell\\open\\command", QSettings::NativeFormat);
	settings5.setValue(".", QString("\"%1\" \"%2\"")
								.arg(QDir::toNativeSeparators(QApplication::applicationFilePath()))
								.arg("%1") );
#endif


	ReferenceModel * referenceModel = new CurrentReferenceModel();
	PaletteModel * paletteBinModel = new PaletteModel(true, false);

	QSettings settings("Fritzing","Fritzing");
	QString prevVersion = settings.value("version").toString();
	QString currVersion = Version::versionString();
	if(prevVersion != currVersion) {
		settings.clear();
	}

	QString binToOpen = settings.value("lastBin").toString();
	binToOpen = binToOpen.isNull() || binToOpen.isEmpty() ? MainWindow::CoreBinLocation : binToOpen;

	if (!paletteBinModel->load(binToOpen, referenceModel, false)) {
		if(binToOpen == MainWindow::CoreBinLocation
		   || !paletteBinModel->load(MainWindow::CoreBinLocation, referenceModel, false)) {
			// TODO: we're really screwed, what now?
			QMessageBox::warning(NULL, QObject::tr("Fritzing"), QObject::tr("Friting cannot load the parts bin"));
			return -1;
		}
	}


	// our MainWindows use WA_DeleteOnClose so this has to be added to the heap (via new) rather than the stack (for local vars)
	MainWindow * mainWindow = new MainWindow(paletteBinModel, referenceModel);

	// on my xp machine in debug mode, 
	// sometimes the activities in doOnce cause the whole machine to peak at 100% cpu for 30 seconds or more
	// so at least the whole machine doesn't lock up anymore with doOnce in its own thread.
	
	DebugDialog::debug("starting thread");
	QMutex mutex;
	mutex.lock();
	DoOnceThread doOnceThread(mainWindow);
	doOnceThread.start();
	while (!doOnceThread.isFinished()) {
		QApplication::processEvents();
		mutex.tryLock(10);							// always fails, but effectively sleeps for 10 ms
	}
	mutex.unlock();
	DebugDialog::debug("ending thread");
	
	if(argc > 1) {
		for(int i=1; i < argc; i++) {
			mainWindow->load(argv[i]);

		}
	} else {
		if(!settings.value("lastOpenSketch").isNull()) {
			QString lastSketchPath = settings.value("lastOpenSketch").toString();
			if(QFileInfo(lastSketchPath).exists()) {
				mainWindow->load(lastSketchPath);
			} else {
				settings.remove("lastOpenSketch");
			}
		}
	}

	mainWindow->show();
	splash.finish(mainWindow);

    int result = app.exec();

    delete paletteBinModel;
    delete referenceModel;

    settings.setValue("version",currVersion);

    return result;
}

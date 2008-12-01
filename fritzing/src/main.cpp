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

#include "mainwindow.h"
#include "debugdialog.h"
#include "version.h"
#include "fsplashscreen.h"
#include "fapplication.h"

// dependency injection :P
#include "referencemodel/sqlitereferencemodel.h"
#define CurrentReferenceModel SqliteReferenceModel

#define kBottomOfAlpha 206

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
	QCoreApplication::addLibraryPath(libPath);

    Q_INIT_RESOURCE(phoenixresources);

    QPixmap pixmap(":/resources/images/splash.png");
    FSplashScreen splash(pixmap);
	splash.setTextPosition(0, kBottomOfAlpha);
	splash.showMessage(QObject::tr("<font face='Lucida Grande, Tahoma, Sans Serif' size='2' color='white'>version %1.%2 (%3%4)&nbsp;</font>")
									.arg(Version::majorVersion())
									.arg(Version::minorVersion())
									.arg(Version::modifier())
									.arg(Version::shortDate()),
		Qt::AlignRight | Qt::AlignTop, Qt::white);
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
	// associate .fz file with fritzing app on windows
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
		DebugDialog::debug("<<<c< clearing ");
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

	mainWindow->doOnce();

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

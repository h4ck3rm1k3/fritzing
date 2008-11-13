// misc Fritzing utility functions
#include "misc.h"
#include "debugdialog.h"
#include <QDesktopServices>
#include <QCoreApplication>
#include <QDir>



// finds the user parts folder based on local desktop (OS) defaults
QString getUserPartsFolder(){
	return QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/UserCreatedParts";
}

// finds a subfolder of the application directory searching backward up the tree
QDir * getApplicationSubFolder(QString search) {
	QString path = QCoreApplication::applicationDirPath();
    path += "/" + search;
	//DebugDialog::debug(QObject::tr("path %1").arg(path) );
    QDir* dir= new QDir(path);
    while (!dir->exists()) {
    	// if we're running from the debug or release folder, go up one to find things
    	dir->cdUp();
		dir->cdUp();
    	if (dir->isRoot()) return NULL;   // didn't find the parts folder

		dir->setPath(dir->absolutePath() + "/" + search);
   	}

   	return dir;
}

QString getApplicationSubFolderPath(QString search) {
	QDir * dir = getApplicationSubFolder(search);
	QString result = dir->path();
	delete dir;
	return result;
}

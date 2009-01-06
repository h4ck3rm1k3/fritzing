/****************************************************************************
**
** Copyright (C) 2007 Trolltech ASA. All rights reserved.
** Copyright (C) 2008 Dave Thorup, Bibble Labs - recursiveDeployPlugins modifications
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QApplication>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <iostream>
#include <QProcess>
#include <QDir>
#include <QRegExp>
#include <QSet>

using std::cout;
using std::endl;

class FrameworkInfo
{
public:
    QString frameworkDirectory;
    QString frameworkName;
    QString frameworkPath;
    QString binaryDirectory;
    QString binaryName;
    QString binaryPath;
    QString version;
    QString installName;
};

bool operator==(const FrameworkInfo &a, const FrameworkInfo &b)
{
    return ((a.frameworkPath == b.frameworkPath) && (a.binaryPath == b.binaryPath));
}

QDebug operator<<(QDebug debug, const FrameworkInfo &info)
{
    debug << "Framework directory" << info.frameworkDirectory << "\n";
    debug << "Framework name" << info.frameworkName << "\n";
    debug << "Framework path" << info.frameworkPath << "\n";
    debug << "Binary directory" << info.binaryDirectory << "\n";
    debug << "Binary name" << info.binaryName << "\n";
    debug << "Binary path" << info.binaryPath << "\n";
    debug << "Version" << info.version << "\n";
    debug << "Install name" << info.installName << "\n";
    return debug;
}

const QString bundleFrameworkDirectory = "Contents/Frameworks";
const QString bundleBinaryDirectory = "Contents/MacOS";
class ApplicationBundleInfo
{
public:    
    QString path;
    QString binaryPath;
};

inline QDebug operator<<(QDebug debug, const ApplicationBundleInfo &info)
{
    debug << "Application bundle path" << info.path << "\n";
    debug << "Binary path" << info.binaryPath << "\n";
    return debug;
}


FrameworkInfo parseOtoolLibratyLine(const QString &line)
{
    FrameworkInfo info;
    QStringList parts = line.simplified().split("/");

    enum State {QtPath, Name, Version, End};
    State state = QtPath;
    int part = 0;
    QString name;
    QString qtPath;
    
    // Split the line into [Qt-path]/lib/qt[Module].framework/Versions/[Version]/
    while (part < parts.count()) {
        const QString currentPart = parts.at(part).simplified() ;
        ++part;
        if (currentPart == "")
            continue;

        if (state == QtPath) {
            // Check for library name part
            if (currentPart == "lib" && parts.at(part).startsWith("Qt") && parts.at(part).endsWith(".framework")) {
                info.installName += "/" + (qtPath + "lib/").simplified();
                info.frameworkDirectory = info.installName;
                state = Name;
                continue;
            } 
			else if ( currentPart.startsWith("Qt") && !qtPath.contains( "usr/local/Trolltech" ) ) {  // If the line starts with "Qt", assume that
//			else if ( currentPart.startsWith("Qt") && !currentPart.contains( "snapshot" ) && !currentPart.contains( "-rc" ) ) {  // If the line starts with "Qt", assume that
                state = Name;                           // the app is using a binary Qt package.
                info.frameworkDirectory = "/Library/Frameworks/";
                --part;
                continue;
            }
            qtPath += (currentPart + "/");
        } if (state == Name) { 
            // remove ".framework"
            name = currentPart;
            name.chop(QString(".framework").length());
            
            state = Version;
            ++part;
            continue;
        } else if (state == Version) {
            info.version = currentPart;
            state = End;
        } else if (state == End) {
            break;
        }
    }

    info.frameworkName = name + ".framework";
    info.frameworkPath = info.frameworkDirectory/* + "/"*/ + info.frameworkName;
    info.binaryDirectory = "Versions/" + info.version; 
    info.binaryName = name;
    info.binaryPath = info.binaryDirectory + "/" + info.binaryName;
    info.installName += info.frameworkName + "/" + info.binaryPath;
    return info;
}

QString findAppBinary(const QString &appBundlePath)
{
    QString appName = QFileInfo(appBundlePath).baseName();
    QString binaryPath = appBundlePath  + "/Contents/MacOS/" + appName;
    
    if (QFile::exists(binaryPath))
        return binaryPath;
    qDebug() << "Error: Could not find bundle binary for" << appBundlePath;
    return QString();
}

QList<FrameworkInfo> getQtFrameworks(const QString &path)
{
    QProcess otool;
    otool.start("otool", QStringList() << "-L" << path);
    otool.waitForFinished();
    
    if (otool.exitCode() != 0) {
        qDebug() << otool.readAllStandardError();
    }
    
    QString output = otool.readAllStandardOutput();
    QStringList outputLines = output.split("\n");
    outputLines.removeFirst(); // remove line containing the binary path
    QList<FrameworkInfo> libraries; 
    foreach(const QString line, outputLines) {
        if (line.contains("Qt") && line.trimmed().startsWith("@executable_path") == false) {
            libraries.append(parseOtoolLibratyLine(line));
        }
    }

    return libraries;
}

QString copyFramework(const FrameworkInfo &framework, const QString path)
{
    //QProcess copy;
    // Copy the release binary only.
    const QString from = framework.frameworkPath +"/"+ framework.binaryPath;
    const QString toDir = path + "/" + bundleFrameworkDirectory + "/" + framework.frameworkName + "/" + framework.binaryDirectory;
    const QString to = toDir + "/" + framework.binaryName;
    QDir dir;
    dir.mkpath(toDir);
    
    if (QFile::exists(from) == false) {
        qDebug() << "ERROR: no file at" << from;
        return QString();
    }

    if (QFile::exists(to)) {
//        qDebug() << framework.frameworkName << "already deployed, skip";
        return QString();
    }

    
    QFile::copy(from, to);
    return to;
}

void runInstallNameTool(QStringList options)
{
    QProcess installNametool;
    installNametool.start("install_name_tool", options);
    installNametool.waitForFinished();
    if (installNametool.exitCode() != 0) {
        qDebug() << installNametool.readAllStandardError();
        qDebug() << installNametool.readAllStandardOutput();
    }
}

void changeIdentification(const QString &id, const QString &binaryPath)
{
    runInstallNameTool(QStringList() << "-id" << id << binaryPath);
}

void changeInstallName(const QString &oldName, const QString &newName, const QString &binaryPath)
{
    runInstallNameTool(QStringList() << "-change" << oldName << newName << binaryPath);
}

void deployQtFrameworks(QList<FrameworkInfo> frameworks, const QString &bundlePath, const QString &binaryPath)
{
    QStringList copiedFrameworks;
    
    while (frameworks.isEmpty() == false) {
        const FrameworkInfo framework = frameworks.takeFirst();
        copiedFrameworks.append(framework.frameworkName);

        const QString oldBinaryId = framework.installName;
        const QString newBinaryId = "@executable_path/../Frameworks/" + framework.frameworkName + "/" + framework.binaryPath;
        
        // Install_name_tool the new id into the binary
        changeInstallName(oldBinaryId, newBinaryId, binaryPath);

        // Copy farmework to app bundle.
        const QString deployedBinaryPath = copyFramework(framework, bundlePath);
        // Skip the rest if already was deployed.
        if (deployedBinaryPath == QString())
            continue;

        qDebug() << "copied" << framework.frameworkPath << "to" << bundlePath + "Contents/Frameworks";
        
        // Install_name_tool it a new id.
        changeIdentification(newBinaryId, deployedBinaryPath);
        // Check for framework dependencies
        QList<FrameworkInfo> dependencies = getQtFrameworks(deployedBinaryPath);

        foreach (FrameworkInfo dependency, dependencies) {
//            qDebug() << "dependent framework" << dependency.frameworkName;
            const QString newBinaryId = "@executable_path/../Frameworks/" + dependency.frameworkName + "/" + dependency.binaryPath;
//            const QString oldBinaryId = dependency.frameworkName + "/" + dependency.binaryPath;
			const QString oldBinaryId = dependency.installName;
            changeInstallName(oldBinaryId, newBinaryId, deployedBinaryPath);
            
            // Deploy framework if neccesary.
            if (copiedFrameworks.contains(dependency.frameworkName) == false && frameworks.contains(dependency) == false) {
                frameworks.append(dependency);
            }
        }
    }
}

void deployQtFrameworks(const QString &appBundlePath)
{
   ApplicationBundleInfo applicationBundle;
   applicationBundle.path = appBundlePath;
   applicationBundle.binaryPath = findAppBinary(appBundlePath);
   deployQtFrameworks(getQtFrameworks(applicationBundle.binaryPath), applicationBundle.path, applicationBundle.binaryPath);
}

void recursiveDeployPlugins( const QString & pluginPath, const QString & pluginDestinationPath, const ApplicationBundleInfo & applicationBundle )
{
	QDir		pluginDir( pluginPath );
    QStringList plugins;// = QDir(pluginPath).entryList();
	
	pluginDir.setFilter( QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot );
	plugins = pluginDir.entryList();
	
    qDebug() << "found plugins" << plugins;
    
    foreach (QString pluginName, plugins) 
	{
        const QString sourcePath = pluginPath + "/" + pluginName;
		
		if ( QFileInfo( sourcePath ).isDir() )
		{
			QDir	dir;
			QString	newDestPath = pluginDestinationPath + "/" + pluginName;
			
			dir.mkpath( newDestPath );
			
			recursiveDeployPlugins( sourcePath, newDestPath, applicationBundle );
		}
		else if ( pluginName.endsWith( ".dylib" ) && !pluginName.contains( "_debug" ) )	//	only copy .dylibs that are not debug libraries
		{
			const QString destinationPath = pluginDestinationPath + "/" + pluginName;
			if (QFile::copy(sourcePath, destinationPath))
				qDebug() << "copied" << sourcePath << "to" <<pluginDestinationPath ;
			
			//        qDebug() << "deploy plugin depedencies:";
			QList<FrameworkInfo> frameworks = getQtFrameworks(destinationPath);
			deployQtFrameworks(frameworks, applicationBundle.path, destinationPath);
		}
    }
	
}

void deployPlugins(const QString &appBundlePath, const QString &pluginPath)
{
    ApplicationBundleInfo applicationBundle;
    applicationBundle.path = appBundlePath;
    applicationBundle.binaryPath = findAppBinary(appBundlePath);
    
    const QString pluginDestinationPath = appBundlePath + "/" + "Contents/Plugins";
    QDir dir;
    dir.mkpath(pluginDestinationPath);
    
    qDebug() << "";
    qDebug() << "copying plugins from" << pluginPath << "to" << pluginDestinationPath;

//    QStringList plugins = QDir(pluginPath).entryList(QStringList() << "*.dylib");
	
	recursiveDeployPlugins( pluginPath, pluginDestinationPath, applicationBundle );
}

/*
void deployPlugins(const QString &appBundlePath, const QString &pluginPath)
{
    ApplicationBundleInfo applicationBundle;
    applicationBundle.path = appBundlePath;
    applicationBundle.binaryPath = findAppBinary(appBundlePath);
    
    const QString pluginDestinationPath = appBundlePath + "/" + "Contents/Plugins";
    QDir dir;
    dir.mkpath(pluginDestinationPath);
    
    qDebug() << "";
    qDebug() << "copying plugins from" << pluginPath << "to" << pluginDestinationPath;
	
    QStringList plugins = QDir(pluginPath).entryList(QStringList() << "*.dylib");
    qDebug() << "found plugins" << plugins;
    
    foreach (QString pluginName, plugins) {
        const QString sourcePath = pluginPath + "/" + pluginName;
        const QString destinationPath = pluginDestinationPath + "/" + pluginName;
        if (QFile::copy(sourcePath, destinationPath))
            qDebug() << "copied" << sourcePath << "to" <<pluginDestinationPath ;
		
		//        qDebug() << "deploy plugin depedencies:";
        QList<FrameworkInfo> frameworks = getQtFrameworks(destinationPath);
        deployQtFrameworks(frameworks, applicationBundle.path, destinationPath);
    }
}
*/
void changeQtFrameworks(const QList<FrameworkInfo> frameworks, const QString &appBinaryPath, const QString &absoluteQtPath)
{
    qDebug() << "Changing" << appBinaryPath << "to link against Qt in" << absoluteQtPath;
    QString finalQtPath = absoluteQtPath;

    if (absoluteQtPath.startsWith("/Library/Frameworks") == false)
        finalQtPath += "/lib/";

    foreach (FrameworkInfo framework, frameworks) {
        const QString oldBinaryId = framework.installName;
        const QString newBinaryId = finalQtPath + framework.frameworkName + "/" + framework.binaryPath;
        qDebug() << "Changing" << oldBinaryId << "to" << newBinaryId;
        changeInstallName(oldBinaryId, newBinaryId, appBinaryPath);
    }
}

void changeQtFrameworks(const QString appPath, const QString &qtPath)
{
    const QString appBinaryPath = findAppBinary(appPath);
    const QList<FrameworkInfo> qtFrameworks = getQtFrameworks(appBinaryPath);
    const QString absoluteQtPath = QDir(qtPath).absolutePath();
    changeQtFrameworks(qtFrameworks, appBinaryPath, absoluteQtPath);
}



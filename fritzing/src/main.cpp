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


#include <QTextStream>
#include <QFile>
#include <QtDebug>

#include "fapplication.h"
#include "debugdialog.h"

#ifdef Q_WS_WIN
#ifndef QT_NO_DEBUG
#include "windows.h"
#endif
#endif

QtMsgHandler originalMsgHandler;

void writeCrashMessage(const char * msg) {
	QString path = QCoreApplication::applicationDirPath();
	path += "/../fritzingcrash.txt";
	QFile file(path);
	if (file.open(QIODevice::Append | QIODevice::Text)) {
		QTextStream out(&file);
		out << QString(msg) << "\n";
		file.close();
	}
}

void fMessageHandler(QtMsgType type, const char *msg)
 {
	switch (type) {
		case QtDebugMsg:
			originalMsgHandler(type, msg);
			break;
		case QtWarningMsg:
			originalMsgHandler(type, msg);
			break;
		case QtCriticalMsg:
			originalMsgHandler(type, msg);
			break;
		case QtFatalMsg:
			{
				writeCrashMessage(msg);
			}

			// don't abort
			originalMsgHandler(QtWarningMsg, msg);
	}
 }



int main(int argc, char *argv[])
{
#ifdef _MSC_VER // just for the MS compiler
#define WIN_CHECK_LEAKS
#endif

#ifdef Q_WS_WIN
	originalMsgHandler = qInstallMsgHandler(fMessageHandler);
#ifndef QT_NO_DEBUG
#ifdef WIN_CHECK_LEAKS
	HANDLE hLogFile;
	hLogFile = CreateFile(L"fritzing_leak_log.txt", GENERIC_WRITE,
		  FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
		  FILE_ATTRIBUTE_NORMAL, NULL);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR, hLogFile);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, hLogFile);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, hLogFile);
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	//_CrtSetBreakAlloc(24378);					// sets a break when this memory id is allocated
#endif
#endif
#endif

	int result = 0;
	try {
		FApplication * app = new FApplication(argc, argv);

		//DebugDialog::setDebugLevel(DebugDialog::Error);
		bool firstRun = true;
		if (app->runAsService()) {
			// for example: -g C:\Users\jonathan\fritzing2\fz\Test_multiple.fz -go C:\Users\jonathan\fritzing2\fz\gerber
			result = app->serviceStartup();
		}
		else {
			do {
				result = app->startup(firstRun);
				if (result == 0) {
					result = app->exec();
					firstRun = false;
				}
			} while(result == FApplication::RestartNeeded);
		}
		app->finish();
		delete app;
	}
	catch (char const *str) {
		writeCrashMessage(str);
	}
	catch (...) {
		result = -1;
	}

	return result;
}


/****************************************************************************
**
** Copyright (C) 2007 Trolltech ASA. All rights reserved.
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
#include "../shared/shared.h"

int main(int argc, char **argv)
{
    if (argc < 2) {
        qDebug() << "DeployQt creates a deployable application by copying the neccesary Qt frameworks into the application bundle";
        qDebug() << "Usage: deployqt app-bundle [plugin-paths]";
        return 0;
    }
    
    QString appBundlePath = QString::fromLocal8Bit(argv[1]);
    if (appBundlePath.endsWith("/"))
        appBundlePath.chop(1);

    deployQtFrameworks(appBundlePath);
    
    for (int i = 2; i < argc; ++i) {
        const QString pluginPath = QString::fromLocal8Bit(argv[i]);
        deployPlugins(appBundlePath, pluginPath);
    }

    if (argc > 2) {
        qDebug() << "";
        qDebug() << "Remember to change plugin search paths to include \"qApp->applicationDirPath() + /../Plugins\"";
    }
}


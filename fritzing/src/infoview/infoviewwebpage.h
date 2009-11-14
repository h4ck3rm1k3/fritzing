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

#ifndef INFOVIEWWEBPAGE_H
#define INFOVIEWWEBPAGE_H

#include <QWebPage>
#include <QPointer>

class InfoViewWebPage : public QWebPage
{
	Q_OBJECT

public:
    InfoViewWebPage(QWidget *parent=0);

	void setCurrentItem(class ItemBase *);

protected:
    QObject * createPlugin ( const QString & classid, const QUrl & url, const QStringList & paramNames, const QStringList & paramValues );

protected:	
	QWidget *m_parent;
	QPointer<class ItemBase> m_currentItem;
};

#endif // INFOVIEWWEBPAGE_H

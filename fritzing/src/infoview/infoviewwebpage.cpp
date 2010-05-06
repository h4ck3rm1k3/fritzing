/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2010 Fachhochschule Potsdam - http://fh-potsdam.de

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

#include "infoviewwebpage.h"
#include "../items/itembase.h"
#include "htmlinfoview.h"

#include <QWebFrame>
#include <QtDebug>


InfoViewWebPage::InfoViewWebPage(HtmlInfoView * infoView, QWidget *parent) : QWebPage(parent)
{
	m_infoView = infoView;
    m_parent = parent;
	m_currentItem = NULL;
    settings()->setAttribute(QWebSettings::PluginsEnabled,true);
}

QObject * InfoViewWebPage::createPlugin(const QString &classid, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues)
{
	if (m_currentItem) {
		QObject * plugin = m_currentItem->createPlugin(m_parent, classid, url, paramNames, paramValues);
		if (plugin) return plugin;
	}

	/*
	if (m_infoView) {
		QObject * plugin = m_infoView->createPlugin(m_parent, classid, url, paramNames, paramValues);
		if (plugin) return plugin;
	}
	*/

    return QWebPage::createPlugin(classid, url, paramNames, paramValues);
}

void InfoViewWebPage::setCurrentItem(ItemBase * itemBase) {
	m_currentItem = itemBase;
}

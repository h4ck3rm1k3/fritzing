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

$Revision: 1923 $:
$Author: cohen@irascible.com $:
$Date: 2008-12-20 03:07:49 +0100 (Sat, 20 Dec 2008) $

********************************************************************/

#include "prefsdialog.h"
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QLocale>

static QVariant emptyVariant;

TranslatorListModel::TranslatorListModel(QFileInfoList & fileInfoList, QObject* parent)
: QAbstractListModel(parent) 
{
	foreach (QFileInfo fileinfo, fileInfoList) {
		QString name = fileinfo.baseName();
		name.replace("fritzing_", "");
		QLocale * locale = new QLocale(name);
		m_localeList.append(locale);
	}
}


TranslatorListModel::~TranslatorListModel() {
	foreach (QLocale * locale, m_localeList) {
		delete locale;
	}
	m_localeList.clear();
}


QVariant TranslatorListModel::data ( const QModelIndex & index, int role) const 
{
	if (role == Qt::DisplayRole && index.row() >= 0 && index.row() < m_localeList.count()) {
		return QLocale::languageToString(m_localeList.at(index.row())->language());
	}

	return emptyVariant;
	
}

int TranslatorListModel::rowCount ( const QModelIndex & parent) const 
{
	Q_UNUSED(parent);

	return m_localeList.count();
}

const QLocale * TranslatorListModel::locale( int index) 
{
	if (index < 0 || index >= m_localeList.count()) return NULL;

	return m_localeList.at(index);	
}

int TranslatorListModel::findIndex(const QString & language) {
	int ix = 0;
	foreach (QLocale * locale, m_localeList) {
		if (language.compare(locale->name()) == 0) return ix;
		ix++;
	}

	ix = 0;
	foreach (QLocale * locale, m_localeList) {
		if (locale->name().startsWith("en")) return ix;
		ix++;
	}

	return 0;

}

/////////////////////////////////////

PrefsDialog::PrefsDialog(const QString & language, QFileInfoList & list, QWidget *parent)
	: QDialog(parent)
{

	// TODO: if no translation files found, don't put up the translation part of this dialog

	m_name = language;
	m_cleared = false;

	this->setWindowTitle(QObject::tr("Preferences"));
	
	QGridLayout * gridLayout = new QGridLayout(this);

	int row = 0;
	
	QLabel * languageLabel = new QLabel(this);
	languageLabel->setMaximumWidth(175);
	languageLabel->setWordWrap(true);
	languageLabel->setText(QObject::tr("Choose your preferred language:\n"
		"Please note that a new language setting will not take effect "
		"until the next time you run Fritzing."));
	gridLayout->addWidget(languageLabel, row, 0, Qt::AlignLeft);
	
	gridLayout->setColumnMinimumWidth(1, 10);
	gridLayout->setColumnMinimumWidth(2, 10);
	gridLayout->setColumnMinimumWidth(3, 10);
	
	QComboBox* comboBox = new QComboBox(this);
	m_translatorListModel = new TranslatorListModel(list, this);
	comboBox->setModel(m_translatorListModel);
	comboBox->setCurrentIndex(m_translatorListModel->findIndex(m_name));
	gridLayout->addWidget(comboBox, row, 4, Qt::AlignRight);	
	connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeLanguage(int)));

	row++;
	
	gridLayout->setRowMinimumHeight(row, 20);

	row++;
	
	QLabel * textLabel = new QLabel(this);
	textLabel->setMaximumWidth(250);
	textLabel->setWordWrap(true);
	textLabel->setText(QObject::tr("This dialog will soon provide the ability to set some other preferences, "
							  "such as your default sketch folder and your fritzing.org login name\n"
							  "Please stay tuned."));	
	gridLayout->addWidget(textLabel, row, 0, 1, 5);

	row++;

	gridLayout->setRowMinimumHeight(row, 20);

	row++;

#ifndef QT_NO_DEBUG

	QLabel * clearLabel = new QLabel(this);
	clearLabel->setMaximumWidth(175);
	clearLabel->setWordWrap(true);
	clearLabel->setText(QObject::tr("Clear all saved settings and close this dialog (debug mode only)."));	
	gridLayout->addWidget(clearLabel, row, 0);

	QPushButton * clear = new QPushButton(QObject::tr("Clear"), this);
	clear->setMaximumWidth(220);
	gridLayout->addWidget(clear, row, 4, Qt::AlignRight);	
	connect(clear, SIGNAL(clicked()), this, SLOT(clear()));

	row++;

	gridLayout->setRowMinimumHeight(row, 20);

	row++;
#endif

	QPushButton * ok = new QPushButton(QObject::tr("OK"), this);
	ok->setMaximumWidth(120);
	gridLayout->addWidget(ok, row, 0, Qt::AlignLeft);	
	connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
	
	QPushButton * cancel = new QPushButton(QObject::tr("Cancel"), this);
	cancel->setMaximumWidth(120);
	gridLayout->addWidget(cancel, row, 4, Qt::AlignRight);
	connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

	row++;


}

PrefsDialog::~PrefsDialog()
{
}

void PrefsDialog::changeLanguage(int index) 
{
	const QLocale * locale = m_translatorListModel->locale(index);
	if (locale) {
		m_name = locale->name();
	}
}

const QString & PrefsDialog::name() {
	return m_name;
}

void PrefsDialog::clear() {
	m_cleared = true;
	accept();
}

bool PrefsDialog::cleared() {
	return m_cleared;
}


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

#include "prefsdialog.h"
#include <QFormLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QLocale>
#include <QDialogButtonBox>
#include <QGroupBox>

static QVariant emptyVariant;
QHash<QString, QString> TranslatorListModel::m_languages;

TranslatorListModel::TranslatorListModel(QFileInfoList & fileInfoList, QObject* parent)
: QAbstractListModel(parent) 
{
	if (m_languages.count() == 0) {
                m_languages.insert("english", tr("English - %1").arg("English"));
                m_languages.insert("french", tr("French - %1").arg("Français"));
                m_languages.insert("german", tr("German - %1").arg("Deutsch"));
                m_languages.insert("spanish", tr("Spanish - %1").arg("Español"));

		// put in extras so if someone does a new translation, we won't have to recompile
                m_languages.insert("dutch", tr("Dutch - %1").arg("Nederlands"));
                m_languages.insert("russian", tr("Russian - %1").arg("???????"));
                m_languages.insert("italian", tr("Italian - %1").arg("Italiano"));
                m_languages.insert("chinese-simplified", tr("Chinese Simp. - %1").arg("?? (??)"));
                m_languages.insert("chinese-traditional", tr("Chinese Trad. - %1").arg("???? (??)"));
                m_languages.insert("japanese", tr("Japanese - %1").arg("???"));
                m_languages.insert("hebrew", tr("Hebrew - %1").arg("?????"));
                m_languages.insert("arabic", tr("Arabic - %1").arg("????"));
                m_languages.insert("portuguese", tr("Portuguese - %1").arg("Português"));
                m_languages.insert("hindi", tr("Hindi - %1").arg("?????? (????)"));
                // More languages written in their own language can be found
                // at http://www.mozilla.com/en-US/firefox/all.html
	}

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
		QString languageString = QLocale::languageToString(m_localeList.at(index.row())->language());

		// QLocale::languageToString() only returns an English string, 
		// so put it through a language-dependent hash table.
		QString trLanguageString = m_languages.value(languageString.toLower(), "");
		if (trLanguageString.isEmpty()) return languageString;
		return trLanguageString;
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

	QVBoxLayout * vLayout = new QVBoxLayout(this);
	vLayout->addWidget(createLanguageForm(list));
	vLayout->addWidget(createOtherForm());

    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
	buttonBox->button(QDialogButtonBox::Ok)->setText(tr("OK"));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	vLayout->addWidget(buttonBox);

	this->setLayout(vLayout);

}

PrefsDialog::~PrefsDialog()
{
}

QWidget * PrefsDialog::createLanguageForm(QFileInfoList & list) 
{
	QGroupBox * formGroupBox = new QGroupBox(tr("Language"));
    QFormLayout *layout = new QFormLayout();

	QLabel * languageLabel = new QLabel(this);
	languageLabel->setWordWrap(true);
	languageLabel->setText(QObject::tr("<b>Language</b>"));
	
	QComboBox* comboBox = new QComboBox(this);
	m_translatorListModel = new TranslatorListModel(list, this);
	comboBox->setModel(m_translatorListModel);
	comboBox->setCurrentIndex(m_translatorListModel->findIndex(m_name));
	connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeLanguage(int)));

	layout->addRow(languageLabel, comboBox);	

	QLabel * ll = new QLabel(this);
	ll->setFixedWidth(250);
	ll->setMinimumHeight(75);
	ll->setWordWrap(true);
	ll->setText(QObject::tr("Please note that a new language setting will not take effect "
		"until the next time you run Fritzing."));
	layout->addRow(ll);

	formGroupBox->setLayout(layout);
	return formGroupBox;
}


QWidget* PrefsDialog::createOtherForm() 
{
	QGroupBox * formGroupBox = new QGroupBox(tr("Coming soon..."));
    QFormLayout *layout = new QFormLayout();

	QLabel * textLabel = new QLabel(this);
	textLabel->setMaximumWidth(250);
	textLabel->setWordWrap(true);
	textLabel->setMinimumHeight(95);
	textLabel->setText(QObject::tr("This dialog will soon provide the ability to set some other preferences, "
							  "such as your default sketch folder and your fritzing.org login name\n"
							  "Please stay tuned."));	
	layout->addRow(textLabel);

#ifndef QT_NO_DEBUG

	QLabel * clearLabel = new QLabel(this);
	clearLabel->setFixedWidth(195);
	clearLabel->setWordWrap(true);
	clearLabel->setText(QObject::tr("Clear all saved settings and close this dialog (debug mode only)."));	

	QPushButton * clear = new QPushButton(QObject::tr("Clear"), this);
	clear->setMaximumWidth(220);
	connect(clear, SIGNAL(clicked()), this, SLOT(clear()));

	layout->addRow(clearLabel, clear);	

#endif

	formGroupBox->setLayout(layout);
	return formGroupBox;
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


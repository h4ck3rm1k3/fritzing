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

#include "prefsdialog.h"
#include "../debugdialog.h"
#include "translatorlistmodel.h"
#include "../items/itembase.h"
#include "../utils/clickablelabel.h"
#include "setcolordialog.h"
#include "../sketch/zoomablegraphicsview.h"

#include <QFormLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QLocale>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QSpinBox>

#define MARGIN 5

PrefsDialog::PrefsDialog(const QString & language, QFileInfoList & list, QWidget *parent)
	: QDialog(parent)
{

	// TODO: if no translation files found, don't put up the translation part of this dialog

	m_name = language;
	m_cleared = false;
	m_wheelMapping = (int) ZoomableGraphicsView::wheelMapping();

	this->setWindowTitle(QObject::tr("Preferences"));

	QVBoxLayout * vLayout = new QVBoxLayout(this);
	vLayout->addWidget(createLanguageForm(list));
	vLayout->addWidget(createColorForm());
	vLayout->addWidget(createZoomerForm());
	vLayout->addWidget(createAutosaveForm());

#ifndef QT_NO_DEBUG
	vLayout->addWidget(createOtherForm());
#endif


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

QWidget * PrefsDialog::createZoomerForm() {
	QGroupBox * zoomer = new QGroupBox(tr("Mouse Wheel Behavior"), this );

	QHBoxLayout * zhlayout = new QHBoxLayout(this);
	zhlayout->setSpacing(5);

#ifdef Q_WS_MAC
	QString cKey = tr("Command");
#else
	QString cKey = tr("Control");
#endif

	QLabel * l = new QLabel(tr("No keys down:\n%1 key down:\nAlt key down:").arg(cKey), this);
	l->setWordWrap(true);
	zhlayout->addWidget(l);

	m_wheelLabel = new QLabel(this);
	m_wheelLabel->setWordWrap(true);
	updateWheelText();
	zhlayout->addWidget(m_wheelLabel);

	QPushButton * pushButton = new QPushButton(tr("Change Wheel Behavior"), this);
	connect(pushButton, SIGNAL(clicked()), this, SLOT(changeWheelBehavior()));
	zhlayout->addWidget(pushButton);

	zoomer->setLayout(zhlayout);

	return zoomer;
}

QWidget * PrefsDialog::createAutosaveForm() {
	QGroupBox * autosave = new QGroupBox(tr("Autosave"), this );

	QHBoxLayout * zhlayout = new QHBoxLayout(this);
	zhlayout->setSpacing(5);


	QCheckBox * box = new QCheckBox(tr("Autosave every:"));
	zhlayout->addWidget(box);

	zhlayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));

	QSpinBox * spinBox = new QSpinBox;
	spinBox->setMinimum(1);
	spinBox->setMaximum(75);
	spinBox->setMaximumWidth(80);
	zhlayout->addWidget(spinBox);

	QLabel * label = new QLabel(tr("minutes"));
	zhlayout->addWidget(label);

	autosave->setLayout(zhlayout);
	return autosave;
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
	ll->setMinimumHeight(45);
	ll->setWordWrap(true);
	ll->setText(QObject::tr("Please note that a new language setting will not take effect "
		"until the next time you run Fritzing."));
	layout->addRow(ll);

	formGroupBox->setLayout(layout);
	return formGroupBox;
}

QWidget* PrefsDialog::createColorForm() 
{
	QGroupBox * formGroupBox = new QGroupBox(tr("Colors"));
    QFormLayout *layout = new QFormLayout();

	QLabel * c1 = new QLabel(this);
	c1->setWordWrap(true);
	c1->setText(QObject::tr("<b>Connected highlight color</b>"));

	QColor connectedColor = ItemBase::connectedColor();
	ClickableLabel * cl1 = new ClickableLabel(tr("%1 (click to change...)").arg(connectedColor.name()), this);
	connect(cl1, SIGNAL(clicked()), this, SLOT(setConnectedColor()));
	cl1->setPalette(QPalette(connectedColor));
	cl1->setAutoFillBackground(true);
	cl1->setMargin(MARGIN);
	layout->addRow(c1, cl1);

	QLabel * c2 = new QLabel(this);
	c2->setWordWrap(true);
	c2->setText(QObject::tr("<b>Unconnected highlight color</b>"));

	QColor unconnectedColor = ItemBase::unconnectedColor();
	ClickableLabel * cl2 = new ClickableLabel(tr("%1 (click to change...)").arg(unconnectedColor.name()), this);
	connect(cl2, SIGNAL(clicked()), this, SLOT(setUnconnectedColor()));
	cl2->setPalette(QPalette(unconnectedColor));
	cl2->setAutoFillBackground(true);
	cl2->setMargin(MARGIN);
	layout->addRow(c2, cl2);

	formGroupBox->setLayout(layout);
	return formGroupBox;
}


QWidget* PrefsDialog::createOtherForm() 
{
	QGroupBox * formGroupBox = new QGroupBox(tr("Debug"));
    QFormLayout *layout = new QFormLayout();

	QLabel * clearLabel = new QLabel(this);
	clearLabel->setFixedWidth(195);
	clearLabel->setWordWrap(true);
	clearLabel->setText(QObject::tr("Clear all saved settings and close this dialog (debug mode only)."));	

	QPushButton * clear = new QPushButton(QObject::tr("Clear"), this);
	clear->setMaximumWidth(220);
	connect(clear, SIGNAL(clicked()), this, SLOT(clear()));

	layout->addRow(clearLabel, clear);	

	formGroupBox->setLayout(layout);
	return formGroupBox;
}

void PrefsDialog::changeLanguage(int index) 
{
	const QLocale * locale = m_translatorListModel->locale(index);
	if (locale) {
		m_name = locale->name();
		m_settings.insert("language", m_name);
	}
}

void PrefsDialog::clear() {
	m_cleared = true;
	accept();
}

bool PrefsDialog::cleared() {
	return m_cleared;
}

void PrefsDialog::setConnectedColor() {
	QColor cc = ItemBase::connectedColor();
	QColor scc = ItemBase::standardConnectedColor();

	SetColorDialog setColorDialog(tr("Connected Highlight"), cc, scc, false, this);
	int result = setColorDialog.exec();
	if (result == QDialog::Rejected) return;

	QColor c = setColorDialog.selectedColor();
	m_settings.insert("connectedColor", c.name());
	ClickableLabel * cl = qobject_cast<ClickableLabel *>(sender());
	if (cl) {
		cl->setPalette(QPalette(c));
	}
}

void PrefsDialog::setUnconnectedColor() {
	QColor cc = ItemBase::unconnectedColor();
	QColor scc = ItemBase::standardUnconnectedColor();

	SetColorDialog setColorDialog(tr("Unconnected Highlight"), cc, scc, false, this);
	int result = setColorDialog.exec();
	if (result == QDialog::Rejected) return;

	QColor c = setColorDialog.selectedColor();
	m_settings.insert("unconnectedColor", c.name());
	ClickableLabel * cl = qobject_cast<ClickableLabel *>(sender());
	if (cl) {
		cl->setPalette(QPalette(c));
	}
}

QHash<QString, QString> & PrefsDialog::settings() {
	return m_settings;
}

void PrefsDialog::changeWheelBehavior() {
	if (++m_wheelMapping >= ZoomableGraphicsView::WheelMappingCount) {
		m_wheelMapping = 0;
	}

	m_settings.insert("wheelMapping", QString("%1").arg(m_wheelMapping));
	updateWheelText();


}

void PrefsDialog::updateWheelText() {
	QString text;
	switch((ZoomableGraphicsView::WheelMapping) m_wheelMapping) {
		case ZoomableGraphicsView::MapNoZCtrlVAltH:
			text = tr("Zoom\nVertical scroll\nHorizontal scroll");
			break;
		case ZoomableGraphicsView::MapNoZCtrlHAltV:
			text = tr("Zoom\nHorizontal scroll\nVertical scroll");
			break;
		case ZoomableGraphicsView::MapNoVCtrlZAltH:
			text = tr("Vertical scroll\nZoom\nHorizontal scroll");
			break;
		case ZoomableGraphicsView::MapNoVCtrlHAltZ:
			text = tr("Vertical scroll\nHorizontal scroll\nZoom");
			break;
		case ZoomableGraphicsView::MapNoHCtrlVAltZ:
			text = tr("Horizontal scroll\nVertical scroll\nZoom");
			break;
		case ZoomableGraphicsView::MapNoHCtrlZAltV:
			text = tr("Horizontal scroll\nZoom\nVertical scroll");
			break;
		default:
			// shouldn't happen
			return;

	}
	m_wheelLabel->setText(text);
}


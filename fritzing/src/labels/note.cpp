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

#include "note.h"
#include "../graphicssvglineitem.h"
#include "../debugdialog.h"
#include "../infographicsview.h"
#include "../modelpart.h"

#include <QTextFrame>
#include <QTextFrameFormat>
#include <QApplication>
#include <QFontDatabase>
#include <QTextDocumentFragment>
#include <QTimer>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>

// TODO:
//		** search for ModelPart:: and fix up
//		check which menu items don't apply
//		** selection
//		** delete
//		** move
//		** undo delete + text
//		** resize
//		** undo resize
//		anchor
//		** undo change text
//		** undo selection
//		** undo move
//		** layers and z order
//		** hide and show layer
//		** save and load
//		format: bold, italic, size (small normal large huge), color?,
//		undo format
//		heads-up controls
//		copy/paste
//		** z-order manipulation
//		hover
//		** multiple selection
//		** icon in taskbar (why does it show up as text until you update it?)

QString Note::moduleIDName = "NoteModuleID";
const int Note::emptyMinWidth = 40;
const int Note::emptyMinHeight = 25;
const int Note::initialMinWidth = 140;
const int Note::initialMinHeight = 45;
const int borderWidth = 3;

QString Note::initialTextString;

QRegExp urlTag("<a.*href=[\"']([^\"]+[.\\s]*)[\"'].*>");    


///////////////////////////////////////

bool findText(QDomNode node, QDomNode & textNode) {
	if (node.isText()) {
		textNode = node;
		return true;
	}

	QDomNode cnode = node.firstChild();
	while (!cnode.isNull()) {
		if (findText(cnode, textNode)) return true;

		cnode = cnode.nextSibling();
	}

	return false;
}

///////////////////////////////////////

void findA(QDomElement element, QList<QDomElement> & aElements) 
{
	if (element.tagName().compare("a", Qt::CaseInsensitive) == 0) {
		aElements.append(element);
		return;
	}

	QDomElement c = element.firstChildElement();
	while (!c.isNull()) {
		findA(c, aElements);

		c = c.nextSiblingElement();
	}
}

///////////////////////////////////////

class NoteGraphicsTextItem : public QGraphicsTextItem
{
public:
	NoteGraphicsTextItem(QGraphicsItem * parent = NULL);

protected:
	void focusInEvent(QFocusEvent *);
	void focusOutEvent(QFocusEvent *);
};

NoteGraphicsTextItem::NoteGraphicsTextItem(QGraphicsItem * parent) : QGraphicsTextItem(parent)
{
	const QTextFrameFormat format = document()->rootFrame()->frameFormat();
	QTextFrameFormat altFormat(format);
	altFormat.setMargin(0);										// so document never thinks a mouse click is a move event
	document()->rootFrame()->setFrameFormat(altFormat);
}

void NoteGraphicsTextItem::focusInEvent(QFocusEvent * event) {
	QApplication::instance()->installEventFilter((Note *) this->parentItem());
	QGraphicsTextItem::focusInEvent(event);
}

void NoteGraphicsTextItem::focusOutEvent(QFocusEvent * event) {
	QApplication::instance()->removeEventFilter((Note *) this->parentItem());
	QGraphicsTextItem::focusOutEvent(event);
}

//////////////////////////////////////////

LinkDialog::LinkDialog(QWidget *parent) : QDialog(parent) 
{
	this->setWindowTitle(QObject::tr("Edit link"));

	QVBoxLayout * vLayout = new QVBoxLayout(this);

	QGroupBox * formGroupBox = new QGroupBox(this);

	QFormLayout * formLayout = new QFormLayout();

	m_urlEdit = new QLineEdit(this);
	m_urlEdit->setFixedHeight(25);
	m_urlEdit->setFixedWidth(200);
	formLayout->addRow( "url:", m_urlEdit );

	m_textEdit = new QLineEdit(this);
	m_textEdit->setFixedHeight(25);
	m_textEdit->setFixedWidth(200);
	formLayout->addRow( "text:", m_textEdit );

	formGroupBox->setLayout(formLayout);

	vLayout->addWidget(formGroupBox);

    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
	buttonBox->button(QDialogButtonBox::Ok)->setText(tr("OK"));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	vLayout->addWidget(buttonBox);

	this->setLayout(vLayout);
}

LinkDialog::~LinkDialog() {
}

void LinkDialog::setText(const QString & text) {
	m_textEdit->setText(text);
}

void LinkDialog::setUrl(const QString & url) {
	m_urlEdit->setText(url);
}

QString LinkDialog::text() {
	return m_textEdit->text();
}

QString LinkDialog::url() {
	return m_urlEdit->text();
}

/////////////////////////////////////////////

Note::Note( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier,  const ViewGeometry & viewGeometry, long id, QMenu* itemMenu)
	: ItemBase(modelPart, viewIdentifier, viewGeometry, id, itemMenu)
{
	if (initialTextString.isEmpty()) {
		initialTextString = tr("[write your note here]");

		/*
		// font test hack
		int ix = QFontDatabase::addApplicationFont ("C:/fritzing2/Isabella.ttf/Isabella.ttf");
		int ix = QFontDatabase::addApplicationFont ("/home/merun/desktop/isabella/Isabella.ttf");
		QFontDatabase database;
		QStringList families = database.families (  );
		foreach (QString string, families) {
			DebugDialog::debug(string);			// should print out the name of the font you loaded
		}
		*/
	}

	m_inResize = false;
	this->setCursor(Qt::CrossCursor);

    setFlag(QGraphicsItem::ItemIsSelectable, true);

	m_rect.setRect(0, 0, viewGeometry.rect().width(), viewGeometry.rect().height());
	m_pen.setWidth(borderWidth);
	m_pen.setBrush(QColor(0xff, 0xd5, 0x0e));

	m_brush.setColor(QColor(0xfb, 0xf7, 0xab));
	m_brush.setStyle(Qt::SolidPattern);

	setPos(m_viewGeometry.loc());

	m_resizeGrip = new QGraphicsPixmapItem(QPixmap(":/resources/images/icons/noteResizeGrip.png"));
	m_resizeGrip->setParentItem(this);
	m_resizeGrip->setCursor(Qt::SizeFDiagCursor);
	m_resizeGrip->setVisible(true);

	m_graphicsTextItem = new NoteGraphicsTextItem();
	m_graphicsTextItem->setParentItem(this);
	m_graphicsTextItem->setVisible(true);
	m_graphicsTextItem->setPlainText(initialTextString);
	m_graphicsTextItem->setTextInteractionFlags(Qt::TextEditorInteraction | Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
	m_graphicsTextItem->setCursor(Qt::IBeamCursor);
	m_graphicsTextItem->setOpenExternalLinks(true);

	/*
	// set the font here
	QFont font("Isabella");
	m_graphicsTextItem->setFont(font);
	*/


	connect(m_graphicsTextItem->document(), SIGNAL(contentsChanged()),
		this, SLOT(contentsChangedSlot()), Qt::DirectConnection);

	positionGrip();

	setAcceptHoverEvents(true);
}

void Note::saveGeometry() {
	m_viewGeometry.setRect(boundingRect());
	m_viewGeometry.setLoc(this->pos());
	m_viewGeometry.setSelected(this->isSelected());
	m_viewGeometry.setZ(this->zValue());
}

bool Note::itemMoved() {
	return (this->pos() != m_viewGeometry.loc());
}

void Note::saveInstanceLocation(QXmlStreamWriter & streamWriter) {
	QRectF rect = m_viewGeometry.rect();
	QPointF loc = m_viewGeometry.loc();
	streamWriter.writeAttribute("x", QString::number(loc.x()));
	streamWriter.writeAttribute("y", QString::number(loc.y()));
	streamWriter.writeAttribute("width", QString::number(rect.width()));
	streamWriter.writeAttribute("height", QString::number(rect.height()));
}

void Note::moveItem(ViewGeometry & viewGeometry) {
	this->setPos(viewGeometry.loc());
}

void Note::findConnectorsUnder() {
}

void Note::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(widget);

	if (m_hidden) return;

	painter->setPen(m_pen);
	painter->setBrush(m_brush);
    painter->drawRect(m_rect);

	if (option->state & QStyle::State_Selected) {
		GraphicsSvgLineItem::qt_graphicsItem_highlightSelected(this, painter, option, boundingRect(), QPainterPath(), NULL);
    }
}

QRectF Note::boundingRect() const
{
	return m_rect;
}

QPainterPath Note::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

void Note::positionGrip() {
	QSizeF gripSize = m_resizeGrip->boundingRect().size();
	QSizeF sz = this->boundingRect().size() - gripSize;
	QPointF p(sz.width(), sz.height());
	m_resizeGrip->setPos(p);
	m_graphicsTextItem->setPos(gripSize.width(), gripSize.height());
	m_graphicsTextItem->setTextWidth(sz.width() - gripSize.width());
}

void Note::mousePressEvent(QGraphicsSceneMouseEvent * event) {
	InfoGraphicsView *infographics = dynamic_cast<InfoGraphicsView *>(this->scene()->parent());
	if (infographics != NULL && infographics->spaceBarIsPressed()) {
		m_spaceBarWasPressed = true;
		event->ignore();
		return;
	}

	m_spaceBarWasPressed = false;
	QPointF p = m_resizeGrip->mapFromParent(event->pos());
	if (m_resizeGrip->boundingRect().contains(p)) {
		saveGeometry();
		m_inResize = true;
		m_resizePos = event->pos();
		event->accept();
		return;
	}

	m_inResize = false;
	ItemBase::mousePressEvent(event);
}

void Note::mouseMoveEvent(QGraphicsSceneMouseEvent * event) {
	if (m_spaceBarWasPressed) {
		event->ignore();
		return;
	}

	if (m_inResize) {
		QRectF r = m_rect;
		r.setWidth(m_viewGeometry.rect().width() + event->pos().x() - m_resizePos.x());
		r.setHeight(m_viewGeometry.rect().height() + event->pos().y() - m_resizePos.y());
		qreal minWidth = emptyMinWidth;
		qreal minHeight = emptyMinHeight;
		QSizeF gripSize = m_resizeGrip->boundingRect().size();
		QSizeF minSize = m_graphicsTextItem->document()->size() + gripSize + gripSize;
		if (minSize.height() > minHeight) minHeight = minSize.height();

		if (r.width() < minWidth) {
			r.setWidth(minWidth);
		}
		if (r.height() < minHeight) {
			r.setHeight(minHeight);
		}

		prepareGeometryChange();
		m_rect = r;
		positionGrip();

		event->accept();
		return;
	}

	ItemBase::mouseMoveEvent(event);
}

void Note::mouseReleaseEvent(QGraphicsSceneMouseEvent * event) {
	if (m_spaceBarWasPressed) {
		event->ignore();
		return;
	}

	if (m_inResize) {
		m_inResize = false;
		InfoGraphicsView *infoGraphicsView = dynamic_cast<InfoGraphicsView *>(this->scene()->parent());
		if (infoGraphicsView != NULL) {
			infoGraphicsView->noteSizeChanged(this, m_viewGeometry.rect(), m_rect);
		}
		event->accept();
		return;
	}

	ItemBase::mouseReleaseEvent(event);
}

bool Note::resizing() {
	return m_inResize;
}

void Note::contentsChangedSlot() {
	InfoGraphicsView *infoGraphicsView = dynamic_cast<InfoGraphicsView *>(this->scene()->parent());
	if (infoGraphicsView != NULL) {
		QString oldText;
		if (m_modelPart) {
			oldText = m_modelPart->instanceText();
		}

		QSizeF oldSize = m_rect.size();
		QSizeF newSize = oldSize;
		QSizeF gripSize = m_resizeGrip->boundingRect().size();
		QSizeF size = m_graphicsTextItem->document()->size();
		if (size.height() + gripSize.height() + gripSize.height() > m_rect.height()) {
			prepareGeometryChange();
			m_rect.setHeight(size.height() + gripSize.height() + gripSize.height());
			newSize.setHeight(m_rect.height());
			positionGrip();
			this->update();
		}

		infoGraphicsView->partLabelChanged(this, oldText, m_graphicsTextItem->document()->toHtml(), oldSize, newSize, false);
	}
	if (m_modelPart) {
		m_modelPart->setInstanceText(m_graphicsTextItem->document()->toHtml());
	}
}

void Note::setText(const QString & text) {
	// disconnect the signal so it doesn't fire recursively
	disconnect(m_graphicsTextItem->document(), SIGNAL(contentsChanged()),
			this, SLOT(contentsChangedSlot()));

	QString oldText = text;
	m_graphicsTextItem->document()->setHtml(text);

	connect(m_graphicsTextItem->document(), SIGNAL(contentsChanged()),
		this, SLOT(contentsChangedSlot()), Qt::DirectConnection);

}

QString Note::text() {
	return m_graphicsTextItem->document()->toHtml();
}

void Note::setSize(const QSizeF & size)
{
	prepareGeometryChange();
	m_rect.setWidth(size.width());
	m_rect.setHeight(size.height());
	positionGrip();
}

void Note::setText(const QDomElement & textElement)
{
	QString t = textElement.text();
	if (t.isEmpty()) {
		t = initialTextString;
	}
	setText(t);
}

void Note::setHidden(bool hide)
{
	ItemBase::setHidden(hide);
	m_graphicsTextItem->setVisible(!hide);
	m_resizeGrip->setVisible(!hide);
}

bool Note::eventFilter(QObject * object, QEvent * event)
{
	if (event->type() == QEvent::Shortcut || event->type() == QEvent::ShortcutOverride)
	{
		if (!object->inherits("QGraphicsView"))
		{
			event->accept();
			return true;
		}
	}

	if (event->type() == QEvent::KeyPress) {
		QKeyEvent * kevent = static_cast<QKeyEvent *>(event);
		if (kevent->matches(QKeySequence::Bold)) {
			QTextCursor textCursor = m_graphicsTextItem->textCursor();
			QTextCharFormat cf = textCursor.charFormat();
			bool isBold = cf.fontWeight() == QFont::Bold;
			QTextCharFormat textCharFormat;
			textCharFormat.setFontWeight(isBold ? QFont::Normal : QFont::Bold);
			textCursor.mergeCharFormat(textCharFormat);
			event->accept();
			return true;
		}
		if (kevent->matches(QKeySequence::Italic)) {
			QTextCursor textCursor = m_graphicsTextItem->textCursor();
			QTextCharFormat cf = textCursor.charFormat();
			QTextCharFormat textCharFormat;
			textCharFormat.setFontItalic(!cf.fontItalic());
			textCursor.mergeCharFormat(textCharFormat);
			event->accept();
			return true;
		}
		if ((kevent->key() == Qt::Key_L) && (kevent->modifiers() & Qt::ControlModifier)) {
			QTimer::singleShot(75, this, SLOT(linkDialog()));
			event->accept();
			return true;

		}
	}
	return false;
}

void Note::linkDialog() {
	QTextCursor textCursor = m_graphicsTextItem->textCursor();
	bool gotUrl = false;
	if (textCursor.anchor() == textCursor.selectionStart()) {
		// the selection returns empty since we're between characters
		// so select one character forward or one character backward 
		// to see whether we're in a url
		int wasAnchor = textCursor.anchor();
		bool atEnd = textCursor.atEnd();
		bool atStart = textCursor.atStart();
		if (!atStart) {
			textCursor.setPosition(wasAnchor - 1, QTextCursor::KeepAnchor);
			QString html = textCursor.selection().toHtml();
			if (urlTag.indexIn(html) >= 0) {
				gotUrl = true;
			}
		}
		if (!gotUrl && !atEnd) {
			textCursor.setPosition(wasAnchor + 1, QTextCursor::KeepAnchor);
			QString html = textCursor.selection().toHtml();
			if (urlTag.indexIn(html) >= 0) {
				gotUrl = true;
			}
		}
		textCursor.setPosition(wasAnchor, QTextCursor::MoveAnchor);
	}
	else {
		QString html = textCursor.selection().toHtml();
		DebugDialog::debug(html);
		if (urlTag.indexIn(html) >= 0) {
			gotUrl = true;
		}
	}

	LinkDialog ld;
	QString originalText;
	QString originalUrl;
	if (gotUrl) {
		originalUrl = urlTag.cap(1);
		ld.setUrl(originalUrl);
		QString html = m_graphicsTextItem->toHtml();

		// assumes html is in xml form
		QString errorStr;
		int errorLine;
		int errorColumn;

		QDomDocument domDocument;
		if (!domDocument.setContent(html, &errorStr, &errorLine, &errorColumn)) {
			return;
		}

		QDomElement root = domDocument.documentElement();
		if (root.isNull()) {
			return;
		}

		if (root.tagName() != "html") {
			return;
		}

		DebugDialog::debug(html);
		QList<QDomElement> aElements;
		findA(root, aElements);
		foreach (QDomElement a, aElements) {
			// TODO: if multiple hrefs point to the same url this will only find the first one
			QString href = a.attribute("href");
			if (href.isEmpty()) {
				href = a.attribute("HREF");
			}
			if (href.compare(originalUrl) == 0) {
				QDomNode textNode;
				if (findText(a, textNode)) {
					originalText = textNode.nodeValue();
					ld.setText(originalText);
					break;
				}
				else {
					return;
				}
			}
		}
	}
	int result = ld.exec();
	if (result == QDialog::Accepted) {
		if (gotUrl) {
			int from = 0;
			while (true) {
				QTextCursor cursor = m_graphicsTextItem->document()->find(originalText, from);
				if (cursor.isNull()) {
					// TODO: tell the user
					return;
				}

				QString html = cursor.selection().toHtml();
				if (html.contains(originalUrl)) {
					cursor.insertHtml(QString("<a href=\"%1\">%2</a>").arg(ld.url()).arg(ld.text()));
					break;
				}

				from = cursor.selectionEnd();
			}
		}
		else {
			textCursor.insertHtml(QString("<a href=\"%1\">%2</a>").arg(ld.url()).arg(ld.text()));
		}
	}
}


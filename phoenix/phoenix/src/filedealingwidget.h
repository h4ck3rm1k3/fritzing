/*
 * (c) Fachhochschule Potsdam
 */

#ifndef FILEDEALINGWIDGET_H_
#define FILEDEALINGWIDGET_H_

#include <QWidget>
#include <QObject>
#include <QDir>
#include <QUndoStack>

class FileDealingWidget;

typedef void (QWidget::*SetTitleFunc)(const QString&);
typedef bool (QWidget::*IsModifiedFunc)() const;
typedef void (QWidget::*SetModifiedFunc)(bool);
typedef void (FileDealingWidget::*SaveAsAuxFunc)(const QString &);

class DirtyFilesManager : public QObject {
	Q_OBJECT
	public:
		DirtyFilesManager(
				QString *fileName,
				const QString &untitledFileName,
				const QString &defaultSaveFolder,
				SetTitleFunc setTitleFunc,
				IsModifiedFunc isModifiedFunc,
				SetModifiedFunc setModifiedFunc,
				SaveAsAuxFunc saveAsAuxFunc,
				int *fileCount,
				QUndoStack *undoStack,
				QWidget *parent,
				bool appendExtensionOnTitle = true);

	protected slots:
		bool save();
		bool saveAs();
		void undoStackCleanChanged(bool isClean);

	protected:
		void setTitle();
		bool isModified();
		void setModified(bool modified);
		void saveAsAux(const QString& filename);

		bool beforeClosing(); // returns true if close, false if cancel

		bool alreadyHasExtension(const QString &fileName);
		QString getExtFromFileDialog(const QString &extOpt);
		bool isEmptyFile();

	protected:
		QUndoStack * m_undoStack;
		QString *m_fileName;
		bool m_appendExtensionOnTitle;

		QString m_untitledFileName;
		QString m_defaultSaveFolder;

		QWidget *m_widgetParent;

		SetTitleFunc m_setTitleFunc;
		IsModifiedFunc m_isModifiedFunc;
		SetModifiedFunc m_setModifiedFunc;
		SaveAsAuxFunc m_saveAsFunc;
		int *m_fileCount;

	protected:
		static QString QtFunkyPlaceholder;  // this is some weird hack Qt uses in window titles as a placeholder to set the modified state

	public:
		static const QString FritzingExtension;
};


class FileDealingWidget {
	public:
		FileDealingWidget(QWidget *meAsWidget, const QString& untitledFileName, const QString &defaultSaveFolder);
		virtual ~FileDealingWidget() {}
		void setUndoStack(QUndoStack *);

	protected:
		virtual void saveAsAux(const QString &filename) = 0;
		void setUpDirtyFilesManager(QUndoStack *, bool appendFileExt = true);

	protected:
		QString m_filename;
		int m_untitledFileCount;
		QString m_untitledFileName;
		QString m_defaultSaveFolder;

		DirtyFilesManager *m_dirtyFilesManager;

	private:
		QUndoStack *m_undoStack;
		QWidget *m_meAsWidget;
};

#endif /* FILEDEALINGWIDGET_H_ */

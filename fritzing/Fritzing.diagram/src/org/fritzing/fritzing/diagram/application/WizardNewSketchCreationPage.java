/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.application;

import java.io.File;

import org.eclipse.core.runtime.IPath;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.DirectoryDialog;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;
import org.fritzing.fritzing.diagram.part.FritzingDiagramEditorUtil;
import org.fritzing.fritzing.diagram.part.Messages;

public class WizardNewSketchCreationPage extends WizardPage {

	protected final IStructuredSelection currentSelection;
	protected String initialProjectName;
	protected IPath initialContainerFullPath;
	protected Text projectNameEditor;
	protected Text folderNameEditor;

	public WizardNewSketchCreationPage(String name,
			IStructuredSelection currentSelection) {
		super(name);
		this.currentSelection = currentSelection;
		String home = FritzingDiagramEditorUtil.getFritzingUserFolder()
				.getPath();
		if (home != null) {
			initialContainerFullPath = new Path(home);
		}
	}

	protected IStructuredSelection getSelection() {
		return currentSelection;
	}

	public String getProjectName() {
		if (projectNameEditor == null) {
			return initialProjectName;
		}
		return projectNameEditor.getText().trim();
	}

	public void setProjectName(String projectName) {
		if (projectNameEditor == null) {
			initialProjectName = projectName;
			return;
		}
		setProjectPath(getContainerFullPath(), projectName);
	}

	public IPath getContainerFullPath() {
		if (folderNameEditor == null) {
			return initialContainerFullPath;
		}
		IPath path = getProjectPath();
		if (path == null || path.isEmpty()) {
			return null;
		}
		if (path.hasTrailingSeparator()) {
			return path;
		}
		if (path.isEmpty()) {
			return null;
		}
		return path.addTrailingSeparator();
	}

	public void setContainerFullPath(IPath containerPath) {
		if (folderNameEditor == null) {
			initialContainerFullPath = containerPath;
			return;
		}
		setProjectPath(containerPath, getProjectName());
	}

	protected IPath getProjectPath() {
		String folderName = folderNameEditor.getText().trim();
		if (folderName.length() == 0) {
			return null;
		}
		return new Path(folderNameEditor.getText());
	}

	protected void setProjectPath(IPath containerPath, String projectName) {
		if (projectName == null) {
			projectName = ""; //$NON-NLS-1$
		} else {
			projectName = projectName.trim();
		}
		projectNameEditor.setText(projectName);
		if (containerPath == null) {
			folderNameEditor.setText(FritzingDiagramEditorUtil.getFritzingUserFolder().toString());
		}
		if (containerPath.hasTrailingSeparator()) {
			containerPath = containerPath.removeTrailingSeparator();
		}
		folderNameEditor.setText(containerPath.toOSString());
		setPageComplete(validatePage());
	}

	public void createControl(Composite parent) {
		Composite plate = new Composite(parent, SWT.NONE);
		plate.setLayout(new GridLayout(2, false));

		// project name
		Label titleLb = new Label(plate, SWT.NONE);
		titleLb.setText("Sketch Title");
		titleLb.setLayoutData(new GridData(SWT.BEGINNING, SWT.CENTER, false,
				false, 2, 1));
		projectNameEditor = new Text(plate, SWT.SINGLE | SWT.BORDER);
		projectNameEditor.setFont(new Font(Display.getCurrent(), Display
				.getDefault().getSystemFont().getFontData()[0].getName(), 14,
				SWT.NORMAL));
		projectNameEditor.setLayoutData(new GridData(SWT.FILL, SWT.TOP, true,
				false, 2, 1));
		
		// workspace folder
		Label folderLb = new Label(plate, SWT.NONE);
		folderLb.setText("Sketchbook Location");
		folderLb.setLayoutData(new GridData(SWT.BEGINNING, SWT.CENTER, false,
				false, 2, 1));
		folderNameEditor = new Text(plate, SWT.SINGLE | SWT.BORDER);
		folderNameEditor.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true,
				false, 1, 2));
		Button browseBtn = new Button(plate, SWT.PUSH);
		browseBtn.setText(Messages.WizardNewFileCreationPage_BrowseButton);
		browseBtn.setLayoutData(new GridData(SWT.BEGINNING, SWT.CENTER, false,
				false));

		// logic
		projectNameEditor.addModifyListener(new ModifyListener() {

			public void modifyText(ModifyEvent e) {
				setPageComplete(validatePage());
			}
		});
		folderNameEditor.addModifyListener(new ModifyListener() {

			public void modifyText(ModifyEvent e) {
				setPageComplete(validatePage());
			}
		});
		browseBtn.addSelectionListener(new SelectionListener() {

			public void widgetSelected(SelectionEvent e) {
				DirectoryDialog dialog = new DirectoryDialog(getShell(), SWT.SAVE);
				dialog.setText("Set sketchbook location");
				dialog.setMessage("Set a different sketchbook location");
				String fritzingFolder = FritzingDiagramEditorUtil
					.getFritzingUserFolder().getPath();
				if (!new File(fritzingFolder).exists())
					new File(fritzingFolder).mkdir();
				dialog.setFilterPath(fritzingFolder);
				String folderName = dialog.open();
				if (folderName != null) {
					folderNameEditor.setText(folderName);
					setPageComplete(validatePage());
				}
			}

			public void widgetDefaultSelected(SelectionEvent e) {
			}
		});

		// init
		setProjectPath(initialContainerFullPath, initialProjectName);
		setControl(plate);
	}

	public void setInitialFocus() {
		projectNameEditor.setFocus();
		projectNameEditor.selectAll();
	}
	
	protected boolean validatePage() {
		String folderName = folderNameEditor.getText().trim();
		if (folderName.length() == 0) {
			setErrorMessage(Messages.WizardNewFileCreationPage_EmptyFileNameError);
			return false;
		}
		if (!new Path("").isValidPath(folderName)) { //$NON-NLS-1$
			setErrorMessage(Messages.WizardNewFileCreationPage_InvalidFileNameError);
			return false;
		}
		String projectName = projectNameEditor.getText().trim();
		if (!new Path("").isValidSegment(projectName)) {
			setErrorMessage("Invalid sketch name");
			return false;
		}
		File combined = new File(folderName + File.separator + projectName);
		if (combined.exists()) {
			setErrorMessage("Sketch title already exists");
			return false;
		}
		setErrorMessage(null);
		return true;
	}
}

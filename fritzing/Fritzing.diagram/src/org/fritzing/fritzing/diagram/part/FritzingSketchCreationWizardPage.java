/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.part;

import org.eclipse.emf.common.util.URI;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.swt.widgets.Composite;
import org.fritzing.fritzing.diagram.application.WizardNewSketchCreationPage;

public class FritzingSketchCreationWizardPage extends WizardNewSketchCreationPage {

	public FritzingSketchCreationWizardPage(String pageName,
			IStructuredSelection selection) {
		super(pageName, selection);
	}

	public URI getBaseFolderURI() {
		return URI.createFileURI(getProjectPath().toString());
	}
	
	public void createControl(Composite parent) {
		super.createControl(parent);
		setProjectName(FritzingDiagramEditorUtil.getUniqueProjectName(
				getContainerFullPath(), getProjectName()));
		setPageComplete(validatePage());
		setInitialFocus();
	}

	protected boolean validatePage() {
		return super.validatePage();
	}
}

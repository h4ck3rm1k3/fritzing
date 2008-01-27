/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.part;

import java.lang.reflect.InvocationTargetException;

import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.emf.common.util.URI;
import org.eclipse.emf.ecore.resource.Resource;
import org.eclipse.jface.dialogs.ErrorDialog;
import org.eclipse.jface.operation.IRunnableWithProgress;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.wizard.Wizard;
import org.eclipse.ui.INewWizard;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.PartInitException;

public class FritzingSketchCreationWizard extends Wizard implements INewWizard {

	private IWorkbench workbench;
	protected IStructuredSelection selection;
	protected FritzingSketchCreationWizardPage projectPage;
	protected Resource diagram;
	private boolean openNewlyCreatedDiagramEditor = true;

	public IWorkbench getWorkbench() {
		return workbench;
	}

	public IStructuredSelection getSelection() {
		return selection;
	}

	public final Resource getDiagram() {
		return diagram;
	}

	public final boolean isOpenNewlyCreatedDiagramEditor() {
		return openNewlyCreatedDiagramEditor;
	}

	public void setOpenNewlyCreatedDiagramEditor(
			boolean openNewlyCreatedDiagramEditor) {
		this.openNewlyCreatedDiagramEditor = openNewlyCreatedDiagramEditor;
	}

	public void init(IWorkbench workbench, IStructuredSelection selection) {
		this.workbench = workbench;
		this.selection = selection;
		setWindowTitle(Messages.FritzingCreationWizardTitle);
		setNeedsProgressMonitor(true);
	}

	public void addPages() {
		projectPage = new FritzingSketchCreationWizardPage(
				"Project", getSelection()); //$NON-NLS-1$ //$NON-NLS-2$
		projectPage
				.setTitle("New Fritzing Sketch");
		projectPage
				.setDescription("Type in a title for your new Sketch");
		addPage(projectPage);
	}

	public boolean performFinish() {
		IRunnableWithProgress op = new IRunnableWithProgress() {

			public void run(IProgressMonitor monitor)
					throws InvocationTargetException, InterruptedException {

				String projectName = projectPage.getProjectName();
				URI projectFolder = projectPage.getBaseFolderURI().appendSegment(projectName);
				
				diagram = FritzingDiagramEditorUtil.createDiagram(
					projectFolder.appendSegment(projectName).appendFileExtension("fzb"), 
					projectFolder.appendSegment(projectName).appendFileExtension("fz"), 
					monitor);
				
				if (isOpenNewlyCreatedDiagramEditor() && diagram != null) {
					try {
						FritzingDiagramEditorUtil.openDiagram(diagram);
					} catch (PartInitException e) {
						ErrorDialog.openError(getContainer().getShell(),
								Messages.FritzingCreationWizardOpenEditorError,
								null, e.getStatus());
					}
				}
			}
		};
		try {
			getContainer().run(false, true, op);
		} catch (InterruptedException e) {
			return false;
		} catch (InvocationTargetException e) {
			if (e.getTargetException() instanceof CoreException) {
				ErrorDialog.openError(getContainer().getShell(),
						Messages.FritzingCreationWizardCreationError, null,
						((CoreException) e.getTargetException()).getStatus());
			} else {
				FritzingDiagramEditorPlugin.getInstance().logError(
						"Error creating diagram", e.getTargetException()); //$NON-NLS-1$
			}
			return false;
		}
		return diagram != null;
	}
}

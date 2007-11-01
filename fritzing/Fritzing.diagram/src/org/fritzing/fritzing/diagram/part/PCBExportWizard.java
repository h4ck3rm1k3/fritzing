/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.part;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.InvocationTargetException;

import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IPath;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.Platform;
import org.eclipse.core.runtime.Preferences;
import org.eclipse.emf.common.ui.URIEditorInput;
import org.eclipse.emf.common.util.URI;
import org.eclipse.emf.ecore.resource.Resource;
import org.eclipse.jface.dialogs.ErrorDialog;
import org.eclipse.jface.operation.IRunnableWithProgress;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.wizard.Wizard;
import org.eclipse.ui.IEditorInput;
import org.eclipse.ui.IEditorPart;
import org.eclipse.ui.INewWizard;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.PlatformUI;
import org.fritzing.fritzing.diagram.preferences.EaglePreferencePage;

/**
 * @generated NOT
 */
public class PCBExportWizard extends Wizard implements INewWizard {

	/**
	 * @generated NOT
	 */
	private IWorkbench workbench;

	/**
	 * @generated NOT
	 */
	protected IStructuredSelection selection;

	/**
	 * @generated NOT
	 */
	protected PCBExportWizardPage conversionFilePage;

	/**
	 * @generated NOT
	 */
	protected Resource diagram;

	/**
	 * @generated NOT
	 */
	public IWorkbench getWorkbench() {
		return workbench;
	}

	/**
	 * @generated NOT
	 */
	public IStructuredSelection getSelection() {
		return selection;
	}

	/**
	 * @generated NOT
	 */
	public final Resource getDiagram() {
		return diagram;
	}


	/**
	 * @generated NOT
	 */
	public void init(IWorkbench workbench, IStructuredSelection selection) {
		this.workbench = workbench;
		this.selection = selection;
		setWindowTitle(Messages.PCBExportWizardTitle);
		setDefaultPageImageDescriptor(FritzingDiagramEditorPlugin
				.getBundledImageDescriptor("icons/wizban/NewPCBWizard.gif")); //$NON-NLS-1$
		setNeedsProgressMonitor(true);
	}

	/**
	 * @generated NOT
	 */
	public void addPages() {
		conversionFilePage = new PCBExportWizardPage(
				"ConversionFile", getSelection(), "fritzing2eagle"); //$NON-NLS-1$ //$NON-NLS-2$
		conversionFilePage
				.setTitle(Messages.PCBExportWizard_DiagramModelFilePageTitle);
		conversionFilePage
				.setDescription(Messages.PCBExportWizard_DiagramModelFilePageDescription);
		addPage(conversionFilePage);
	}

	/**
	 * @generated NOT
	 */
	public boolean performFinish() {
		IRunnableWithProgress op = new IRunnableWithProgress() {

			public void run(IProgressMonitor monitor)
					throws InvocationTargetException, InterruptedException, NullPointerException {
			    
				// STEP 1: create Eagle script file from Fritzing files
				
			    // Fritzing diagram file = source
			    URI fritzingDiagramURI = FritzingDiagramEditorUtil.getActiveDiagramURI();
			    // XXX: run Zach's conversion here
			    URI fritzing2eagleURI = fritzingDiagramURI.trimFileExtension().
			    	appendFileExtension("fritzing2eagle");
			    
			    
			    // STEP 2: start Eagle script on the created file
			    
				// EAGLE folder
				Preferences preferences = FritzingDiagramEditorPlugin.getInstance().getPluginPreferences();
				String eagleLocation = preferences.getString(
					EaglePreferencePage.EAGLE_LOCATION) + File.separator;
				// EAGLE executable
				String eagleExec = "";
			    if(Platform.getOS().equals(Platform.OS_WIN32)) {
			    	eagleExec = "bin/eagle.exe"; 
			    }
			    else if(Platform.getOS().equals(Platform.OS_MACOSX)) {
			    	eagleExec = "EAGLE.app/Contents/MacOS/eagle"; 
			    }
			    else if(Platform.getOS().equals(Platform.OS_LINUX)) {
			    	eagleExec = "bin/eagle"; 
			    }
			    // EAGLE PCB ULP
//			    String eagleULP = Platform.getLocation().toString() + "/eagle/ulp/fritzing_master.ulp";
				String eagleULP = eagleLocation + "ulp/fritzing_master.ulp";
			    // EAGLE Schematc
				String eagleSCH = fritzingDiagramURI.trimFileExtension()
		    		.appendFileExtension("sch").toFileString();
			    // Fritzin2Eagle script file
//			    String fritzing2eagleSCR = conversionFilePage.getURI().toFileString();
//			    String fritzing2eagleSCR = fritzing2eagleURI.toFileString();
			    String fritzing2eagleSCR = fritzingDiagramURI.trimFileExtension()
			    	.appendFileExtension("scr").toFileString();
			    // EAGLE parameters
			    String eagleParams = "-C\"RUN " + 
			    	"'" + eagleULP + "' " + 
			    	"'" + fritzing2eagleSCR + "'\" " +
			    	"\"" + eagleSCH + "\"" ;	
			    // Run!
			    String command = 
			    	"\"" + eagleLocation + eagleExec + "\" " + eagleParams;
			    try {
					Process process = Runtime.getRuntime().exec(command);
				} catch (IOException e) {
					e.printStackTrace();
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
						Messages.PCBExportWizardCreationError, null,
						((CoreException) e.getTargetException()).getStatus());
			} else {
				FritzingDiagramEditorPlugin.getInstance().logError(
						"Error creating PCB file", e.getTargetException()); //$NON-NLS-1$
			}
			return false;
		} catch (NullPointerException e) {
			ErrorDialog.openError(getContainer().getShell(),
					Messages.PCBExportWizardSourceError, null,
					((CoreException) e.getCause()).getStatus());
			return false;
		}
//		return diagram != null;
		return true;
	}
}

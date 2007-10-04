/*******************************************************************************
 * Copyright (c) 2000, 2005 IBM Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     IBM Corporation - initial API and implementation
 *******************************************************************************/
package org.fritzing;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.io.ObjectOutputStream;

import org.eclipse.core.resources.IFile;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Label;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.dialogs.WizardNewFileCreationPage;
import org.eclipse.ui.ide.IDE;
import org.fritzing.model.FritzingDiagram;
import org.fritzing.model.FritzingDiagramFactory;

public class FritzingWizardPage1 
	extends WizardNewFileCreationPage 
	implements SelectionListener
{

private IWorkbench	workbench;
private static int exampleCount = 1;
private Button model1 = null;
private Button model2 = null;
private int modelSelected = 1;

public FritzingWizardPage1(IWorkbench aWorkbench, IStructuredSelection selection) {
	super("sampleFritzingPage1", selection);  //$NON-NLS-1$
	this.setTitle(FritzingMessages.CreateFritzingPage1_Title);
	this.setDescription(FritzingMessages.CreateFritzingPage1_Description);
	this.setImageDescriptor(ImageDescriptor.createFromFile(getClass(),"icons/fritzingbanner.gif"));  //$NON-NLS-1$
	this.workbench = aWorkbench;
}

public void createControl(Composite parent) {
	super.createControl(parent);
	this.setFileName("emptyModel" + exampleCount + ".fritzing");  //$NON-NLS-2$//$NON-NLS-1$
	
	Composite composite = (Composite)getControl();
	
	// sample section generation group
	Group group = new Group(composite,SWT.NONE);
	group.setLayout(new GridLayout());
	group.setText(FritzingMessages.CreateFritzingPage1_ModelNames_GroupName); 
	group.setLayoutData(new GridData(GridData.GRAB_HORIZONTAL | GridData.HORIZONTAL_ALIGN_FILL));
	
	// sample section generation checkboxes
	model1 = new Button(group,SWT.RADIO);
	model1.setText(FritzingMessages.CreateFritzingPage1_ModelNames_EmptyModelName);
	model1.addSelectionListener(this);
	model1.setSelection(true);

	model2 = new Button(group,SWT.RADIO);
	model2.setText(FritzingMessages.CreateFritzingPage1_ModelNames_FourBitAdderModelName);
	model2.addSelectionListener(this);
	
	new Label(composite,SWT.NONE);

	setPageComplete(validatePage());
}

protected InputStream getInitialContents() {
	FritzingDiagram ld = new FritzingDiagram();
	if (modelSelected == 2)
			ld = (FritzingDiagram)FritzingDiagramFactory.createLargeModel();
	ByteArrayInputStream bais = null;
	try {
		ByteArrayOutputStream baos = new ByteArrayOutputStream();
		ObjectOutputStream oos = new ObjectOutputStream(baos);
		oos.writeObject(ld);
		oos.flush();
		oos.close();
		baos.close();
		bais = new ByteArrayInputStream(baos.toByteArray());
		bais.close();
	}
	catch(Exception e) {
		e.printStackTrace();
	}
	return bais;
}

public boolean finish() {
	IFile newFile = createNewFile();
	if (newFile == null) 
		return false;  // ie.- creation was unsuccessful

	// Since the file resource was created fine, open it for editing
	// iff requested by the user
	try {
		IWorkbenchWindow dwindow = workbench.getActiveWorkbenchWindow();
		IWorkbenchPage page = dwindow.getActivePage();
		if (page != null)
			IDE.openEditor(page, newFile, true);
	} 
	catch (org.eclipse.ui.PartInitException e) {
		e.printStackTrace();
		return false;
	}
	exampleCount++;
	return true;
}

/**
 * @see org.eclipse.swt.events.SelectionListener#widgetSelected(SelectionEvent)
 */
public void widgetSelected(SelectionEvent e) {
	if( e.getSource() == model1 ){
		modelSelected = 1;
		setFileName("emptyModel" + exampleCount + ".fritzing");  //$NON-NLS-2$//$NON-NLS-1$
	} else {
		modelSelected = 2;
		setFileName("fourBitAdder" + exampleCount + ".fritzing");  //$NON-NLS-2$//$NON-NLS-1$
	}
}

/**
 * Empty method
 */
public void widgetDefaultSelected(SelectionEvent e) {
}

}

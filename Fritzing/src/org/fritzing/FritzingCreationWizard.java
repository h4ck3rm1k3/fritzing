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

import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.wizard.Wizard;
import org.eclipse.ui.INewWizard;
import org.eclipse.ui.IWorkbench;

public class FritzingCreationWizard extends Wizard implements INewWizard {
	private FritzingWizardPage1 fritzingPage = null;
	private IStructuredSelection selection;
	private IWorkbench workbench;

public void addPages(){
	fritzingPage = new FritzingWizardPage1(workbench,selection);
	addPage(fritzingPage);
}

public void init(IWorkbench aWorkbench,IStructuredSelection currentSelection) {
	workbench = aWorkbench;
	selection = currentSelection;
}

public boolean performFinish(){
	return fritzingPage.finish();
}

}

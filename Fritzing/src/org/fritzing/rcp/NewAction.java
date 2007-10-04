/*******************************************************************************
 * Copyright (c) 2005 Chris Aniszczyk
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *    Chris Aniszczyk - initial API and implementation
 *    IBM Corporation
 *******************************************************************************/

package org.fritzing.rcp;

import org.eclipse.core.runtime.Path;
import org.eclipse.jface.action.Action;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.FileDialog;
import org.eclipse.ui.IEditorInput;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.PartInitException;
import org.fritzing.FritzingEditor;
import org.fritzing.FritzingEditorInput;

public class NewAction extends Action {
	
	IWorkbenchWindow workbenchWindow;
	
	public NewAction(IWorkbenchWindow window) {
		setId("New Fritzing Sketch");
		setText("New");
		workbenchWindow = window;
	}
	
	public void run() {
		String path = openFileDialog();
		if (path != null) {
			IEditorInput input = new FritzingEditorInput(new Path(path));
			
			try {
				workbenchWindow.getActivePage().openEditor(
						input, 
						FritzingEditor.ID, 
						true);
			} catch (PartInitException e) {
				e.printStackTrace();
			}
		}
		
	}
	
	private String openFileDialog() {
		FileDialog dialog = new FileDialog(workbenchWindow.getShell(), SWT.OPEN);
		dialog.setText("??");
		dialog.setFilterExtensions(new String[] { ".fritzing" });
		return dialog.open();
	}
}

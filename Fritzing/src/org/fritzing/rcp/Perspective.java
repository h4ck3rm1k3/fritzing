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

import org.eclipse.ui.IPageLayout;
import org.eclipse.ui.IPerspectiveFactory;

public class Perspective implements IPerspectiveFactory {

	/**
	 * Creates a simply layout to demonstrate RCP usage
	 */
	public void createInitialLayout(IPageLayout layout) {
		String editorArea = layout.getEditorArea();
		layout.addView(IPageLayout.ID_PROP_SHEET, IPageLayout.LEFT, .20f, editorArea);
		layout.addView(IPageLayout.ID_OUTLINE, IPageLayout.LEFT, .20f, editorArea);
		// TODO: add console view, zoom view, separate palette view
		layout.setEditorAreaVisible(true);
		layout.setFixed(true);
	}

}

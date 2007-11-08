/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.application;

import org.eclipse.ui.IFolderLayout;
import org.eclipse.ui.IPageLayout;
import org.eclipse.ui.IPerspectiveFactory;

/**
 * @generated
 */
public class DiagramEditorPerspective implements IPerspectiveFactory {
	/**
	 * @generated
	 */
	public void createInitialLayout(IPageLayout layout) {
		layout.setEditorAreaVisible(true);
		layout
				.addPerspectiveShortcut(DiagramEditorWorkbenchAdvisor.PERSPECTIVE_ID);
		IFolderLayout bottom = layout.createFolder(
				"bottom", IPageLayout.BOTTOM, 0.8f, layout.getEditorArea()); //$NON-NLS-1$
		bottom.addView(IPageLayout.ID_PROP_SHEET);
		IFolderLayout bottomRight = layout.createFolder(
				"bottomRight", IPageLayout.RIGHT, 0.7f, "bottom"); //$NON-NLS-1$	//$NON-NLS-2$
		bottomRight.addView(IPageLayout.ID_OUTLINE);
	}
}

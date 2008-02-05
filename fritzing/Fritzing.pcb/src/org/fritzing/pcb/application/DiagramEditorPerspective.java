/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.pcb.application;

import org.eclipse.ui.IFolderLayout;
import org.eclipse.ui.IPageLayout;
import org.eclipse.ui.IPerspectiveFactory;

/**
 * @generated
 */
public class DiagramEditorPerspective implements IPerspectiveFactory {
	/**
	 * @generated NOT
	 */
	public void createInitialLayout(IPageLayout layout) {
		layout.setEditorAreaVisible(true);
		layout
				.addPerspectiveShortcut(DiagramEditorWorkbenchAdvisor.PERSPECTIVE_ID);
		
		IFolderLayout bottom = layout.createFolder(
				"bottom", IPageLayout.BOTTOM, 0.8f, layout.getEditorArea());
		bottom.addView(IPageLayout.ID_PROP_SHEET);
		
		IFolderLayout bottomMiddle = layout.createFolder(
				"bottomMiddle", IPageLayout.RIGHT, 0.5f, "bottom");
		bottomMiddle.addView("fritzing.diagram.views.ElementInfoView");
		
		IFolderLayout bottomRight = layout.createFolder(
				"bottomRight", IPageLayout.RIGHT, 0.6f, "bottomMiddle");
		bottomRight.addView(IPageLayout.ID_OUTLINE);
	}
}

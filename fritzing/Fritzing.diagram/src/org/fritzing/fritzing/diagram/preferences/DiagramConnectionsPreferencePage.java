/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.preferences;

import org.eclipse.gmf.runtime.diagram.ui.preferences.ConnectionsPreferencePage;
import org.fritzing.fritzing.diagram.part.FritzingDiagramEditorPlugin;

/**
 * @generated
 */
public class DiagramConnectionsPreferencePage extends ConnectionsPreferencePage {

	/**
	 * @generated
	 */
	public DiagramConnectionsPreferencePage() {
		setPreferenceStore(FritzingDiagramEditorPlugin.getInstance()
				.getPreferenceStore());
	}
}

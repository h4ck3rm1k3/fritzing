/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.pcb.preferences;

import org.eclipse.gmf.runtime.diagram.ui.preferences.ConnectionsPreferencePage;
import org.fritzing.pcb.part.FritzingDiagramEditorPlugin;

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

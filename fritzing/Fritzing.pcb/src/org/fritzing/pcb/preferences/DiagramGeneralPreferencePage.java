/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.pcb.preferences;

import org.eclipse.gmf.runtime.diagram.ui.preferences.DiagramsPreferencePage;
import org.eclipse.gmf.runtime.diagram.ui.preferences.IPreferenceConstants;
import org.eclipse.jface.preference.IPreferenceStore;
import org.fritzing.pcb.part.FritzingDiagramEditorPlugin;

/**
 * @generated
 */
public class DiagramGeneralPreferencePage extends DiagramsPreferencePage {

	/**
	 * @generated
	 */
	public DiagramGeneralPreferencePage() {
		setPreferenceStore(FritzingDiagramEditorPlugin.getInstance()
				.getPreferenceStore());
	}

	/**
	 * Initializes the default preference values for this preference store.
	 * 
	 * @param IPreferenceStore preferenceStore
	 * 
	 * @generated NOT
	 */
	public static void initDefaults(IPreferenceStore preferenceStore) {
		DiagramsPreferencePage.initDefaults(preferenceStore);
		preferenceStore.setDefault(
				IPreferenceConstants.PREF_SHOW_CONNECTION_HANDLES, false);
		preferenceStore.setDefault(IPreferenceConstants.PREF_SHOW_POPUP_BARS,
				false);
	}
}

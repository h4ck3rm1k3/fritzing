/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.preferences;

import org.eclipse.core.runtime.Preferences;
import org.eclipse.gef.rulers.RulerProvider;
import org.eclipse.gmf.runtime.diagram.ui.preferences.IPreferenceConstants;
import org.eclipse.gmf.runtime.diagram.ui.preferences.RulerGridPreferencePage;
import org.eclipse.jface.preference.IPreferenceStore;
import org.fritzing.fritzing.diagram.part.FritzingDiagramEditorPlugin;

/**
 * @generated
 */
public class DiagramRulersAndGridPreferencePage extends RulerGridPreferencePage {

	/**
	 * @generated
	 */
	public DiagramRulersAndGridPreferencePage() {
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
		RulerGridPreferencePage.initDefaults(preferenceStore);
		preferenceStore.setDefault(IPreferenceConstants.PREF_SHOW_RULERS, false);
		preferenceStore.setDefault(IPreferenceConstants.PREF_RULER_UNITS, RulerProvider.UNIT_INCHES);
		preferenceStore.setDefault(IPreferenceConstants.PREF_SHOW_GRID, true);
		preferenceStore.setDefault(IPreferenceConstants.PREF_SNAP_TO_GRID, true);
		preferenceStore.setDefault(IPreferenceConstants.PREF_GRID_SPACING, 0.1);
		/* more preferences (WorkbenchPreferences) are set in 
		 * FritzingDiagramEditor.configureGraphicalViewer()
		 */
	}
}

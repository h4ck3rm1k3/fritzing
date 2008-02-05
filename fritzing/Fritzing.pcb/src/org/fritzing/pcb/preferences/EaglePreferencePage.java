/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.pcb.preferences;

import org.eclipse.jface.preference.DirectoryFieldEditor;
import org.eclipse.jface.preference.FieldEditorPreferencePage;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.IWorkbenchPreferencePage;
import org.fritzing.pcb.part.FritzingDiagramEditorPlugin;

/**
 * @generated NOT
 */
public class EaglePreferencePage extends FieldEditorPreferencePage implements IWorkbenchPreferencePage {

	public static final String EAGLE_LOCATION = "prefs_eagle_location";
	
	/**
	 * @generated NOT
	 */
	public EaglePreferencePage() {
		super(GRID);
		setPreferenceStore(FritzingDiagramEditorPlugin.getInstance()
				.getPreferenceStore());
	}

	/**
	 * @generated NOT
	 */
	public void init(IWorkbench workbench) {
	}
	
	/**
	 * @generated NOT
	 */
	@Override
	protected void createFieldEditors() {
		DirectoryFieldEditor eagleEditor = new DirectoryFieldEditor(
				EAGLE_LOCATION,
				"Eagle Install Location",
				getFieldEditorParent());
		addField(eagleEditor);
	}

}

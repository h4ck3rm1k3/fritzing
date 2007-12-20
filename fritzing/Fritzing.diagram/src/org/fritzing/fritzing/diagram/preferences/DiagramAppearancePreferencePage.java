/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.preferences;

import org.eclipse.draw2d.ColorConstants;
import org.eclipse.gmf.runtime.diagram.ui.figures.DiagramColorConstants;
import org.eclipse.gmf.runtime.diagram.ui.preferences.AppearancePreferencePage;
import org.eclipse.gmf.runtime.diagram.ui.preferences.IPreferenceConstants;
import org.eclipse.jface.preference.IPreferenceStore;
import org.eclipse.jface.preference.PreferenceConverter;
import org.eclipse.swt.graphics.Color;
import org.fritzing.fritzing.diagram.part.FritzingDiagramEditorPlugin;

/**
 * @generated
 */
public class DiagramAppearancePreferencePage extends AppearancePreferencePage {

	/**
	 * @generated
	 */
	public DiagramAppearancePreferencePage() {
		setPreferenceStore(FritzingDiagramEditorPlugin.getInstance()
				.getPreferenceStore());
	}

	/**
	 * Initializes the default preference values 
	 * for this preference store.
	 * 
	 * @param store
	 */
	public static void initDefaults(IPreferenceStore store) {
		AppearancePreferencePage.initDefaults(store);

		setDefaultFontPreference(store);

		Color fontColor = ColorConstants.black;
		PreferenceConverter.setDefault(store,
				IPreferenceConstants.PREF_FONT_COLOR, fontColor.getRGB());

		Color fillColor = DiagramColorConstants.white;
		PreferenceConverter.setDefault(store,
				IPreferenceConstants.PREF_FILL_COLOR, fillColor.getRGB());

		Color lineColor = DiagramColorConstants.white;
		PreferenceConverter.setDefault(store,
				IPreferenceConstants.PREF_LINE_COLOR, lineColor.getRGB());

		Color noteFillColor = DiagramColorConstants.diagramLightYellow;
		PreferenceConverter.setDefault(store,
				IPreferenceConstants.PREF_NOTE_FILL_COLOR, noteFillColor
						.getRGB());

		Color noteLineColor = DiagramColorConstants.diagramDarkYellow;
		PreferenceConverter.setDefault(store,
				IPreferenceConstants.PREF_NOTE_LINE_COLOR, noteLineColor
						.getRGB());
	}
}

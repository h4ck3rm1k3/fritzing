/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.view.factories;

import java.util.ArrayList;
import java.util.List;

import org.eclipse.core.runtime.IAdaptable;
import org.eclipse.gmf.runtime.diagram.ui.view.factories.ConnectionViewFactory;
import org.eclipse.gmf.runtime.notation.View;
import org.fritzing.fritzing.diagram.edit.parts.TrackEditPart;
import org.fritzing.fritzing.diagram.part.FritzingVisualIDRegistry;

/**
 * @generated
 */
public class TrackViewFactory extends ConnectionViewFactory {

	/**
	 * @generated NOT
	 */
	protected List createStyles(View view) {
		List styles = new ArrayList();
//		styles.add(NotationFactory.eINSTANCE.createConnectorStyle());
//		styles.add(NotationFactory.eINSTANCE.createFontStyle());
		return styles;
	}
	
	/* FIXME: Tried to remove bendpoints here, because we don't need them for tracks.
	 * However, it seems that they are necessary.
	 */
//	protected Bendpoints createBendpoints() {
//		return null;
//	}

	/**
	 * @generated
	 */
	protected void decorateView(View containerView, View view,
			IAdaptable semanticAdapter, String semanticHint, int index,
			boolean persisted) {
		if (semanticHint == null) {
			semanticHint = FritzingVisualIDRegistry
					.getType(TrackEditPart.VISUAL_ID);
			view.setType(semanticHint);
		}
		super.decorateView(containerView, view, semanticAdapter, semanticHint,
				index, persisted);
	}
}

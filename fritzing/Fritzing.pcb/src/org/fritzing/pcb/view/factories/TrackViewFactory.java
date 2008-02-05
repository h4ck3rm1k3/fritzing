/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.pcb.view.factories;

import java.util.ArrayList;
import java.util.List;

import org.eclipse.core.runtime.IAdaptable;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.gmf.runtime.diagram.core.preferences.PreferencesHint;
import org.eclipse.gmf.runtime.diagram.ui.view.factories.ConnectionViewFactory;
import org.eclipse.gmf.runtime.notation.View;
import org.fritzing.pcb.edit.parts.TrackEditPart;
import org.fritzing.pcb.part.FritzingVisualIDRegistry;

/**
 * @generated
 */
public class TrackViewFactory extends ConnectionViewFactory {

	/*
	 * Prevent the creation of a view for Tracks completely!
	 */
	@Override
	public View createView(IAdaptable semanticAdapter, View containerView,
			String semanticHint, int index, boolean persisted,
			PreferencesHint preferencesHint) {
		return null;
	}

	/* (non-Javadoc)
	 * @see org.eclipse.gmf.runtime.diagram.ui.view.factories.AbstractViewFactory#requiresElement(org.eclipse.emf.ecore.EObject, org.eclipse.gmf.runtime.notation.View)
	 */
	@Override
	protected boolean requiresElement(EObject semanticElement, View view) {
		// TODO Auto-generated method stub
		return super.requiresElement(semanticElement, view);
	}

	/**
	 * @generated NOT
	 */
	protected List createStyles(View view) {
		List styles = new ArrayList();
//		styles.add(NotationFactory.eINSTANCE.createConnectorStyle());
//		styles.add(NotationFactory.eINSTANCE.createFontStyle());
		return styles;
	}
	
	/* Tried to remove bendpoints here, because we don't need them for tracks.
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

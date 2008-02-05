/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.pcb.view.factories;

import java.util.ArrayList;
import java.util.List;

import org.eclipse.core.runtime.IAdaptable;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.gmf.runtime.diagram.core.util.ViewUtil;
import org.eclipse.gmf.runtime.diagram.ui.view.factories.AbstractShapeViewFactory;
import org.eclipse.gmf.runtime.emf.core.util.EObjectAdapter;
import org.eclipse.gmf.runtime.notation.View;
import org.fritzing.fritzing.Part;
import org.fritzing.fritzing.Terminal;
import org.fritzing.pcb.edit.PartDefinition;
import org.fritzing.pcb.edit.PartDefinitionRegistry;
import org.fritzing.pcb.edit.parts.Terminal2EditPart;
import org.fritzing.pcb.edit.parts.TerminalName2EditPart;
import org.fritzing.pcb.part.FritzingVisualIDRegistry;

/**
 * @generated
 */
public class Terminal2ViewFactory extends AbstractShapeViewFactory {

	/**
	 * @generated NOT
	 */
	protected List createStyles(View view) {
		List styles = new ArrayList();
//		styles.add(NotationFactory.eINSTANCE.createShapeStyle());
		return styles;
	}

	/**
	 * @generated NOT
	 */
	protected void decorateView(View containerView, View view,
			IAdaptable semanticAdapter, String semanticHint, int index,
			boolean persisted) {
		if (semanticHint == null) {
			semanticHint = FritzingVisualIDRegistry
					.getType(Terminal2EditPart.VISUAL_ID);
			view.setType(semanticHint);
		}
		super.decorateView(containerView, view, semanticAdapter, semanticHint,
				index, persisted);
		
		/*
		 * Don't create the TerminalNameEditPart if the part definiton
		 * tells us not to: This reduces the .fzb file a lot for cases
		 * like the Breadboard.
		 */
		Terminal terminal = (Terminal)view.getElement();
		Part part = terminal.getParent();
		PartDefinition partDefinition = PartDefinitionRegistry.getInstance().get(
				part.getGenus() + part.getSpecies());
		if (partDefinition.getTerminalLabelVisible(terminal.getId())) {
			IAdaptable eObjectAdapter = null;
			EObject eObject = (EObject) semanticAdapter.getAdapter(EObject.class);
			if (eObject != null) {
				eObjectAdapter = new EObjectAdapter(eObject);
			}
			getViewService().createNode(
					eObjectAdapter,
					view,
					FritzingVisualIDRegistry
							.getType(TerminalName2EditPart.VISUAL_ID),
					ViewUtil.APPEND, true, getPreferencesHint());
		}
	}
}

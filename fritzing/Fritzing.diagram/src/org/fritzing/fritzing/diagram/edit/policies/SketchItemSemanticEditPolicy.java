/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.edit.policies;

import org.eclipse.emf.transaction.TransactionalEditingDomain;
import org.eclipse.gef.commands.Command;
import org.eclipse.gef.commands.UnexecutableCommand;
import org.eclipse.gmf.runtime.diagram.ui.editparts.IGraphicalEditPart;
import org.eclipse.gmf.runtime.emf.commands.core.commands.DuplicateEObjectsCommand;
import org.eclipse.gmf.runtime.emf.type.core.requests.CreateElementRequest;
import org.eclipse.gmf.runtime.emf.type.core.requests.DuplicateElementsRequest;
import org.eclipse.gmf.runtime.emf.type.core.requests.ReorientRelationshipRequest;
import org.fritzing.fritzing.FritzingPackage;
import org.fritzing.fritzing.diagram.edit.commands.GenericPartCreateCommand;
import org.fritzing.fritzing.diagram.edit.commands.LEDCreateCommand;
import org.fritzing.fritzing.diagram.edit.commands.LegReorientCommand;
import org.fritzing.fritzing.diagram.edit.commands.ResistorCreateCommand;
import org.fritzing.fritzing.diagram.edit.commands.TerminalCreateCommand;
import org.fritzing.fritzing.diagram.edit.commands.WireReorientCommand;
import org.fritzing.fritzing.diagram.edit.parts.LegEditPart;
import org.fritzing.fritzing.diagram.edit.parts.WireEditPart;
import org.fritzing.fritzing.diagram.providers.FritzingElementTypes;

/**
 * @generated
 */
public class SketchItemSemanticEditPolicy extends
		FritzingBaseItemSemanticEditPolicy {

	/**
	 * @generated
	 */
	protected Command getCreateCommand(CreateElementRequest req) {
		if (FritzingElementTypes.LED_2001 == req.getElementType()) {
			if (req.getContainmentFeature() == null) {
				req.setContainmentFeature(FritzingPackage.eINSTANCE
						.getComposite_Parts());
			}
			return getGEFWrapper(new LEDCreateCommand(req));
		}
		if (FritzingElementTypes.Resistor_2002 == req.getElementType()) {
			if (req.getContainmentFeature() == null) {
				req.setContainmentFeature(FritzingPackage.eINSTANCE
						.getComposite_Parts());
			}
			return getGEFWrapper(new ResistorCreateCommand(req));
		}
		if (FritzingElementTypes.Terminal_2003 == req.getElementType()) {
			if (req.getContainmentFeature() == null) {
				req.setContainmentFeature(FritzingPackage.eINSTANCE
						.getPart_Terminals());
			}
			return getGEFWrapper(new TerminalCreateCommand(req));
		}
		if (FritzingElementTypes.GenericPart_2004 == req.getElementType()) {
			if (req.getContainmentFeature() == null) {
				req.setContainmentFeature(FritzingPackage.eINSTANCE
						.getComposite_Parts());
			}
			return getGEFWrapper(new GenericPartCreateCommand(req));
		}
		return super.getCreateCommand(req);
	}

	/**
	 * @generated
	 */
	protected Command getDuplicateCommand(DuplicateElementsRequest req) {
		TransactionalEditingDomain editingDomain = ((IGraphicalEditPart) getHost())
				.getEditingDomain();
		return getGEFWrapper(new DuplicateAnythingCommand(editingDomain, req));
	}

	/**
	 * @generated
	 */
	private static class DuplicateAnythingCommand extends
			DuplicateEObjectsCommand {

		/**
		 * @generated
		 */
		public DuplicateAnythingCommand(
				TransactionalEditingDomain editingDomain,
				DuplicateElementsRequest req) {
			super(editingDomain, req.getLabel(), req
					.getElementsToBeDuplicated(), req
					.getAllDuplicatedElementsMap());
		}

	}
	
	// this override allows the leg to be reconnected to the sketch	
	protected Command getReorientRelationshipCommand(
			ReorientRelationshipRequest req) {
		switch (getVisualID(req)) {
			case LegEditPart.VISUAL_ID:
				return getGEFWrapper(new LegReorientCommand(req));
		}
	
		return super.getReorientRelationshipCommand(req);
	}


}

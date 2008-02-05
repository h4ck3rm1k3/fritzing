/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.pcb.part;

import org.eclipse.core.runtime.Platform;
import org.eclipse.emf.ecore.EAnnotation;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.gmf.runtime.notation.Diagram;
import org.eclipse.gmf.runtime.notation.View;
import org.fritzing.fritzing.FritzingPackage;
import org.fritzing.fritzing.Sketch;
import org.fritzing.pcb.edit.parts.GenericPartEditPart;
import org.fritzing.pcb.edit.parts.GenericPartNameEditPart;
import org.fritzing.pcb.edit.parts.LEDEditPart;
import org.fritzing.pcb.edit.parts.LEDNameEditPart;
import org.fritzing.pcb.edit.parts.LegEditPart;
import org.fritzing.pcb.edit.parts.ResistorEditPart;
import org.fritzing.pcb.edit.parts.ResistorNameEditPart;
import org.fritzing.pcb.edit.parts.SketchEditPart;
import org.fritzing.pcb.edit.parts.Terminal2EditPart;
import org.fritzing.pcb.edit.parts.TerminalEditPart;
import org.fritzing.pcb.edit.parts.TerminalName2EditPart;
import org.fritzing.pcb.edit.parts.TerminalNameEditPart;
import org.fritzing.pcb.edit.parts.TrackEditPart;
import org.fritzing.pcb.edit.parts.WireEditPart;
import org.fritzing.pcb.edit.parts.WireNameEditPart;
import org.fritzing.pcb.expressions.FritzingAbstractExpression;

/**
 * This registry is used to determine which type of visual object should be
 * created for the corresponding Diagram, Node, ChildNode or Link represented
 * by a domain model object.
 * 
 * @generated
 */
public class FritzingVisualIDRegistry {

	/**
	 * @generated
	 */
	private static final String DEBUG_KEY = FritzingDiagramEditorPlugin
			.getInstance().getBundle().getSymbolicName()
			+ "/debug/visualID"; //$NON-NLS-1$

	/**
	 * @generated
	 */
	public static int getVisualID(View view) {
		if (view instanceof Diagram) {
			if (SketchEditPart.MODEL_ID.equals(view.getType())) {
				return SketchEditPart.VISUAL_ID;
			} else {
				return -1;
			}
		}
		return org.fritzing.pcb.part.FritzingVisualIDRegistry
				.getVisualID(view.getType());
	}

	/**
	 * @generated
	 */
	public static String getModelID(View view) {
		View diagram = view.getDiagram();
		while (view != diagram) {
			EAnnotation annotation = view.getEAnnotation("Shortcut"); //$NON-NLS-1$
			if (annotation != null) {
				return (String) annotation.getDetails().get("modelID"); //$NON-NLS-1$
			}
			view = (View) view.eContainer();
		}
		return diagram != null ? diagram.getType() : null;
	}

	/**
	 * @generated
	 */
	public static int getVisualID(String type) {
		try {
			return Integer.parseInt(type);
		} catch (NumberFormatException e) {
			if (Boolean.TRUE.toString().equalsIgnoreCase(
					Platform.getDebugOption(DEBUG_KEY))) {
				FritzingDiagramEditorPlugin.getInstance().logError(
						"Unable to parse view type as a visualID number: "
								+ type);
			}
		}
		return -1;
	}

	/**
	 * @generated
	 */
	public static String getType(int visualID) {
		return String.valueOf(visualID);
	}

	/**
	 * @generated
	 */
	public static int getDiagramVisualID(EObject domainElement) {
		if (domainElement == null) {
			return -1;
		}
		if (FritzingPackage.eINSTANCE.getSketch().isSuperTypeOf(
				domainElement.eClass())
				&& isDiagram((Sketch) domainElement)) {
			return SketchEditPart.VISUAL_ID;
		}
		return -1;
	}

	/**
	 * @generated
	 */
	public static int getNodeVisualID(View containerView, EObject domainElement) {
		if (domainElement == null) {
			return -1;
		}
		String containerModelID = org.fritzing.pcb.part.FritzingVisualIDRegistry
				.getModelID(containerView);
		if (!SketchEditPart.MODEL_ID.equals(containerModelID)) {
			return -1;
		}
		int containerVisualID;
		if (SketchEditPart.MODEL_ID.equals(containerModelID)) {
			containerVisualID = org.fritzing.pcb.part.FritzingVisualIDRegistry
					.getVisualID(containerView);
		} else {
			if (containerView instanceof Diagram) {
				containerVisualID = SketchEditPart.VISUAL_ID;
			} else {
				return -1;
			}
		}
		switch (containerVisualID) {
		case LEDEditPart.VISUAL_ID:
			if (FritzingPackage.eINSTANCE.getTerminal().isSuperTypeOf(
					domainElement.eClass())) {
				return Terminal2EditPart.VISUAL_ID;
			}
			break;
		case ResistorEditPart.VISUAL_ID:
			if (FritzingPackage.eINSTANCE.getTerminal().isSuperTypeOf(
					domainElement.eClass())) {
				return Terminal2EditPart.VISUAL_ID;
			}
			break;
		case GenericPartEditPart.VISUAL_ID:
			if (FritzingPackage.eINSTANCE.getTerminal().isSuperTypeOf(
					domainElement.eClass())) {
				return Terminal2EditPart.VISUAL_ID;
			}
			break;
		case SketchEditPart.VISUAL_ID:
			if (FritzingPackage.eINSTANCE.getLED().isSuperTypeOf(
					domainElement.eClass())) {
				return LEDEditPart.VISUAL_ID;
			}
			if (FritzingPackage.eINSTANCE.getResistor().isSuperTypeOf(
					domainElement.eClass())) {
				return ResistorEditPart.VISUAL_ID;
			}
			if (FritzingPackage.eINSTANCE.getTerminal().isSuperTypeOf(
					domainElement.eClass())) {
				return TerminalEditPart.VISUAL_ID;
			}
			if (FritzingPackage.eINSTANCE.getGenericPart().isSuperTypeOf(
					domainElement.eClass())) {
				return GenericPartEditPart.VISUAL_ID;
			}
			break;
		}
		return -1;
	}

	/**
	 * @generated
	 */
	public static boolean canCreateNode(View containerView, int nodeVisualID) {
		String containerModelID = org.fritzing.pcb.part.FritzingVisualIDRegistry
				.getModelID(containerView);
		if (!SketchEditPart.MODEL_ID.equals(containerModelID)) {
			return false;
		}
		int containerVisualID;
		if (SketchEditPart.MODEL_ID.equals(containerModelID)) {
			containerVisualID = org.fritzing.pcb.part.FritzingVisualIDRegistry
					.getVisualID(containerView);
		} else {
			if (containerView instanceof Diagram) {
				containerVisualID = SketchEditPart.VISUAL_ID;
			} else {
				return false;
			}
		}
		switch (containerVisualID) {
		case LEDEditPart.VISUAL_ID:
			if (LEDNameEditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			if (Terminal2EditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			break;
		case ResistorEditPart.VISUAL_ID:
			if (ResistorNameEditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			if (Terminal2EditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			break;
		case TerminalEditPart.VISUAL_ID:
			if (TerminalNameEditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			break;
		case GenericPartEditPart.VISUAL_ID:
			if (GenericPartNameEditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			if (Terminal2EditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			break;
		case Terminal2EditPart.VISUAL_ID:
			if (TerminalName2EditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			break;
		case SketchEditPart.VISUAL_ID:
			if (LEDEditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			if (ResistorEditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			if (TerminalEditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			if (GenericPartEditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			break;
		case WireEditPart.VISUAL_ID:
			if (WireNameEditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			break;
		}
		return false;
	}

	/**
	 * @generated
	 */
	public static int getLinkWithClassVisualID(EObject domainElement) {
		if (domainElement == null) {
			return -1;
		}
		if (FritzingPackage.eINSTANCE.getWire().isSuperTypeOf(
				domainElement.eClass())) {
			return WireEditPart.VISUAL_ID;
		}
		if (FritzingPackage.eINSTANCE.getTrack().isSuperTypeOf(
				domainElement.eClass())) {
			return TrackEditPart.VISUAL_ID;
		}
		if (FritzingPackage.eINSTANCE.getLeg().isSuperTypeOf(
				domainElement.eClass())) {
			return LegEditPart.VISUAL_ID;
		}
		return -1;
	}

	/**
	 * User can change implementation of this method to handle some specific
	 * situations not covered by default logic.
	 * 
	 * @generated
	 */
	private static boolean isDiagram(Sketch element) {
		return true;
	}

	/**
	 * @generated
	 */
	private static boolean evaluate(FritzingAbstractExpression expression,
			Object element) {
		Object result = expression.evaluate(element);
		return result instanceof Boolean && ((Boolean) result).booleanValue();
	}

}

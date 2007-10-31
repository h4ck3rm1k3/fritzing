/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.part;

import org.eclipse.core.runtime.Platform;
import org.eclipse.emf.ecore.EAnnotation;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.gmf.runtime.notation.Diagram;
import org.eclipse.gmf.runtime.notation.View;
import org.fritzing.fritzing.FritzingPackage;
import org.fritzing.fritzing.Sketch;
import org.fritzing.fritzing.diagram.edit.parts.ArduinoEditPart;
import org.fritzing.fritzing.diagram.edit.parts.ArduinoIdEditPart;
import org.fritzing.fritzing.diagram.edit.parts.BreadboardEditPart;
import org.fritzing.fritzing.diagram.edit.parts.BreadboardIdEditPart;
import org.fritzing.fritzing.diagram.edit.parts.ButtonEditPart;
import org.fritzing.fritzing.diagram.edit.parts.ButtonIdEditPart;
import org.fritzing.fritzing.diagram.edit.parts.LEDEditPart;
import org.fritzing.fritzing.diagram.edit.parts.LEDIdEditPart;
import org.fritzing.fritzing.diagram.edit.parts.ModuleEditPart;
import org.fritzing.fritzing.diagram.edit.parts.ModuleIdEditPart;
import org.fritzing.fritzing.diagram.edit.parts.ResistorEditPart;
import org.fritzing.fritzing.diagram.edit.parts.ResistorIdEditPart;
import org.fritzing.fritzing.diagram.edit.parts.SketchEditPart;
import org.fritzing.fritzing.diagram.edit.parts.Terminal2EditPart;
import org.fritzing.fritzing.diagram.edit.parts.TerminalEditPart;
import org.fritzing.fritzing.diagram.edit.parts.TerminalName2EditPart;
import org.fritzing.fritzing.diagram.edit.parts.TerminalNameEditPart;
import org.fritzing.fritzing.diagram.edit.parts.WireEditPart;
import org.fritzing.fritzing.diagram.edit.parts.WireIdEditPart;
import org.fritzing.fritzing.diagram.expressions.FritzingAbstractExpression;

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
		return org.fritzing.fritzing.diagram.part.FritzingVisualIDRegistry
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
		String containerModelID = org.fritzing.fritzing.diagram.part.FritzingVisualIDRegistry
				.getModelID(containerView);
		if (!SketchEditPart.MODEL_ID.equals(containerModelID)) {
			return -1;
		}
		int containerVisualID;
		if (SketchEditPart.MODEL_ID.equals(containerModelID)) {
			containerVisualID = org.fritzing.fritzing.diagram.part.FritzingVisualIDRegistry
					.getVisualID(containerView);
		} else {
			if (containerView instanceof Diagram) {
				containerVisualID = SketchEditPart.VISUAL_ID;
			} else {
				return -1;
			}
		}
		switch (containerVisualID) {
		case ModuleEditPart.VISUAL_ID:
			if (FritzingPackage.eINSTANCE.getTerminal().isSuperTypeOf(
					domainElement.eClass())) {
				return Terminal2EditPart.VISUAL_ID;
			}
			break;
		case ArduinoEditPart.VISUAL_ID:
			if (FritzingPackage.eINSTANCE.getTerminal().isSuperTypeOf(
					domainElement.eClass())) {
				return Terminal2EditPart.VISUAL_ID;
			}
			break;
		case BreadboardEditPart.VISUAL_ID:
			if (FritzingPackage.eINSTANCE.getTerminal().isSuperTypeOf(
					domainElement.eClass())) {
				return Terminal2EditPart.VISUAL_ID;
			}
			break;
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
		case ButtonEditPart.VISUAL_ID:
			if (FritzingPackage.eINSTANCE.getTerminal().isSuperTypeOf(
					domainElement.eClass())) {
				return Terminal2EditPart.VISUAL_ID;
			}
			break;
		case SketchEditPart.VISUAL_ID:
			if (FritzingPackage.eINSTANCE.getModule().isSuperTypeOf(
					domainElement.eClass())) {
				return ModuleEditPart.VISUAL_ID;
			}
			if (FritzingPackage.eINSTANCE.getArduino().isSuperTypeOf(
					domainElement.eClass())) {
				return ArduinoEditPart.VISUAL_ID;
			}
			if (FritzingPackage.eINSTANCE.getBreadboard().isSuperTypeOf(
					domainElement.eClass())) {
				return BreadboardEditPart.VISUAL_ID;
			}
			if (FritzingPackage.eINSTANCE.getLED().isSuperTypeOf(
					domainElement.eClass())) {
				return LEDEditPart.VISUAL_ID;
			}
			if (FritzingPackage.eINSTANCE.getResistor().isSuperTypeOf(
					domainElement.eClass())) {
				return ResistorEditPart.VISUAL_ID;
			}
			if (FritzingPackage.eINSTANCE.getButton().isSuperTypeOf(
					domainElement.eClass())) {
				return ButtonEditPart.VISUAL_ID;
			}
			if (FritzingPackage.eINSTANCE.getTerminal().isSuperTypeOf(
					domainElement.eClass())) {
				return TerminalEditPart.VISUAL_ID;
			}
			break;
		}
		return -1;
	}

	/**
	 * @generated
	 */
	public static boolean canCreateNode(View containerView, int nodeVisualID) {
		String containerModelID = org.fritzing.fritzing.diagram.part.FritzingVisualIDRegistry
				.getModelID(containerView);
		if (!SketchEditPart.MODEL_ID.equals(containerModelID)) {
			return false;
		}
		int containerVisualID;
		if (SketchEditPart.MODEL_ID.equals(containerModelID)) {
			containerVisualID = org.fritzing.fritzing.diagram.part.FritzingVisualIDRegistry
					.getVisualID(containerView);
		} else {
			if (containerView instanceof Diagram) {
				containerVisualID = SketchEditPart.VISUAL_ID;
			} else {
				return false;
			}
		}
		switch (containerVisualID) {
		case ModuleEditPart.VISUAL_ID:
			if (ModuleIdEditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			if (Terminal2EditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			break;
		case ArduinoEditPart.VISUAL_ID:
			if (ArduinoIdEditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			if (Terminal2EditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			break;
		case BreadboardEditPart.VISUAL_ID:
			if (BreadboardIdEditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			if (Terminal2EditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			break;
		case LEDEditPart.VISUAL_ID:
			if (LEDIdEditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			if (Terminal2EditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			break;
		case ResistorEditPart.VISUAL_ID:
			if (ResistorIdEditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			if (Terminal2EditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			break;
		case ButtonEditPart.VISUAL_ID:
			if (ButtonIdEditPart.VISUAL_ID == nodeVisualID) {
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
		case Terminal2EditPart.VISUAL_ID:
			if (TerminalName2EditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			break;
		case SketchEditPart.VISUAL_ID:
			if (ModuleEditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			if (ArduinoEditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			if (BreadboardEditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			if (LEDEditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			if (ResistorEditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			if (ButtonEditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			if (TerminalEditPart.VISUAL_ID == nodeVisualID) {
				return true;
			}
			break;
		case WireEditPart.VISUAL_ID:
			if (WireIdEditPart.VISUAL_ID == nodeVisualID) {
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

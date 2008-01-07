/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.providers;

import org.eclipse.core.runtime.IAdaptable;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.gmf.runtime.diagram.core.providers.AbstractViewProvider;
import org.eclipse.gmf.runtime.emf.type.core.IElementType;
import org.eclipse.gmf.runtime.emf.type.core.IHintedType;
import org.eclipse.gmf.runtime.notation.View;
import org.fritzing.fritzing.diagram.edit.parts.GenericPartEditPart;
import org.fritzing.fritzing.diagram.edit.parts.GenericPartNameEditPart;
import org.fritzing.fritzing.diagram.edit.parts.LEDEditPart;
import org.fritzing.fritzing.diagram.edit.parts.LEDNameEditPart;
import org.fritzing.fritzing.diagram.edit.parts.ResistorEditPart;
import org.fritzing.fritzing.diagram.edit.parts.ResistorNameEditPart;
import org.fritzing.fritzing.diagram.edit.parts.SketchEditPart;
import org.fritzing.fritzing.diagram.edit.parts.Terminal2EditPart;
import org.fritzing.fritzing.diagram.edit.parts.TerminalEditPart;
import org.fritzing.fritzing.diagram.edit.parts.TerminalName2EditPart;
import org.fritzing.fritzing.diagram.edit.parts.TerminalNameEditPart;
import org.fritzing.fritzing.diagram.edit.parts.TrackEditPart;
import org.fritzing.fritzing.diagram.edit.parts.TrackNameEditPart;
import org.fritzing.fritzing.diagram.edit.parts.WireEditPart;
import org.fritzing.fritzing.diagram.edit.parts.WireNameEditPart;
import org.fritzing.fritzing.diagram.part.FritzingVisualIDRegistry;
import org.fritzing.fritzing.diagram.view.factories.GenericPartNameViewFactory;
import org.fritzing.fritzing.diagram.view.factories.GenericPartViewFactory;
import org.fritzing.fritzing.diagram.view.factories.LEDNameViewFactory;
import org.fritzing.fritzing.diagram.view.factories.LEDViewFactory;
import org.fritzing.fritzing.diagram.view.factories.ResistorNameViewFactory;
import org.fritzing.fritzing.diagram.view.factories.ResistorViewFactory;
import org.fritzing.fritzing.diagram.view.factories.SketchViewFactory;
import org.fritzing.fritzing.diagram.view.factories.Terminal2ViewFactory;
import org.fritzing.fritzing.diagram.view.factories.TerminalName2ViewFactory;
import org.fritzing.fritzing.diagram.view.factories.TerminalNameViewFactory;
import org.fritzing.fritzing.diagram.view.factories.TerminalViewFactory;
import org.fritzing.fritzing.diagram.view.factories.TrackNameViewFactory;
import org.fritzing.fritzing.diagram.view.factories.TrackViewFactory;
import org.fritzing.fritzing.diagram.view.factories.WireNameViewFactory;
import org.fritzing.fritzing.diagram.view.factories.WireViewFactory;

/**
 * @generated
 */
public class FritzingViewProvider extends AbstractViewProvider {

	/**
	 * @generated
	 */
	protected Class getDiagramViewClass(IAdaptable semanticAdapter,
			String diagramKind) {
		EObject semanticElement = getSemanticElement(semanticAdapter);
		if (SketchEditPart.MODEL_ID.equals(diagramKind)
				&& FritzingVisualIDRegistry.getDiagramVisualID(semanticElement) != -1) {
			return SketchViewFactory.class;
		}
		return null;
	}

	/**
	 * @generated
	 */
	protected Class getNodeViewClass(IAdaptable semanticAdapter,
			View containerView, String semanticHint) {
		if (containerView == null) {
			return null;
		}
		IElementType elementType = getSemanticElementType(semanticAdapter);
		EObject domainElement = getSemanticElement(semanticAdapter);
		int visualID;
		if (semanticHint == null) {
			// Semantic hint is not specified. Can be a result of call from CanonicalEditPolicy.
			// In this situation there should be NO elementType, visualID will be determined
			// by VisualIDRegistry.getNodeVisualID() for domainElement.
			if (elementType != null || domainElement == null) {
				return null;
			}
			visualID = FritzingVisualIDRegistry.getNodeVisualID(containerView,
					domainElement);
		} else {
			visualID = FritzingVisualIDRegistry.getVisualID(semanticHint);
			if (elementType != null) {
				// Semantic hint is specified together with element type.
				// Both parameters should describe exactly the same diagram element.
				// In addition we check that visualID returned by VisualIDRegistry.getNodeVisualID() for
				// domainElement (if specified) is the same as in element type.
				if (!FritzingElementTypes.isKnownElementType(elementType)
						|| (!(elementType instanceof IHintedType))) {
					return null; // foreign element type
				}
				String elementTypeHint = ((IHintedType) elementType)
						.getSemanticHint();
				if (!semanticHint.equals(elementTypeHint)) {
					return null; // if semantic hint is specified it should be the same as in element type
				}
				if (domainElement != null
						&& visualID != FritzingVisualIDRegistry
								.getNodeVisualID(containerView, domainElement)) {
					return null; // visual id for node EClass should match visual id from element type
				}
			} else {
				// Element type is not specified. Domain element should be present (except pure design elements).
				// This method is called with EObjectAdapter as parameter from:
				//   - ViewService.createNode(View container, EObject eObject, String type, PreferencesHint preferencesHint) 
				//   - generated ViewFactory.decorateView() for parent element
				if (!SketchEditPart.MODEL_ID.equals(FritzingVisualIDRegistry
						.getModelID(containerView))) {
					return null; // foreign diagram
				}
				switch (visualID) {
				case LEDEditPart.VISUAL_ID:
				case ResistorEditPart.VISUAL_ID:
				case GenericPartEditPart.VISUAL_ID:
				case Terminal2EditPart.VISUAL_ID:
				case TerminalEditPart.VISUAL_ID:
					if (domainElement == null
							|| visualID != FritzingVisualIDRegistry
									.getNodeVisualID(containerView,
											domainElement)) {
						return null; // visual id in semantic hint should match visual id for domain element
					}
					break;
				case LEDNameEditPart.VISUAL_ID:
					if (LEDEditPart.VISUAL_ID != FritzingVisualIDRegistry
							.getVisualID(containerView)
							|| containerView.getElement() != domainElement) {
						return null; // wrong container
					}
					break;
				case ResistorNameEditPart.VISUAL_ID:
					if (ResistorEditPart.VISUAL_ID != FritzingVisualIDRegistry
							.getVisualID(containerView)
							|| containerView.getElement() != domainElement) {
						return null; // wrong container
					}
					break;
				case TerminalNameEditPart.VISUAL_ID:
					if (TerminalEditPart.VISUAL_ID != FritzingVisualIDRegistry
							.getVisualID(containerView)
							|| containerView.getElement() != domainElement) {
						return null; // wrong container
					}
					break;
				case GenericPartNameEditPart.VISUAL_ID:
					if (GenericPartEditPart.VISUAL_ID != FritzingVisualIDRegistry
							.getVisualID(containerView)
							|| containerView.getElement() != domainElement) {
						return null; // wrong container
					}
					break;
				case TerminalName2EditPart.VISUAL_ID:
					if (Terminal2EditPart.VISUAL_ID != FritzingVisualIDRegistry
							.getVisualID(containerView)
							|| containerView.getElement() != domainElement) {
						return null; // wrong container
					}
					break;
				case WireNameEditPart.VISUAL_ID:
					if (WireEditPart.VISUAL_ID != FritzingVisualIDRegistry
							.getVisualID(containerView)
							|| containerView.getElement() != domainElement) {
						return null; // wrong container
					}
					break;
				case TrackNameEditPart.VISUAL_ID:
					if (TrackEditPart.VISUAL_ID != FritzingVisualIDRegistry
							.getVisualID(containerView)
							|| containerView.getElement() != domainElement) {
						return null; // wrong container
					}
					break;
				default:
					return null;
				}
			}
		}
		return getNodeViewClass(containerView, visualID);
	}

	/**
	 * @generated
	 */
	protected Class getNodeViewClass(View containerView, int visualID) {
		if (containerView == null
				|| !FritzingVisualIDRegistry.canCreateNode(containerView,
						visualID)) {
			return null;
		}
		switch (visualID) {
		case LEDEditPart.VISUAL_ID:
			return LEDViewFactory.class;
		case LEDNameEditPart.VISUAL_ID:
			return LEDNameViewFactory.class;
		case ResistorEditPart.VISUAL_ID:
			return ResistorViewFactory.class;
		case ResistorNameEditPart.VISUAL_ID:
			return ResistorNameViewFactory.class;
		case TerminalEditPart.VISUAL_ID:
			return TerminalViewFactory.class;
		case TerminalNameEditPart.VISUAL_ID:
			return TerminalNameViewFactory.class;
		case GenericPartEditPart.VISUAL_ID:
			return GenericPartViewFactory.class;
		case GenericPartNameEditPart.VISUAL_ID:
			return GenericPartNameViewFactory.class;
		case Terminal2EditPart.VISUAL_ID:
			return Terminal2ViewFactory.class;
		case TerminalName2EditPart.VISUAL_ID:
			return TerminalName2ViewFactory.class;
		case WireNameEditPart.VISUAL_ID:
			return WireNameViewFactory.class;
		case TrackNameEditPart.VISUAL_ID:
			return TrackNameViewFactory.class;
		}
		return null;
	}

	/**
	 * @generated
	 */
	protected Class getEdgeViewClass(IAdaptable semanticAdapter,
			View containerView, String semanticHint) {
		IElementType elementType = getSemanticElementType(semanticAdapter);
		if (!FritzingElementTypes.isKnownElementType(elementType)
				|| (!(elementType instanceof IHintedType))) {
			return null; // foreign element type
		}
		String elementTypeHint = ((IHintedType) elementType).getSemanticHint();
		if (elementTypeHint == null) {
			return null; // our hint is visual id and must be specified
		}
		if (semanticHint != null && !semanticHint.equals(elementTypeHint)) {
			return null; // if semantic hint is specified it should be the same as in element type
		}
		int visualID = FritzingVisualIDRegistry.getVisualID(elementTypeHint);
		EObject domainElement = getSemanticElement(semanticAdapter);
		if (domainElement != null
				&& visualID != FritzingVisualIDRegistry
						.getLinkWithClassVisualID(domainElement)) {
			return null; // visual id for link EClass should match visual id from element type
		}
		return getEdgeViewClass(visualID);
	}

	/**
	 * @generated
	 */
	protected Class getEdgeViewClass(int visualID) {
		switch (visualID) {
		case WireEditPart.VISUAL_ID:
			return WireViewFactory.class;
		case TrackEditPart.VISUAL_ID:
			return TrackViewFactory.class;
		}
		return null;
	}

	/**
	 * @generated
	 */
	private IElementType getSemanticElementType(IAdaptable semanticAdapter) {
		if (semanticAdapter == null) {
			return null;
		}
		return (IElementType) semanticAdapter.getAdapter(IElementType.class);
	}
}

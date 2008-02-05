/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.pcb.providers;

import org.eclipse.core.runtime.IAdaptable;
import org.eclipse.emf.ecore.EAttribute;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.gmf.runtime.common.core.service.AbstractProvider;
import org.eclipse.gmf.runtime.common.core.service.IOperation;
import org.eclipse.gmf.runtime.common.ui.services.parser.GetParserOperation;
import org.eclipse.gmf.runtime.common.ui.services.parser.IParser;
import org.eclipse.gmf.runtime.common.ui.services.parser.IParserProvider;
import org.eclipse.gmf.runtime.emf.type.core.IElementType;
import org.eclipse.gmf.runtime.emf.ui.services.parser.ParserHintAdapter;
import org.eclipse.gmf.runtime.notation.View;
import org.fritzing.fritzing.FritzingPackage;
import org.fritzing.pcb.edit.parts.GenericPartNameEditPart;
import org.fritzing.pcb.edit.parts.LEDNameEditPart;
import org.fritzing.pcb.edit.parts.ResistorNameEditPart;
import org.fritzing.pcb.edit.parts.TerminalName2EditPart;
import org.fritzing.pcb.edit.parts.TerminalNameEditPart;
import org.fritzing.pcb.edit.parts.WireNameEditPart;
import org.fritzing.pcb.parsers.MessageFormatParser;
import org.fritzing.pcb.part.FritzingVisualIDRegistry;

/**
 * @generated
 */
public class FritzingParserProvider extends AbstractProvider implements
		IParserProvider {

	/**
	 * @generated
	 */
	private IParser lEDName_5002Parser;

	/**
	 * @generated
	 */
	private IParser getLEDName_5002Parser() {
		if (lEDName_5002Parser == null) {
			lEDName_5002Parser = createLEDName_5002Parser();
		}
		return lEDName_5002Parser;
	}

	/**
	 * @generated
	 */
	protected IParser createLEDName_5002Parser() {
		EAttribute[] features = new EAttribute[] { FritzingPackage.eINSTANCE
				.getIElement_Name(), };
		MessageFormatParser parser = new MessageFormatParser(features);
		return parser;
	}

	/**
	 * @generated
	 */
	private IParser resistorName_5003Parser;

	/**
	 * @generated
	 */
	private IParser getResistorName_5003Parser() {
		if (resistorName_5003Parser == null) {
			resistorName_5003Parser = createResistorName_5003Parser();
		}
		return resistorName_5003Parser;
	}

	/**
	 * @generated
	 */
	protected IParser createResistorName_5003Parser() {
		EAttribute[] features = new EAttribute[] { FritzingPackage.eINSTANCE
				.getIElement_Name(), };
		MessageFormatParser parser = new MessageFormatParser(features);
		return parser;
	}

	/**
	 * @generated
	 */
	private IParser terminalName_5004Parser;

	/**
	 * @generated
	 */
	private IParser getTerminalName_5004Parser() {
		if (terminalName_5004Parser == null) {
			terminalName_5004Parser = createTerminalName_5004Parser();
		}
		return terminalName_5004Parser;
	}

	/**
	 * @generated
	 */
	protected IParser createTerminalName_5004Parser() {
		EAttribute[] features = new EAttribute[] { FritzingPackage.eINSTANCE
				.getIElement_Name(), };
		MessageFormatParser parser = new MessageFormatParser(features);
		return parser;
	}

	/**
	 * @generated
	 */
	private IParser genericPartName_5005Parser;

	/**
	 * @generated
	 */
	private IParser getGenericPartName_5005Parser() {
		if (genericPartName_5005Parser == null) {
			genericPartName_5005Parser = createGenericPartName_5005Parser();
		}
		return genericPartName_5005Parser;
	}

	/**
	 * @generated
	 */
	protected IParser createGenericPartName_5005Parser() {
		EAttribute[] features = new EAttribute[] { FritzingPackage.eINSTANCE
				.getIElement_Name(), };
		MessageFormatParser parser = new MessageFormatParser(features);
		return parser;
	}

	/**
	 * @generated
	 */
	private IParser terminalName_5001Parser;

	/**
	 * @generated
	 */
	private IParser getTerminalName_5001Parser() {
		if (terminalName_5001Parser == null) {
			terminalName_5001Parser = createTerminalName_5001Parser();
		}
		return terminalName_5001Parser;
	}

	/**
	 * @generated
	 */
	protected IParser createTerminalName_5001Parser() {
		EAttribute[] features = new EAttribute[] { FritzingPackage.eINSTANCE
				.getIElement_Name(), };
		MessageFormatParser parser = new MessageFormatParser(features);
		return parser;
	}

	/**
	 * @generated
	 */
	private IParser wireName_6001Parser;

	/**
	 * @generated
	 */
	private IParser getWireName_6001Parser() {
		if (wireName_6001Parser == null) {
			wireName_6001Parser = createWireName_6001Parser();
		}
		return wireName_6001Parser;
	}

	/**
	 * @generated
	 */
	protected IParser createWireName_6001Parser() {
		EAttribute[] features = new EAttribute[] { FritzingPackage.eINSTANCE
				.getIElement_Name(), };
		MessageFormatParser parser = new MessageFormatParser(features);
		return parser;
	}

	/**
	 * @generated
	 */
	protected IParser getParser(int visualID) {
		switch (visualID) {
		case LEDNameEditPart.VISUAL_ID:
			return getLEDName_5002Parser();
		case ResistorNameEditPart.VISUAL_ID:
			return getResistorName_5003Parser();
		case TerminalNameEditPart.VISUAL_ID:
			return getTerminalName_5004Parser();
		case GenericPartNameEditPart.VISUAL_ID:
			return getGenericPartName_5005Parser();
		case TerminalName2EditPart.VISUAL_ID:
			return getTerminalName_5001Parser();
		case WireNameEditPart.VISUAL_ID:
			return getWireName_6001Parser();
		}
		return null;
	}

	/**
	 * @generated
	 */
	public IParser getParser(IAdaptable hint) {
		String vid = (String) hint.getAdapter(String.class);
		if (vid != null) {
			return getParser(FritzingVisualIDRegistry.getVisualID(vid));
		}
		View view = (View) hint.getAdapter(View.class);
		if (view != null) {
			return getParser(FritzingVisualIDRegistry.getVisualID(view));
		}
		return null;
	}

	/**
	 * @generated
	 */
	public boolean provides(IOperation operation) {
		if (operation instanceof GetParserOperation) {
			IAdaptable hint = ((GetParserOperation) operation).getHint();
			if (FritzingElementTypes.getElement(hint) == null) {
				return false;
			}
			return getParser(hint) != null;
		}
		return false;
	}

	/**
	 * @generated
	 */
	public static class HintAdapter extends ParserHintAdapter {

		/**
		 * @generated
		 */
		private final IElementType elementType;

		/**
		 * @generated
		 */
		public HintAdapter(IElementType type, EObject object, String parserHint) {
			super(object, parserHint);
			assert type != null;
			elementType = type;
		}

		/**
		 * @generated
		 */
		public Object getAdapter(Class adapter) {
			if (IElementType.class.equals(adapter)) {
				return elementType;
			}
			return super.getAdapter(adapter);
		}
	}

}

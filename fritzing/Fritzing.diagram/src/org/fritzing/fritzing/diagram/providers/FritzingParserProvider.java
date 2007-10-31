/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.providers;

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
import org.fritzing.fritzing.diagram.edit.parts.ArduinoIdEditPart;
import org.fritzing.fritzing.diagram.edit.parts.BreadboardIdEditPart;
import org.fritzing.fritzing.diagram.edit.parts.ButtonIdEditPart;
import org.fritzing.fritzing.diagram.edit.parts.LEDIdEditPart;
import org.fritzing.fritzing.diagram.edit.parts.ModuleIdEditPart;
import org.fritzing.fritzing.diagram.edit.parts.ResistorIdEditPart;
import org.fritzing.fritzing.diagram.edit.parts.TerminalName2EditPart;
import org.fritzing.fritzing.diagram.edit.parts.TerminalNameEditPart;
import org.fritzing.fritzing.diagram.edit.parts.WireIdEditPart;
import org.fritzing.fritzing.diagram.parsers.MessageFormatParser;
import org.fritzing.fritzing.diagram.part.FritzingVisualIDRegistry;

/**
 * @generated
 */
public class FritzingParserProvider extends AbstractProvider implements
		IParserProvider {

	/**
	 * @generated
	 */
	private IParser moduleId_5002Parser;

	/**
	 * @generated
	 */
	private IParser getModuleId_5002Parser() {
		if (moduleId_5002Parser == null) {
			moduleId_5002Parser = createModuleId_5002Parser();
		}
		return moduleId_5002Parser;
	}

	/**
	 * @generated
	 */
	protected IParser createModuleId_5002Parser() {
		EAttribute[] features = new EAttribute[] { FritzingPackage.eINSTANCE
				.getElement_Id(), };
		MessageFormatParser parser = new MessageFormatParser(features);
		return parser;
	}

	/**
	 * @generated
	 */
	private IParser arduinoId_5003Parser;

	/**
	 * @generated
	 */
	private IParser getArduinoId_5003Parser() {
		if (arduinoId_5003Parser == null) {
			arduinoId_5003Parser = createArduinoId_5003Parser();
		}
		return arduinoId_5003Parser;
	}

	/**
	 * @generated
	 */
	protected IParser createArduinoId_5003Parser() {
		EAttribute[] features = new EAttribute[] { FritzingPackage.eINSTANCE
				.getElement_Id(), };
		MessageFormatParser parser = new MessageFormatParser(features);
		return parser;
	}

	/**
	 * @generated
	 */
	private IParser breadboardId_5004Parser;

	/**
	 * @generated
	 */
	private IParser getBreadboardId_5004Parser() {
		if (breadboardId_5004Parser == null) {
			breadboardId_5004Parser = createBreadboardId_5004Parser();
		}
		return breadboardId_5004Parser;
	}

	/**
	 * @generated
	 */
	protected IParser createBreadboardId_5004Parser() {
		EAttribute[] features = new EAttribute[] { FritzingPackage.eINSTANCE
				.getElement_Id(), };
		MessageFormatParser parser = new MessageFormatParser(features);
		return parser;
	}

	/**
	 * @generated
	 */
	private IParser lEDId_5005Parser;

	/**
	 * @generated
	 */
	private IParser getLEDId_5005Parser() {
		if (lEDId_5005Parser == null) {
			lEDId_5005Parser = createLEDId_5005Parser();
		}
		return lEDId_5005Parser;
	}

	/**
	 * @generated
	 */
	protected IParser createLEDId_5005Parser() {
		EAttribute[] features = new EAttribute[] { FritzingPackage.eINSTANCE
				.getElement_Id(), };
		MessageFormatParser parser = new MessageFormatParser(features);
		return parser;
	}

	/**
	 * @generated
	 */
	private IParser resistorId_5006Parser;

	/**
	 * @generated
	 */
	private IParser getResistorId_5006Parser() {
		if (resistorId_5006Parser == null) {
			resistorId_5006Parser = createResistorId_5006Parser();
		}
		return resistorId_5006Parser;
	}

	/**
	 * @generated
	 */
	protected IParser createResistorId_5006Parser() {
		EAttribute[] features = new EAttribute[] { FritzingPackage.eINSTANCE
				.getElement_Id(), };
		MessageFormatParser parser = new MessageFormatParser(features);
		return parser;
	}

	/**
	 * @generated
	 */
	private IParser buttonId_5007Parser;

	/**
	 * @generated
	 */
	private IParser getButtonId_5007Parser() {
		if (buttonId_5007Parser == null) {
			buttonId_5007Parser = createButtonId_5007Parser();
		}
		return buttonId_5007Parser;
	}

	/**
	 * @generated
	 */
	protected IParser createButtonId_5007Parser() {
		EAttribute[] features = new EAttribute[] { FritzingPackage.eINSTANCE
				.getElement_Id(), };
		MessageFormatParser parser = new MessageFormatParser(features);
		return parser;
	}

	/**
	 * @generated
	 */
	private IParser terminalName_5008Parser;

	/**
	 * @generated
	 */
	private IParser getTerminalName_5008Parser() {
		if (terminalName_5008Parser == null) {
			terminalName_5008Parser = createTerminalName_5008Parser();
		}
		return terminalName_5008Parser;
	}

	/**
	 * @generated
	 */
	protected IParser createTerminalName_5008Parser() {
		EAttribute[] features = new EAttribute[] { FritzingPackage.eINSTANCE
				.getTerminal_Name(), };
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
				.getTerminal_Name(), };
		MessageFormatParser parser = new MessageFormatParser(features);
		return parser;
	}

	/**
	 * @generated
	 */
	private IParser wireId_6001Parser;

	/**
	 * @generated
	 */
	private IParser getWireId_6001Parser() {
		if (wireId_6001Parser == null) {
			wireId_6001Parser = createWireId_6001Parser();
		}
		return wireId_6001Parser;
	}

	/**
	 * @generated
	 */
	protected IParser createWireId_6001Parser() {
		EAttribute[] features = new EAttribute[] { FritzingPackage.eINSTANCE
				.getElement_Id(), };
		MessageFormatParser parser = new MessageFormatParser(features);
		return parser;
	}

	/**
	 * @generated
	 */
	protected IParser getParser(int visualID) {
		switch (visualID) {
		case ModuleIdEditPart.VISUAL_ID:
			return getModuleId_5002Parser();
		case ArduinoIdEditPart.VISUAL_ID:
			return getArduinoId_5003Parser();
		case BreadboardIdEditPart.VISUAL_ID:
			return getBreadboardId_5004Parser();
		case LEDIdEditPart.VISUAL_ID:
			return getLEDId_5005Parser();
		case ResistorIdEditPart.VISUAL_ID:
			return getResistorId_5006Parser();
		case ButtonIdEditPart.VISUAL_ID:
			return getButtonId_5007Parser();
		case TerminalNameEditPart.VISUAL_ID:
			return getTerminalName_5008Parser();
		case TerminalName2EditPart.VISUAL_ID:
			return getTerminalName_5001Parser();
		case WireIdEditPart.VISUAL_ID:
			return getWireId_6001Parser();
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

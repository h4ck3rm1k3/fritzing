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
import org.fritzing.fritzing.diagram.edit.parts.ArduinoNameEditPart;
import org.fritzing.fritzing.diagram.edit.parts.ButtonNameEditPart;
import org.fritzing.fritzing.diagram.edit.parts.FsrSensorNameEditPart;
import org.fritzing.fritzing.diagram.edit.parts.GenericPartNameEditPart;
import org.fritzing.fritzing.diagram.edit.parts.LEDNameEditPart;
import org.fritzing.fritzing.diagram.edit.parts.LightSensorNameEditPart;
import org.fritzing.fritzing.diagram.edit.parts.PotentiometerNameEditPart;
import org.fritzing.fritzing.diagram.edit.parts.PowerTransistorNameEditPart;
import org.fritzing.fritzing.diagram.edit.parts.ResistorNameEditPart;
import org.fritzing.fritzing.diagram.edit.parts.TerminalName2EditPart;
import org.fritzing.fritzing.diagram.edit.parts.TerminalNameEditPart;
import org.fritzing.fritzing.diagram.edit.parts.TransistorNameEditPart;
import org.fritzing.fritzing.diagram.edit.parts.WireNameEditPart;
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
	private IParser arduinoName_5002Parser;

	/**
	 * @generated
	 */
	private IParser getArduinoName_5002Parser() {
		if (arduinoName_5002Parser == null) {
			arduinoName_5002Parser = createArduinoName_5002Parser();
		}
		return arduinoName_5002Parser;
	}

	/**
	 * @generated
	 */
	protected IParser createArduinoName_5002Parser() {
		EAttribute[] features = new EAttribute[] { FritzingPackage.eINSTANCE
				.getPart_Name(), };
		MessageFormatParser parser = new MessageFormatParser(features);
		return parser;
	}

	/**
	 * @generated
	 */
	private IParser lEDName_5003Parser;

	/**
	 * @generated
	 */
	private IParser getLEDName_5003Parser() {
		if (lEDName_5003Parser == null) {
			lEDName_5003Parser = createLEDName_5003Parser();
		}
		return lEDName_5003Parser;
	}

	/**
	 * @generated
	 */
	protected IParser createLEDName_5003Parser() {
		EAttribute[] features = new EAttribute[] { FritzingPackage.eINSTANCE
				.getPart_Name(), };
		MessageFormatParser parser = new MessageFormatParser(features);
		return parser;
	}

	/**
	 * @generated
	 */
	private IParser resistorName_5004Parser;

	/**
	 * @generated
	 */
	private IParser getResistorName_5004Parser() {
		if (resistorName_5004Parser == null) {
			resistorName_5004Parser = createResistorName_5004Parser();
		}
		return resistorName_5004Parser;
	}

	/**
	 * @generated
	 */
	protected IParser createResistorName_5004Parser() {
		EAttribute[] features = new EAttribute[] { FritzingPackage.eINSTANCE
				.getPart_Name(), };
		MessageFormatParser parser = new MessageFormatParser(features);
		return parser;
	}

	/**
	 * @generated
	 */
	private IParser buttonName_5005Parser;

	/**
	 * @generated
	 */
	private IParser getButtonName_5005Parser() {
		if (buttonName_5005Parser == null) {
			buttonName_5005Parser = createButtonName_5005Parser();
		}
		return buttonName_5005Parser;
	}

	/**
	 * @generated
	 */
	protected IParser createButtonName_5005Parser() {
		EAttribute[] features = new EAttribute[] { FritzingPackage.eINSTANCE
				.getPart_Name(), };
		MessageFormatParser parser = new MessageFormatParser(features);
		return parser;
	}

	/**
	 * @generated
	 */
	private IParser potentiometerName_5006Parser;

	/**
	 * @generated
	 */
	private IParser getPotentiometerName_5006Parser() {
		if (potentiometerName_5006Parser == null) {
			potentiometerName_5006Parser = createPotentiometerName_5006Parser();
		}
		return potentiometerName_5006Parser;
	}

	/**
	 * @generated
	 */
	protected IParser createPotentiometerName_5006Parser() {
		EAttribute[] features = new EAttribute[] { FritzingPackage.eINSTANCE
				.getPart_Name(), };
		MessageFormatParser parser = new MessageFormatParser(features);
		return parser;
	}

	/**
	 * @generated
	 */
	private IParser fsrSensorName_5007Parser;

	/**
	 * @generated
	 */
	private IParser getFsrSensorName_5007Parser() {
		if (fsrSensorName_5007Parser == null) {
			fsrSensorName_5007Parser = createFsrSensorName_5007Parser();
		}
		return fsrSensorName_5007Parser;
	}

	/**
	 * @generated
	 */
	protected IParser createFsrSensorName_5007Parser() {
		EAttribute[] features = new EAttribute[] { FritzingPackage.eINSTANCE
				.getPart_Name(), };
		MessageFormatParser parser = new MessageFormatParser(features);
		return parser;
	}

	/**
	 * @generated
	 */
	private IParser lightSensorName_5008Parser;

	/**
	 * @generated
	 */
	private IParser getLightSensorName_5008Parser() {
		if (lightSensorName_5008Parser == null) {
			lightSensorName_5008Parser = createLightSensorName_5008Parser();
		}
		return lightSensorName_5008Parser;
	}

	/**
	 * @generated
	 */
	protected IParser createLightSensorName_5008Parser() {
		EAttribute[] features = new EAttribute[] { FritzingPackage.eINSTANCE
				.getPart_Name(), };
		MessageFormatParser parser = new MessageFormatParser(features);
		return parser;
	}

	/**
	 * @generated
	 */
	private IParser terminalName_5009Parser;

	/**
	 * @generated
	 */
	private IParser getTerminalName_5009Parser() {
		if (terminalName_5009Parser == null) {
			terminalName_5009Parser = createTerminalName_5009Parser();
		}
		return terminalName_5009Parser;
	}

	/**
	 * @generated
	 */
	protected IParser createTerminalName_5009Parser() {
		EAttribute[] features = new EAttribute[] { FritzingPackage.eINSTANCE
				.getTerminal_Name(), };
		MessageFormatParser parser = new MessageFormatParser(features);
		return parser;
	}

	/**
	 * @generated
	 */
	private IParser transistorName_5010Parser;

	/**
	 * @generated
	 */
	private IParser getTransistorName_5010Parser() {
		if (transistorName_5010Parser == null) {
			transistorName_5010Parser = createTransistorName_5010Parser();
		}
		return transistorName_5010Parser;
	}

	/**
	 * @generated
	 */
	protected IParser createTransistorName_5010Parser() {
		EAttribute[] features = new EAttribute[] { FritzingPackage.eINSTANCE
				.getPart_Name(), };
		MessageFormatParser parser = new MessageFormatParser(features);
		return parser;
	}

	/**
	 * @generated
	 */
	private IParser powerTransistorName_5011Parser;

	/**
	 * @generated
	 */
	private IParser getPowerTransistorName_5011Parser() {
		if (powerTransistorName_5011Parser == null) {
			powerTransistorName_5011Parser = createPowerTransistorName_5011Parser();
		}
		return powerTransistorName_5011Parser;
	}

	/**
	 * @generated
	 */
	protected IParser createPowerTransistorName_5011Parser() {
		EAttribute[] features = new EAttribute[] { FritzingPackage.eINSTANCE
				.getPart_Name(), };
		MessageFormatParser parser = new MessageFormatParser(features);
		return parser;
	}

	/**
	 * @generated
	 */
	private IParser genericPartName_5012Parser;

	/**
	 * @generated
	 */
	private IParser getGenericPartName_5012Parser() {
		if (genericPartName_5012Parser == null) {
			genericPartName_5012Parser = createGenericPartName_5012Parser();
		}
		return genericPartName_5012Parser;
	}

	/**
	 * @generated
	 */
	protected IParser createGenericPartName_5012Parser() {
		EAttribute[] features = new EAttribute[] { FritzingPackage.eINSTANCE
				.getPart_Name(), };
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
				.getWire_Name(), };
		MessageFormatParser parser = new MessageFormatParser(features);
		return parser;
	}

	/**
	 * @generated
	 */
	protected IParser getParser(int visualID) {
		switch (visualID) {
		case ArduinoNameEditPart.VISUAL_ID:
			return getArduinoName_5002Parser();
		case LEDNameEditPart.VISUAL_ID:
			return getLEDName_5003Parser();
		case ResistorNameEditPart.VISUAL_ID:
			return getResistorName_5004Parser();
		case ButtonNameEditPart.VISUAL_ID:
			return getButtonName_5005Parser();
		case PotentiometerNameEditPart.VISUAL_ID:
			return getPotentiometerName_5006Parser();
		case FsrSensorNameEditPart.VISUAL_ID:
			return getFsrSensorName_5007Parser();
		case LightSensorNameEditPart.VISUAL_ID:
			return getLightSensorName_5008Parser();
		case TerminalNameEditPart.VISUAL_ID:
			return getTerminalName_5009Parser();
		case TransistorNameEditPart.VISUAL_ID:
			return getTransistorName_5010Parser();
		case PowerTransistorNameEditPart.VISUAL_ID:
			return getPowerTransistorName_5011Parser();
		case GenericPartNameEditPart.VISUAL_ID:
			return getGenericPartName_5012Parser();
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

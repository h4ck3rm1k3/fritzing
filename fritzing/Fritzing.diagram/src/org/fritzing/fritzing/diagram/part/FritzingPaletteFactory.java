/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.part;

import java.util.ArrayList;
import java.util.List;

import org.eclipse.gef.Tool;
import org.eclipse.gef.palette.PaletteContainer;
import org.eclipse.gef.palette.PaletteDrawer;
import org.eclipse.gef.palette.PaletteGroup;
import org.eclipse.gef.palette.PaletteRoot;
import org.eclipse.gef.palette.ToolEntry;
import org.eclipse.gmf.runtime.diagram.ui.tools.UnspecifiedTypeConnectionTool;
import org.eclipse.gmf.runtime.diagram.ui.tools.UnspecifiedTypeCreationTool;
import org.fritzing.fritzing.diagram.providers.FritzingElementTypes;

/**
 * @generated
 */
public class FritzingPaletteFactory {

	/**
	 * @generated
	 */
	public void fillPalette(PaletteRoot paletteRoot) {
		paletteRoot.add(createConnect1Group());
		paletteRoot.add(createParts2Group());
	}

	/**
	 * Creates "Connect" palette tool group
	 * @generated
	 */
	private PaletteContainer createConnect1Group() {
		PaletteGroup paletteContainer = new PaletteGroup(
				Messages.Connect1Group_title);
		paletteContainer.add(createWire1CreationTool());
		return paletteContainer;
	}

	/**
	 * Creates "Parts" palette tool group
	 * @generated
	 */
	private PaletteContainer createParts2Group() {
		PaletteDrawer paletteContainer = new PaletteDrawer(
				Messages.Parts2Group_title);
		paletteContainer.add(createArduino1CreationTool());
		paletteContainer.add(createResistor2CreationTool());
		paletteContainer.add(createLED3CreationTool());
		paletteContainer.add(createButton4CreationTool());
		return paletteContainer;
	}

	/**
	 * @generated
	 */
	private ToolEntry createWire1CreationTool() {
		List/*<IElementType>*/types = new ArrayList/*<IElementType>*/(1);
		types.add(FritzingElementTypes.Wire_4001);
		LinkToolEntry entry = new LinkToolEntry(
				Messages.Wire1CreationTool_title,
				Messages.Wire1CreationTool_desc, types);
		entry.setSmallIcon(FritzingElementTypes
				.getImageDescriptor(FritzingElementTypes.Wire_4001));
		entry.setLargeIcon(entry.getSmallIcon());
		return entry;
	}

	/**
	 * @generated
	 */
	private ToolEntry createArduino1CreationTool() {
		List/*<IElementType>*/types = new ArrayList/*<IElementType>*/(1);
		types.add(FritzingElementTypes.Arduino_2002);
		NodeToolEntry entry = new NodeToolEntry(
				Messages.Arduino1CreationTool_title,
				Messages.Arduino1CreationTool_desc, types);
		entry.setSmallIcon(FritzingElementTypes
				.getImageDescriptor(FritzingElementTypes.Arduino_2002));
		entry.setLargeIcon(entry.getSmallIcon());
		return entry;
	}

	/**
	 * @generated
	 */
	private ToolEntry createResistor2CreationTool() {
		List/*<IElementType>*/types = new ArrayList/*<IElementType>*/(1);
		types.add(FritzingElementTypes.Resistor_2005);
		NodeToolEntry entry = new NodeToolEntry(
				Messages.Resistor2CreationTool_title,
				Messages.Resistor2CreationTool_desc, types);
		entry.setSmallIcon(FritzingElementTypes
				.getImageDescriptor(FritzingElementTypes.Resistor_2005));
		entry.setLargeIcon(entry.getSmallIcon());
		return entry;
	}

	/**
	 * @generated
	 */
	private ToolEntry createLED3CreationTool() {
		List/*<IElementType>*/types = new ArrayList/*<IElementType>*/(1);
		types.add(FritzingElementTypes.LED_2004);
		NodeToolEntry entry = new NodeToolEntry(
				Messages.LED3CreationTool_title,
				Messages.LED3CreationTool_desc, types);
		entry.setSmallIcon(FritzingElementTypes
				.getImageDescriptor(FritzingElementTypes.LED_2004));
		entry.setLargeIcon(entry.getSmallIcon());
		return entry;
	}

	/**
	 * @generated
	 */
	private ToolEntry createButton4CreationTool() {
		List/*<IElementType>*/types = new ArrayList/*<IElementType>*/(1);
		types.add(FritzingElementTypes.Button_2006);
		NodeToolEntry entry = new NodeToolEntry(
				Messages.Button4CreationTool_title,
				Messages.Button4CreationTool_desc, types);
		entry.setSmallIcon(FritzingElementTypes
				.getImageDescriptor(FritzingElementTypes.Button_2006));
		entry.setLargeIcon(entry.getSmallIcon());
		return entry;
	}

	/**
	 * @generated
	 */
	private static class NodeToolEntry extends ToolEntry {

		/**
		 * @generated
		 */
		private final List elementTypes;

		/**
		 * @generated
		 */
		private NodeToolEntry(String title, String description,
				List elementTypes) {
			super(title, description, null, null);
			this.elementTypes = elementTypes;
		}

		/**
		 * @generated
		 */
		public Tool createTool() {
			Tool tool = new UnspecifiedTypeCreationTool(elementTypes);
			tool.setProperties(getToolProperties());
			return tool;
		}
	}

	/**
	 * @generated
	 */
	private static class LinkToolEntry extends ToolEntry {

		/**
		 * @generated
		 */
		private final List relationshipTypes;

		/**
		 * @generated
		 */
		private LinkToolEntry(String title, String description,
				List relationshipTypes) {
			super(title, description, null, null);
			this.relationshipTypes = relationshipTypes;
		}

		/**
		 * @generated
		 */
		public Tool createTool() {
			Tool tool = new UnspecifiedTypeConnectionTool(relationshipTypes);
			tool.setProperties(getToolProperties());
			return tool;
		}
	}
}

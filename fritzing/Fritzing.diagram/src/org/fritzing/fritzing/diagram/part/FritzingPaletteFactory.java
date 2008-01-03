/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.part;

import java.io.File;
import java.net.URL;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.List;

import org.eclipse.emf.ecore.ENamedElement;
import org.eclipse.gef.Request;
import org.eclipse.gef.Tool;
import org.eclipse.gef.palette.PaletteContainer;
import org.eclipse.gef.palette.PaletteDrawer;
import org.eclipse.gef.palette.PaletteGroup;
import org.eclipse.gef.palette.PaletteRoot;
import org.eclipse.gef.palette.PanningSelectionToolEntry;
import org.eclipse.gef.palette.ToolEntry;
import org.eclipse.gef.requests.CreateRequest;
import org.eclipse.gmf.runtime.diagram.core.preferences.PreferencesHint;
import org.eclipse.gmf.runtime.diagram.ui.requests.CreateUnspecifiedTypeRequest;
import org.eclipse.gmf.runtime.diagram.ui.services.palette.PaletteService;
import org.eclipse.gmf.runtime.diagram.ui.tools.UnspecifiedTypeConnectionTool;
import org.eclipse.gmf.runtime.diagram.ui.tools.UnspecifiedTypeCreationTool;
import org.eclipse.gmf.runtime.emf.type.core.IElementType;
import org.eclipse.jface.resource.ImageDescriptor;
import org.fritzing.fritzing.diagram.edit.PartLoader;
import org.fritzing.fritzing.diagram.edit.PartLoaderRegistry;
import org.fritzing.fritzing.diagram.providers.FritzingElementTypes;

/**
 * @generated
 */
public class FritzingPaletteFactory {
	
	
	/**
	 * @generated NOT
	 * 
	 * differentiates between predefined class xml and generic xml
	 */
	static final Hashtable<String, IElementType> coreMap = new Hashtable<String, IElementType>();	
	
	/**
	 * @generated NOT
	 */
	public void fillPalette(PaletteRoot paletteRoot) {
		coreMap.put("arduino", FritzingElementTypes.Arduino_2001);
		coreMap.put("button", FritzingElementTypes.Button_2004);
		coreMap.put("fsr", FritzingElementTypes.FsrSensor_2006);
		coreMap.put("led", FritzingElementTypes.LED_2002);
		coreMap.put("lightsensor", FritzingElementTypes.LightSensor_2007);
		coreMap.put("potentiometer", FritzingElementTypes.Potentiometer_2005);
		coreMap.put("powertransistor", FritzingElementTypes.PowerTransistor_2010);
		coreMap.put("resistor", FritzingElementTypes.Resistor_2003);
		coreMap.put("transistor", FritzingElementTypes.Transistor_2009);
		for (Enumeration<String> keys = coreMap.keys(); keys.hasMoreElements(); ) {
			String folder = keys.nextElement();
			PartLoader partLoader = PartLoaderRegistry.getInstance().get("libraries/core/" + folder + "/partdescription.xml");
			if (partLoader == null) {
				// alert user;
				continue;
			}
			
			IElementType type = coreMap.get(folder);
					
			String key = FritzingElementTypes.getImageRegistryKey(FritzingElementTypes.getElement(type));
			if (key == null) {
				// alert user
				continue;
			}
				
			
			// set up the ImageDescriptor for the icon to point to the library
			// instead of internal icons
			ImageDescriptor id = ImageDescriptor.createFromFile(null, 
					FritzingDiagramEditorUtil.getFritzingLocation() + 
						"libraries/core/" + 
						folder + "/" + 
						partLoader.getIconFilename());
			FritzingElementTypes.getImageRegistry().put(key, id);
			
			
			// set up the ImageDescriptor for the icon to point to the library
			// instead of internal icons
			id = ImageDescriptor.createFromFile(null, 
					FritzingDiagramEditorUtil.getFritzingLocation() + 
						"libraries/core/" + 
						folder + "/" + 
						partLoader.getLargeIconFilename());
			FritzingElementTypes.getImageRegistry().put(key + "Large", id);

		}
			
		customiseStandardGroup(paletteRoot);
		paletteRoot.add(createConnect1Group());
		PaletteContainer pc = createParts2Group();		
		addGenerics(pc);
		paletteRoot.add(pc);
	}
	
	
	/**
	 * @generated NOT
	 */
	protected void addGenerics(PaletteContainer pc) {
		File file = new File(FritzingDiagramEditorUtil.getFritzingLocation() + "libraries/core/");
		File[] files = file.listFiles();
		for (int i = 0; i < files.length; i++) {			
			String filename = files[i].getName();
			// now go on to deal with generics
			boolean gotOne = false;
			
			for (Enumeration<String> keys = coreMap.keys(); keys.hasMoreElements(); ) {
				if (filename.equalsIgnoreCase(keys.nextElement())) {
					gotOne = true;
					break;
				}
			}

			if (gotOne) continue;

			PartLoader partLoader = PartLoaderRegistry.getInstance().get("libraries/core/" + filename + "/partdescription.xml");
			
			List<IElementType>types = new ArrayList<IElementType>(1);
			types.add(FritzingElementTypes.GenericPart_2011);
			NodeToolEntry entry = new GenericNodeToolEntry(
					partLoader.getSpecies(),
					partLoader.getDescription(), 
					types, partLoader);
			
			ImageDescriptor id = ImageDescriptor.createFromFile(null, 
					FritzingDiagramEditorUtil.getFritzingLocation() + 
						"libraries/core/" + 
						filename + "/" + 
						partLoader.getIconFilename());
			entry.setSmallIcon(id);
						
			id = ImageDescriptor.createFromFile(null, 
					FritzingDiagramEditorUtil.getFritzingLocation() + 
						"libraries/core/" + 
						filename + "/" + 
						partLoader.getLargeIconFilename());
			
			entry.setLargeIcon(id);
			pc.add(entry);
			
		}
		
	}

	/**
	 * @generated NOT
	 */
	public void customiseStandardGroup(PaletteRoot paletteRoot) {
		PaletteContainer standard = (PaletteContainer) paletteRoot
				.getChildren().get(0);
		ToolEntry standardSelectionTool = (ToolEntry) standard.getChildren()
				.get(1);
		standard.remove(standardSelectionTool);
		ToolEntry selectionPanTool = createPanningSelectionTool();
		standard.add(1, selectionPanTool);
		paletteRoot.setDefaultEntry(selectionPanTool);
	}

	/**
	 * @generated NOT
	 */
	private ToolEntry createPanningSelectionTool() {
		PanningSelectionToolEntry entry = new PanningSelectionToolEntry();
		return entry;
	}
	
	protected ImageDescriptor getLargeImageDescriptor(IElementType type) {
		return FritzingElementTypes.getImageRegistry().getDescriptor(
				FritzingElementTypes.getImageRegistryKey(FritzingElementTypes.getElement(type)) + "Large");
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
		paletteContainer.add(createPotentiometer5CreationTool());
		paletteContainer.add(createFSRSensor6CreationTool());
		paletteContainer.add(createLightSensor7CreationTool());
		paletteContainer.add(createTransistor8CreationTool());
		paletteContainer.add(createPowerTransistor9CreationTool());
		
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
	 * @generated NOT
	 */
	private ToolEntry createArduino1CreationTool() {
		List/*<IElementType>*/types = new ArrayList/*<IElementType>*/(1);
		types.add(FritzingElementTypes.Arduino_2001);
		NodeToolEntry entry = new NodeToolEntry(
				Messages.Arduino1CreationTool_title,
				Messages.Arduino1CreationTool_desc, types);
		entry.setSmallIcon(FritzingElementTypes
				.getImageDescriptor(FritzingElementTypes.Arduino_2001));
		entry.setLargeIcon(getLargeImageDescriptor(FritzingElementTypes.Arduino_2001));
		return entry;
	}

	/**
	 * @generated NOT
	 */
	private ToolEntry createResistor2CreationTool() {
		List/*<IElementType>*/types = new ArrayList/*<IElementType>*/(1);
		types.add(FritzingElementTypes.Resistor_2003);
		NodeToolEntry entry = new NodeToolEntry(
				Messages.Resistor2CreationTool_title,
				Messages.Resistor2CreationTool_desc, types);
		entry.setSmallIcon(FritzingElementTypes
				.getImageDescriptor(FritzingElementTypes.Resistor_2003));
		entry.setLargeIcon(getLargeImageDescriptor(FritzingElementTypes.Resistor_2003));
		return entry;
	}

	/** 
	 * @generated NOT
	 */
	private ToolEntry createLED3CreationTool() {
		List/*<IElementType>*/types = new ArrayList/*<IElementType>*/(1);
		types.add(FritzingElementTypes.LED_2002);
		NodeToolEntry entry = new NodeToolEntry(
				Messages.LED3CreationTool_title,
				Messages.LED3CreationTool_desc, types);
		entry.setSmallIcon(FritzingElementTypes
				.getImageDescriptor(FritzingElementTypes.LED_2002));
		entry.setLargeIcon(getLargeImageDescriptor(FritzingElementTypes.LED_2002));
		return entry;
	}

	/**
	 * @generated NOT
	 */
	private ToolEntry createButton4CreationTool() {
		List/*<IElementType>*/types = new ArrayList/*<IElementType>*/(1);
		types.add(FritzingElementTypes.Button_2004);
		NodeToolEntry entry = new NodeToolEntry(
				Messages.Button4CreationTool_title,
				Messages.Button4CreationTool_desc, types);
		entry.setSmallIcon(FritzingElementTypes
				.getImageDescriptor(FritzingElementTypes.Button_2004));
		entry.setLargeIcon(getLargeImageDescriptor(FritzingElementTypes.Button_2004));
		return entry;
	}

	/**
	 * @generated NOT
	 */
	private ToolEntry createPotentiometer5CreationTool() {
		List/*<IElementType>*/types = new ArrayList/*<IElementType>*/(1);
		types.add(FritzingElementTypes.Potentiometer_2005);
		NodeToolEntry entry = new NodeToolEntry(
				Messages.Potentiometer5CreationTool_title,
				Messages.Potentiometer5CreationTool_desc, types);
		entry.setSmallIcon(FritzingElementTypes
				.getImageDescriptor(FritzingElementTypes.Potentiometer_2005));
		entry.setLargeIcon(getLargeImageDescriptor(FritzingElementTypes.Potentiometer_2005));
		return entry;
	}

	/**
	 * @generated NOT
	 */
	private ToolEntry createFSRSensor6CreationTool() {
		List/*<IElementType>*/types = new ArrayList/*<IElementType>*/(1);
		types.add(FritzingElementTypes.FsrSensor_2006);
		NodeToolEntry entry = new NodeToolEntry(
				Messages.FSRSensor6CreationTool_title,
				Messages.FSRSensor6CreationTool_desc, types);
		entry.setSmallIcon(FritzingElementTypes
				.getImageDescriptor(FritzingElementTypes.FsrSensor_2006));
		entry.setLargeIcon(getLargeImageDescriptor(FritzingElementTypes.FsrSensor_2006));
		return entry;
	}

	/**
	 * @generated NOT
	 */
	private ToolEntry createLightSensor7CreationTool() {
		List/*<IElementType>*/types = new ArrayList/*<IElementType>*/(1);
		types.add(FritzingElementTypes.LightSensor_2007);
		NodeToolEntry entry = new NodeToolEntry(
				Messages.LightSensor7CreationTool_title,
				Messages.LightSensor7CreationTool_desc, types);
		entry.setSmallIcon(FritzingElementTypes
				.getImageDescriptor(FritzingElementTypes.LightSensor_2007));
		entry.setLargeIcon(getLargeImageDescriptor(FritzingElementTypes.LightSensor_2007));
		return entry;
	}

	/**
	 * @generated NOT
	 */
	private ToolEntry createTransistor8CreationTool() {
		List/*<IElementType>*/types = new ArrayList/*<IElementType>*/(1);
		types.add(FritzingElementTypes.Transistor_2009);
		NodeToolEntry entry = new NodeToolEntry(
				Messages.Transistor8CreationTool_title,
				Messages.Transistor8CreationTool_desc, types);
		entry.setSmallIcon(FritzingElementTypes
				.getImageDescriptor(FritzingElementTypes.Transistor_2009));
		entry.setLargeIcon(getLargeImageDescriptor(FritzingElementTypes.Transistor_2009));
		return entry;
	}

	/**
	 * @generated NOT
	 */
	private ToolEntry createPowerTransistor9CreationTool() {
		List/*<IElementType>*/types = new ArrayList/*<IElementType>*/(1);
		types.add(FritzingElementTypes.PowerTransistor_2010);
		NodeToolEntry entry = new NodeToolEntry(
				Messages.PowerTransistor9CreationTool_title,
				Messages.PowerTransistor9CreationTool_desc, types);
		entry.setSmallIcon(FritzingElementTypes
				.getImageDescriptor(FritzingElementTypes.PowerTransistor_2010));
		entry.setLargeIcon(getLargeImageDescriptor(FritzingElementTypes.PowerTransistor_2010));
		return entry;
	}

	
	
	/**
	 * @generated NOT
	 */
	private static class GenericNodeToolEntry extends NodeToolEntry {
		/**
		 * @generated NOT
		 */
		private final PartLoader partLoader;

		/**
		 * @generated NOT
		 */
		private GenericNodeToolEntry(String title, String description,
				List elementTypes, PartLoader partLoader) {
			super(title, description, elementTypes);
			
			// eventually passes the partloader along to GenericPartCreateCommand
			this.partLoader = partLoader;
		}
		
		/**
		 * @generated NOT
		 */
		public Tool createTool() {
			// eventually passes the partloader along to GenericPartCreateCommand
			Tool tool = new GenericUnspecifiedTypeCreationTool(elementTypes, partLoader);
			tool.setProperties(getToolProperties());
			return tool;
		}
	}
	

	/**
	 * @generated NOT
	 */
	private static class GenericUnspecifiedTypeCreationTool extends UnspecifiedTypeCreationTool {
		/**
		 * keep a copy for later.
		 */
		private List et;

		/**
		 * @generated NOT
		 */
		private final PartLoader partLoader;

		/**
		 * @generated NOT
		 */
		public GenericUnspecifiedTypeCreationTool(List elementTypes, PartLoader partLoader) {
			super(elementTypes);
			// eventually passes the partloader along to GenericPartCreateCommand
			this.partLoader = partLoader;
			et = elementTypes;
		}
		
		protected Request createTargetRequest() {
			// eventually passes the partloader along to GenericPartCreateCommand
			return new GenericCreateUnspecifiedTypeRequest(et, getPreferencesHint(), partLoader);
		}
	}
	
	/**
	 * @generated NOT
	 */
	private static class GenericCreateUnspecifiedTypeRequest extends CreateUnspecifiedTypeRequest {
		/**
		 * @generated NOT
		 */
		private final PartLoader partLoader;
		/**
		 * @generated NOT
		 */
		public GenericCreateUnspecifiedTypeRequest(List elementTypes, PreferencesHint preferencesHint, PartLoader partLoader) {
			super(elementTypes, preferencesHint);
			// eventually passes the partloader along to GenericPartCreateCommand
			this.partLoader = partLoader;
		}
				
		/**
		 * @generated NOT
		 */
		public CreateRequest getRequestForType(IElementType creationHint) {
			CreateRequest cr = super.getRequestForType(creationHint);
			
			// eventually passes the partloader along to GenericPartCreateCommand
			cr.getExtendedData().put("partLoader", partLoader);
			return cr;
		}
	}	
		
	/**
	 * @generated
	 */
	private static class NodeToolEntry extends ToolEntry {

		/**
		 * @generated NOT
		 */
		protected final List elementTypes;

		/**
		 * @generated
		 */
		private NodeToolEntry(String title, String description,
				List elementTypes) {
			super(title, description, null, null);
			this.elementTypes = elementTypes;
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

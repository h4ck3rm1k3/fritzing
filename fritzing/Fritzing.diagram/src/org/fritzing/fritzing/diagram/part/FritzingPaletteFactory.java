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
	Hashtable<String, PaletteDrawer> drawerMap = new Hashtable<String, PaletteDrawer>();	
	
	/**
	 * @generated NOT
	 */
	public void fillPalette(PaletteRoot paletteRoot) {
		coreMap.put("led", FritzingElementTypes.LED_2001);
		coreMap.put("resistor", FritzingElementTypes.Resistor_2002);
		
		for (Enumeration<String> keys = coreMap.keys(); keys.hasMoreElements(); ) {
			String folder = keys.nextElement();
			PartLoader partLoader = PartLoaderRegistry.getInstance().get(
					"libraries" + File.separator + "core" + File.separator + folder + File.separator + "partdescription.xml");
			if (partLoader == null) {
				// TODO: alert user;
				System.out.println("Couldn't load part " + folder);
				continue;
			}				
		}
			
		customiseStandardGroup(paletteRoot);
		createParts2Group();		
		addGenerics("libraries");
		for (Enumeration<PaletteDrawer> e = drawerMap.elements(); e.hasMoreElements(); ) {
			paletteRoot.add(e.nextElement());
		}
	}
	
	protected PaletteDrawer getDrawer(String drawerName) {
		PaletteDrawer drawer = drawerMap.get(drawerName);
		if (drawer == null) {
			drawer = new PaletteDrawer(drawerName);
			drawerMap.put(drawerName, drawer);
		}
		return drawer;
	}
	
	
	/**
	 * @generated NOT
	 */
	protected void addGenerics(String folder) {
//		if (folder.equals("libraries" + File.separator + "core")) return;
		
		File file = new File(FritzingDiagramEditorUtil.getFritzingLocation() + folder);		
		if (!file.exists()) return;
		if (!file.isDirectory()) return;
		
		File xmlFile = new File(file.getAbsolutePath() + File.separator + "partdescription.xml");
		if (xmlFile.exists()) {
			addPart(folder, FritzingElementTypes.GenericPart_2004);	
		}
				
		File[] files = file.listFiles();
		for (int i = 0; i < files.length; i++) {			
			String filename = files[i].getName();
			addGenerics(folder + File.separator + filename);
		}	
	}
	
	protected void addPart(String prefix, IElementType type) {
		PartLoader partLoader = PartLoaderRegistry.getInstance().get(prefix + File.separator + "partdescription.xml");
		if (partLoader == null) return;
		
		if (partLoader.isGeneric()) {
			type = FritzingElementTypes.GenericPart_2004;
		}
		
		List<IElementType>types = new ArrayList<IElementType>(1);
		types.add(type);
		NodeToolEntry entry = new NodeToolEntry(
				partLoader.getTitle(),
				partLoader.getDescription(), 
				types, partLoader);
		
		ImageDescriptor id = ImageDescriptor.createFromFile(null, 
				FritzingDiagramEditorUtil.getFritzingLocation() + 
					prefix + File.separator + 
					partLoader.getIconFilename());
		entry.setSmallIcon(id);
					
		id = ImageDescriptor.createFromFile(null, 
				FritzingDiagramEditorUtil.getFritzingLocation() + 
					prefix + File.separator + 
					partLoader.getLargeIconFilename());
		
		entry.setLargeIcon(id);
		
		PaletteDrawer drawer = getDrawer(partLoader.getGenus());
		drawer.add(entry);

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
		
		standard.add(createWire1CreationTool());
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
	 * Creates "Parts" palette tool group
	 * @generated NOT
	 */
	private void createParts2Group() {
		for (Enumeration<String> keys = coreMap.keys(); keys.hasMoreElements(); ) {
			String key = keys.nextElement();
			IElementType type = coreMap.get(key);
			
			addPart("libraries/core/" + key, type);
			
			PartLoader partLoader = PartLoaderRegistry.getInstance().get("libraries" + File.separator + "core" + File.separator + key + File.separator + "partdescription.xml");
			
		}
		
		
	}

	/**
	 * @generated
	 */
	private ToolEntry createWire1CreationTool() {
		List/*<IElementType>*/types = new ArrayList/*<IElementType>*/(1);
		types.add(FritzingElementTypes.Wire_4001);
		LinkToolEntry entry = new LinkToolEntry(
				"Wire",
				"Connect parts with a wire", types);
		entry.setSmallIcon(FritzingElementTypes
				.getImageDescriptor(FritzingElementTypes.Wire_4001));
		entry.setLargeIcon(entry.getSmallIcon());
		return entry;
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
		private final PartLoader partLoader;

		/**
		 * @generated NOT
		 */
		protected final List elementTypes;

		/**
		 * @generated NOT
		 */
		private NodeToolEntry(String title, String description,
				List elementTypes, PartLoader partLoader) {
			super(partLoader.getTitle(), partLoader.getDescription(), null, null);
						
			// eventually passes the partloader along to PartCreateCommand
			this.partLoader = partLoader;
			this.elementTypes = elementTypes;
		}

		/**
		 * @generated NOT
		 */
		public Tool createTool() {
			// eventually passes the partloader along to PartCreateCommand
			Tool tool = new GenericUnspecifiedTypeCreationTool(elementTypes, partLoader);
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

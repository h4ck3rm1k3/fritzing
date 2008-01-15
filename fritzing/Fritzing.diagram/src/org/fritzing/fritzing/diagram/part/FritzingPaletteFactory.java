/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.part;

import java.io.File;
import java.net.URL;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;
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
import org.eclipse.gef.palette.PaletteSeparator;
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
	Hashtable<String, SortPaletteDrawer> drawerMap = new Hashtable<String, SortPaletteDrawer>();

	/**
	 * @generated NOT
	 */
	public void fillPalette(PaletteRoot paletteRoot) {
		coreMap.put("LED", FritzingElementTypes.LED_2001);
		coreMap.put("Resistor", FritzingElementTypes.Resistor_2002);

		customiseStandardGroup(paletteRoot);
		addParts("libraries");

		String[] keys = drawerMap.keySet().toArray(new String[1]);
		Arrays.sort(keys);

		for (int i = 0; i < keys.length; i++) {
			SortPaletteDrawer drawer = drawerMap.get(keys[i]);
			drawer.sortAdd();
			paletteRoot.add(drawer);
		}

	}

	/**
	 * Creates "Default" palette tool group
	 * @generated
	 */
	private PaletteContainer createDefault1Group() {
		PaletteGroup paletteContainer = new PaletteGroup(
				Messages.Default1Group_title);
		paletteContainer.setDescription(Messages.Default1Group_desc);
		paletteContainer.add(createWire1CreationTool());
		return paletteContainer;
	}

	protected SortPaletteDrawer getDrawer(String drawerName) {
		SortPaletteDrawer drawer = drawerMap.get(drawerName);
		if (drawer == null) {
			drawer = new SortPaletteDrawer(drawerName);
			drawerMap.put(drawerName, drawer);

			//			drawer.setSmallIcon(FritzingDiagramEditorPlugin
			//					.getBundledImageDescriptor("full/obj16/dot.gif") );
			//			drawer.setLargeIcon(FritzingDiagramEditorPlugin
			//					.getBundledImageDescriptor("full/obj16/dot.gif") );
		}
		return drawer;
	}

	/**
	 * @generated NOT
	 */
	protected void addParts(String folder) {

		File file = new File(FritzingDiagramEditorUtil.getFritzingLocation()
				+ folder);
		if (!file.exists())
			return;
		if (!file.isDirectory())
			return;

		File xmlFile = new File(file.getAbsolutePath() + File.separator
				+ "partdescription.xml");
		if (xmlFile.exists()) {
			addPart(folder);
		}

		File[] files = file.listFiles();
		for (int i = 0; i < files.length; i++) {
			String filename = files[i].getName();
			addParts(folder + File.separator + filename);
		}
	}

	protected void addPart(String prefix) {
		PartLoader partLoader = PartLoaderRegistry.getInstance().get(
				prefix + File.separator + "partdescription.xml");
		if (partLoader == null)
			return;

		IElementType type = FritzingElementTypes.GenericPart_2004;

		if (!partLoader.isGeneric()) {
			IElementType tempType = coreMap.get(partLoader.getSpecies());
			if (tempType != null) {
				type = tempType;
			} else {
				// alert the user
			}
		}

		List<IElementType> types = new ArrayList<IElementType>(1);
		types.add(type);
		NodeToolEntry entry = new NodeToolEntry(partLoader.getTitle(),
				partLoader.getDescription(), types, partLoader, prefix
						.startsWith("libraries" + File.separator + "core"));

		ImageDescriptor id = ImageDescriptor.createFromFile(null,
				FritzingDiagramEditorUtil.getFritzingLocation() + prefix
						+ File.separator + partLoader.getIconFilename());
		entry.setSmallIcon(id);

		id = ImageDescriptor.createFromFile(null, FritzingDiagramEditorUtil
				.getFritzingLocation()
				+ prefix + File.separator + partLoader.getLargeIconFilename());

		entry.setLargeIcon(id);

		SortPaletteDrawer drawer = getDrawer(partLoader.getGenus());
		drawer.preAdd(entry);

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
		//standard.add(createLegCreationTool());
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
				FritzingElementTypes.getImageRegistryKey(FritzingElementTypes
						.getElement(type))
						+ "Large");
	}

	/**
	 * @generated NOT
	 */
	private ToolEntry createWire1CreationTool() {
		List/*<IElementType>*/types = new ArrayList/*<IElementType>*/(1);
		types.add(FritzingElementTypes.Wire_4001);
		LinkToolEntry entry = new LinkToolEntry(
				Messages.Wire1CreationTool_title,
				Messages.Wire1CreationTool_desc, types);
		entry.setSmallIcon(FritzingDiagramEditorPlugin
				.getBundledImageDescriptor("icons/obj16/wireIconSmall.gif"));
		entry.setLargeIcon(FritzingDiagramEditorPlugin
				.getBundledImageDescriptor("icons/obj16/wireIconLarge.gif"));
		return entry;
	}

	/**
	 * @generated NOT
	 */
	private ToolEntry createLegCreationTool() {
		List/*<IElementType>*/types = new ArrayList/*<IElementType>*/(1);
		types.add(FritzingElementTypes.Leg_4003);
		LinkToolEntry entry = new LinkToolEntry("Create Leg",
				"Leg description", types);
		entry.setSmallIcon(FritzingDiagramEditorPlugin
				.getBundledImageDescriptor("icons/obj16/wireIconSmall.gif"));
		entry.setLargeIcon(FritzingDiagramEditorPlugin
				.getBundledImageDescriptor("icons/obj16/wireIconLarge.gif"));
		return entry;
	}

	/**
	 * @generated NOT
	 */
	private static class GenericUnspecifiedTypeCreationTool extends
			UnspecifiedTypeCreationTool {
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
		public GenericUnspecifiedTypeCreationTool(List elementTypes,
				PartLoader partLoader) {
			super(elementTypes);
			// eventually passes the partloader along to GenericPartCreateCommand
			this.partLoader = partLoader;
			et = elementTypes;
		}

		protected Request createTargetRequest() {
			// eventually passes the partloader along to GenericPartCreateCommand
			return new GenericCreateUnspecifiedTypeRequest(et,
					getPreferencesHint(), partLoader);
		}
	}

	/**
	 * @generated NOT
	 */
	private static class GenericCreateUnspecifiedTypeRequest extends
			CreateUnspecifiedTypeRequest {
		/**
		 * @generated NOT
		 */
		private final PartLoader partLoader;

		/**
		 * @generated NOT
		 */
		public GenericCreateUnspecifiedTypeRequest(List elementTypes,
				PreferencesHint preferencesHint, PartLoader partLoader) {
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
	 * @generated NOT
	 */
	private static class SortPaletteDrawer extends PaletteDrawer {
		protected ArrayList<NodeToolEntry> entries;

		/**
		 * @generated NOT
		 */
		public SortPaletteDrawer(String drawerName) {
			super(drawerName);
			entries = new ArrayList<NodeToolEntry>();
		}

		public void preAdd(NodeToolEntry entry) {
			entries.add(entry);
		}

		public void sortAdd() {
			NodeToolEntry[] tools = entries.toArray(new NodeToolEntry[1]);
			Arrays.sort(tools);
			boolean needSeparator = true;
			for (int i = 0; i < tools.length; i++) {
				NodeToolEntry tool = tools[i];
				if (!tool.core && needSeparator) {
					this.add(new PaletteSeparator());
					needSeparator = false;
				}
				this.add(tool);
			}
		}
	}

	/**
	 * @generated NOT
	 */
	private static class NodeToolEntry extends ToolEntry implements
			Comparable<NodeToolEntry> {
		/**
		 * @generated NOT
		 */
		private final PartLoader partLoader;

		/**
		 * @generated NOT
		 */
		private final boolean core;

		/**
		 * @generated NOT
		 */
		protected final List elementTypes;

		/**
		 * @generated NOT
		 */
		private NodeToolEntry(String title, String description,
				List elementTypes, PartLoader partLoader, boolean core) {
			super(partLoader.getTitle(), partLoader.getDescription(), null,
					null);

			// eventually passes the partloader along to PartCreateCommand
			this.partLoader = partLoader;
			this.elementTypes = elementTypes;
			this.core = core;
		}

		/**
		 * @generated NOT
		 */
		public Tool createTool() {
			// eventually passes the partloader along to PartCreateCommand
			Tool tool = new GenericUnspecifiedTypeCreationTool(elementTypes,
					partLoader);
			tool.setProperties(getToolProperties());
			return tool;
		}

		/**
		 * @generated NOT
		 */
		public int compareTo(NodeToolEntry other) {
			if (this.core && !other.core)
				return -1;
			if (other.core && !this.core)
				return 1;

			return this.partLoader.getTitle().compareTo(
					other.partLoader.getTitle());
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

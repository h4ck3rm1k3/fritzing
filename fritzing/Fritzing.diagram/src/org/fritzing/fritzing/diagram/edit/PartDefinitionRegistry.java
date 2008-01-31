/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.edit;

import java.util.HashMap;
import java.util.Map;

import org.fritzing.fritzing.diagram.part.FritzingDiagramEditorUtil;

/**
 * Manages PartLoader resources.
 * 
 */


public class PartDefinitionRegistry {

	/**
	 * Singleton instance for the partLoader registry.
	 */
	private static PartDefinitionRegistry singletonInstance = new PartDefinitionRegistry();

	/**
	 * Return singleton instance of the partLoader registry.
	 * 
	 * @return the PartLoader registry
	 */
	public static PartDefinitionRegistry getInstance() {
		return singletonInstance;
	}

	// HashMap that holds onto partLoader instances
	private final static Map<String, PartDefinition> partDefinitionRegistry = new HashMap<String, PartDefinition>();

	/**
	 * Private constructor.
	 */
	private PartDefinitionRegistry() {
		super();
	}

	
	public PartDefinition get(String path) {
		return this.get(FritzingDiagramEditorUtil.getFritzingLocation(), path, false);
	}

	/**
	 * Returns the PartDefinition based on the path. If the PartDefinition does not exist in the
	 * cache, creates a new one and caches.
	 * 
	 * @param path -
	 *            a string, either genus.species or the relative path (e.g. libraries/core/arduino/partDescription.xml)
	 * @return PartDefinition
	 */
	public PartDefinition get(String root, String path, boolean getDocument) {
		PartDefinition pd = partDefinitionRegistry.get(path);
		if (pd != null) {
			return pd;
		}
						
		pd = PartLoader.loadXMLFromLibrary(root, path, getDocument);
		
		partDefinitionRegistry.put(path, pd);
		if (pd.genus != null && pd.species != null) {
			partDefinitionRegistry.put(pd.genus + pd.species, pd);
		}
		return pd;
	}


}


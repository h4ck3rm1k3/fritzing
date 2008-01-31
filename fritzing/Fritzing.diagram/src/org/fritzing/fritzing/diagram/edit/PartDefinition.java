package org.fritzing.fritzing.diagram.edit;

import java.io.File;
import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Iterator;

import org.eclipse.core.runtime.FileLocator;
import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.draw2d.geometry.Point;
import org.eclipse.jface.resource.ImageDescriptor;
import org.fritzing.fritzing.diagram.edit.PartLoader.PointName;
import org.fritzing.fritzing.diagram.edit.PartLoader.PointPoint;
import org.fritzing.fritzing.diagram.providers.FritzingElementTypes;
import org.w3c.dom.Document;

public class PartDefinition {
	protected Hashtable<String, PointName> terminals;
	protected Point gridOffset;
	protected Hashtable<Double, String> bitmapFilenames;
	protected String svgFilename;
	protected Dimension size;
	protected boolean loaded;
	protected String iconFilename;
	protected String largeIconFilename;
	protected String description;
	protected String species;
	protected String genus;
	protected String contentsPath;
	protected String label;
	protected String title;
	protected boolean generic;
	protected String version;
	protected String footprint;
	protected ArrayList<ArrayList<PointName>> nets;
	protected Hashtable<String, Boolean> tracksVisible = new Hashtable<String, Boolean>();
	protected Document doc = null;
	protected File documentFile = null;
	protected String zorder = null;
	
	public final static String REQUEST_PARAM = "partDefinition";

	public PartDefinition() {				
		nets = new ArrayList<ArrayList<PointName>>();
		contentsPath = "";
		terminals = new Hashtable<String, PointName>();
		size = new Dimension(0,0);
		gridOffset = new Point(0,0);
		bitmapFilenames = new Hashtable<Double, String>();
		svgFilename = null;
	}

	public boolean isLoaded() {
		return loaded;
	}
		
	public boolean isGeneric() {
		return generic;
	}
		
	public Dimension getSize() {
		return size;
	}
	
	public boolean getTrackVisible(String trackString) {
		Boolean b = tracksVisible.get(trackString);
		if (b == null) return false;
		
		return b;
	}
	
	public void setTrackVisible(String trackId, boolean visible) {
		tracksVisible.put(trackId, visible);
	}
		
	public Point getTerminalLegTargetPosition(String id) {
		if (terminals == null) return new Point(0,400);
		
		PointName pointName = terminals.get(id);
		if (pointName == null) return new Point(0,400);
		
		if (pointName.points.size() < 2) return new Point(0,400);
				
		PointPoint pp0 = pointName.points.get(0);		
		PointPoint pp1 = pointName.points.get(1);
		
		return new Point(pp1.modified.x - pp0.modified.x, pp1.modified.y - pp0.modified.y);	
	}
	
	public void addTerminal(String id, PointName pn) {
		terminals.put(id, pn);
	}

//	public Hashtable<String, PointName> getTerminals() {
//		return terminals;
//	}
	
	public Enumeration<String> getTerminalIds() {
		return terminals.keys();
	}
	
	public PointName getTerminalData(String id) {
		return terminals.get(id);
	}
	
	public Point getTerminalPoint(String id) {
		if (terminals == null) return null;
	
		PointName pointName = terminals.get(id);
		if (pointName == null) return null;
		
		if (pointName.points.size() < 1) return null;
		
		return pointName.points.get(0).modified;
	}
	
	public String getVersion() {
		return version;
	}
	
	public String getTerminalType(String id) {
		if (terminals == null) return null;
	
		PointName pointName = terminals.get(id);
		if (pointName == null) return null;
		
		return pointName.type;
	}
	
	public boolean getTerminalLabelVisible(String id) {
		if (terminals == null) return false;
	
		PointName pointName = terminals.get(id);
		if (pointName == null) return false;
		
		return pointName.visible;
	}
	
	public boolean getTerminalFemale(String id) {
		if (terminals == null) return false;
		
		PointName pointName = terminals.get(id);
		if (pointName == null) return false;
		
		if (pointName.type == null) return false;
		
		// at the moment, male and female are treated the same
		return pointName.type.equalsIgnoreCase("female") || pointName.type.equalsIgnoreCase("male");		
	}
	
		
	public String getSvgFilename() {
		return svgFilename;
	}
	
	public Hashtable<Double,String> getBitmapFilenames() {
		return bitmapFilenames;
	}
	
	public String getIconFilename() {
		return iconFilename;
	}
	
	public String getLargeIconFilename() {
		return largeIconFilename;
	}
	
	public String getLabel() {
		return label;
	}
	
	public String getTitle() {
		return title;
	}
	
	public String getContentsPath() {
		return contentsPath;
	}
	
	public String getDescription() {
		return description;
	}
	
	public String getSpecies() {
		return species;
	}
	
	public String getGenus() {
		return genus;
	}

	/**
	 * @return the gridOffset
	 */
	public Point getGridOffset() {
		return gridOffset;
	}

	/**
	 * @param gridOffset the gridOffset to set
	 */
	public void setGridOffset(Point gridOffset) {
		this.gridOffset = gridOffset;
	}

	/**
	 * @return the footprint
	 */
	public String getFootprint() {
		return footprint;
	}

	/**
	 * @param footprint the footprint to set
	 */
	public void setFootprint(String footprint) {
		this.footprint = footprint;
	}

	/**
	 * @return the nets
	 */
	public Iterator<ArrayList<PointName>> getNets() {
		return nets.iterator();
	}

	/*
	 * @return if it has nets
	 */
	public boolean hasNets() {
		return nets.size() > 0;
	}
	
	/**
	 * @param nets the nets to set
	 */
	public void addNet(ArrayList<PointName> net) {
		nets.add(net);
	}

	/**
	 * @return the doc
	 */
	public Document getDocument() {
		return doc;
	}

	/**
	 * @param doc the doc to set
	 */
	public void setDocument(Document doc) {
		this.doc = doc;
	}

	/**
	 * @return the documentFile
	 */
	public File getDocumentFile() {
		return documentFile;
	}

	/**
	 * @param documentFile the documentFile to set
	 */
	public void setDocumentFile(File documentFile) {
		this.documentFile = documentFile;
	}

	/**
	 * @return the zorder
	 */
	public String getZOrder() {
		return zorder;
	}

	/**
	 * @param zorder the zorder to set
	 */
	public void setZOrder(String zorder) {
		this.zorder = zorder;
	}

	/**
	 * @param bitmapFilenames the bitmapFilenames to set
	 */
	public void addBitmapFilename(Double zoom, String bitmapFilename) {
		bitmapFilenames.put(zoom, bitmapFilename);
		
		ImageDescriptor desc;
		try {
			desc = ImageDescriptor.createFromURL(
					FileLocator.toFileURL(
							new URL("file", null, getContentsPath() + bitmapFilename)));
			FritzingElementTypes.getImageRegistry().put(
					getContentsPath() + bitmapFilename, desc);
		} catch (MalformedURLException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	/**
	 * @param svgFilename the svgFilename to set
	 */
	public void setSvgFilename(String svgFilename) {
		this.svgFilename = svgFilename;
	}

	/**
	 * @param size the size to set
	 */
	public void setSize(Dimension size) {
		this.size = size;
	}

	/**
	 * @param loaded the loaded to set
	 */
	public void setLoaded(boolean loaded) {
		this.loaded = loaded;
	}

	/**
	 * @param iconFilename the iconFilename to set
	 */
	public void setIconFilename(String iconFilename) {
		this.iconFilename = iconFilename;
	}

	/**
	 * @param largeIconFilename the largeIconFilename to set
	 */
	public void setLargeIconFilename(String largeIconFilename) {
		this.largeIconFilename = largeIconFilename;
	}

	/**
	 * @param description the description to set
	 */
	public void setDescription(String description) {
		this.description = description;
	}

	/**
	 * @param species the species to set
	 */
	public void setSpecies(String species) {
		this.species = species;
	}

	/**
	 * @param genus the genus to set
	 */
	public void setGenus(String genus) {
		this.genus = genus;
	}

	/**
	 * @param contentsPath the contentsPath to set
	 */
	public void setContentsPath(String contentsPath) {
		this.contentsPath = contentsPath;
	}

	/**
	 * @param label the label to set
	 */
	public void setLabel(String label) {
		this.label = label;
	}

	/**
	 * @param title the title to set
	 */
	public void setTitle(String title) {
		this.title = title;
	}

	/**
	 * @param generic the generic to set
	 */
	public void setGeneric(boolean generic) {
		this.generic = generic;
	}

	/**
	 * @param version the version to set
	 */
	public void setVersion(String version) {
		this.version = version;
	}
	
	
}

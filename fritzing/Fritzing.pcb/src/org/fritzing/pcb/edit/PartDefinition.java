package org.fritzing.pcb.edit;

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
import org.eclipse.emf.ecore.EObject;
import org.eclipse.jface.resource.ImageDescriptor;
import org.fritzing.pcb.providers.FritzingElementTypes;
import org.w3c.dom.Document;

public class PartDefinition {
	/*
	 * Absolute path to the folder of partdescription.xml
	 */
	protected String contentsPath;
	/*
	 * The partdescription.xml file
	 */
	protected File documentFile = null;
	/*
	 * Flag if the definition was loaded
	 */
	protected boolean loaded;
	/*
	 * The XML document, needed by the PartDefinitionUpdater
	 */
	protected Document doc = null;

	protected boolean generic;
	protected String species;
	protected String genus;
	protected String title;
	protected String description;
	protected String version;
	protected String label;
	protected URL reference;
	protected ArrayList<Author> authors;
	protected String iconSmallFilename;
	protected String iconLargeFilename;
	protected Dimension size;
	protected Point gridOffset;
	protected Hashtable<String, TerminalDefinition> terminals;
	protected ArrayList<ArrayList<TerminalDefinition>> nets;
	protected Hashtable<String, Boolean> tracksVisible = new Hashtable<String, Boolean>();
	protected String zorder = null;
	protected Hashtable<Double, String> bitmapFilenames;
	protected String svgFilename;
	protected String footprint;
	
	public final static String REQUEST_PARAM = "partDefinition";

	public PartDefinition() {				
		nets = new ArrayList<ArrayList<TerminalDefinition>>();
		authors = new ArrayList<Author>();
		contentsPath = "";
		terminals = new Hashtable<String, TerminalDefinition>();
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
		return size.getCopy();
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
		
		TerminalDefinition terminal = terminals.get(id);
		if (terminal == null) return new Point(0,400);
		
		if (terminal.points.size() < 2) return new Point(0,400);
			
		PointPoint pp0 = terminal.points.get(0);		
		PointPoint pp1 = terminal.points.get(1);
		
		return new Point(pp1.modified.x - pp0.modified.x, pp1.modified.y - pp0.modified.y);	
	}
	
	public void addTerminal(String id, TerminalDefinition pn) {
		terminals.put(id, pn);
	}

//	public Hashtable<String, PointName> getTerminals() {
//		return terminals;
//	}
	
	public Enumeration<String> getTerminalIds() {
		return terminals.keys();
	}
	
	public TerminalDefinition getTerminalDefinition(String id) {
		return terminals.get(id);
	}
	
	public Point getTerminalPoint(String id) {
		if (terminals == null) return null;
	
		TerminalDefinition terminal = terminals.get(id);
		if (terminal == null) return null;
		
		if (terminal.points.size() < 1) return null;
		
		return terminal.points.get(0).modified;
	}
	
	public String getVersion() {
		return version;
	}
	
	public String getTerminalType(String id) {
		if (terminals == null) return null;
	
		TerminalDefinition terminal = terminals.get(id);
		if (terminal == null) return null;
		
		return terminal.type;
	}
	
	public boolean getTerminalLabelVisible(String id) {
		if (terminals == null) return false;
	
		TerminalDefinition terminal = terminals.get(id);
		if (terminal == null) return false;
		
		return terminal.visible;
	}
	
	public boolean getTerminalFemale(String id) {
		if (terminals == null) return false;
		
		TerminalDefinition terminal = terminals.get(id);
		if (terminal == null) return false;
		
		if (terminal.type == null) return false;
		
		// at the moment, male and female are treated the same
		return terminal.type.equalsIgnoreCase("female") || terminal.type.equalsIgnoreCase("male");		
	}
	
		
	public String getSvgFilename() {
		return svgFilename;
	}
	
	public Hashtable<Double,String> getBitmapFilenames() {
		return bitmapFilenames;
	}
	
	public String getIconSmallFilename() {
		return iconSmallFilename;
	}
	
	public String getIconLargeFilename() {
		return iconLargeFilename;
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
	public Iterator<ArrayList<TerminalDefinition>> getNets() {
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
	public void addNet(ArrayList<TerminalDefinition> net) {
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
	 * @param iconSmallFilename the iconFilename to set
	 */
	public void setIconSmallFilename(String iconSmallFilename) {
		this.iconSmallFilename = iconSmallFilename;
	}

	/**
	 * @param largeIconFilename the largeIconFilename to set
	 */
	public void setIconLargeFilename(String iconLargeFilename) {
		this.iconLargeFilename = iconLargeFilename;
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

	/**
	 * @return the reference
	 */
	public URL getReference() {
		return reference;
	}

	/**
	 * @param reference the reference to set
	 */
	public void setReference(URL reference) {
		this.reference = reference;
	}

	/**
	 * @return the authors
	 */
	public Iterator<Author> getAuthors() {
		return authors.iterator();
	}

	/**
	 * @return the number of authors
	 */
	public int getAuthorsNum() {
		return authors.size();
	}
	
	/**
	 * @param author the author to add
	 */
	public void addAuthor(Author author) {
		authors.add(author);
	}

	/*
	 * Original point location and the translated one, including
	 * unit conversion and grid offset
	 */
	static class PointPoint {
		
		public Point original;
		public Point modified;
		
		public PointPoint() {
			modified = new Point(0,0);
			original = new Point(0,0);
		}
		
		public PointPoint(Point p1, Point p2) {
			original = p1;
			modified = p2;
		}
	}
	
	static class TerminalDefinition {
		public ArrayList<PointPoint> points;
		public String name;
		public boolean visible;
		public EObject terminal;
		public String type;
		
		public TerminalDefinition(String name, boolean visible, String type) {
			points = new ArrayList<PointPoint>();
			this.name = name;
			this.visible = visible;
			this.terminal = null;
			this.type = type;
		}
		
		public void addPoint(PointPoint p) {
			points.add(p);
		}
	}
	
	public static class Author {
		public String name;
		public URL url;

		public Author(String name) {
			this.name = name;
		}
		
		public Author(String name, URL url) {
			this.name = name;
			this.url = url;
		}
	}
	
}

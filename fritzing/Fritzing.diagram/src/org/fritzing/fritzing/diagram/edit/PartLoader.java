/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.edit;

import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.util.Collection;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.List;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathExpressionException;
import javax.xml.xpath.XPathFactory;

import org.eclipse.core.runtime.Platform;
import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.draw2d.geometry.Point;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.emf.ecore.EStructuralFeature;
import org.fritzing.fritzing.FritzingPackage;
import org.fritzing.fritzing.Part;
import org.fritzing.fritzing.diagram.expressions.FritzingAbstractExpression;
import org.fritzing.fritzing.diagram.expressions.FritzingOCLFactory;
import org.fritzing.fritzing.diagram.part.FritzingDiagramEditorUtil;
import org.w3c.dom.Document;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;
import org.xml.sax.SAXParseException;

import java.util.UUID;


public class PartLoader {
	
	static protected Hashtable<Character, String> bitHash = new Hashtable<Character, String>();
	static protected Hashtable<String, Character> c64Hash = new Hashtable<String, Character>();
	static protected String[] zeroes = { "000000", "00000", "0000", "000", "00", "0", "" };
	
	protected Hashtable<String, PointName> terminalHash;
	protected Point gridOffset;
	protected String bitmapFilename;
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
	
	public PartLoader() {
		if (bitHash.size() == 0) {
			bitHash.put('0', "0000");
			bitHash.put('1', "0001");
			bitHash.put('2', "0010");
			bitHash.put('3', "0011");
			bitHash.put('4', "0100");
			bitHash.put('5', "0101");
			bitHash.put('6', "0110");
			bitHash.put('7', "0111");
			bitHash.put('8', "1000");
			bitHash.put('9', "1001");
			bitHash.put('a', "1010");
			bitHash.put('b', "1011");
			bitHash.put('c', "1100");
			bitHash.put('d', "1101");
			bitHash.put('e', "1110");
			bitHash.put('f', "1111");
			bitHash.put('A', "1010");
			bitHash.put('B', "1011");
			bitHash.put('C', "1100");
			bitHash.put('D', "1101");
			bitHash.put('E', "1110");
			bitHash.put('F', "1111");
		}
		if (c64Hash.size() == 0) {
			c64Hash.put("000000", '0');	
			c64Hash.put("000001", '1');	
			c64Hash.put("000010", '2');	
			c64Hash.put("000011", '3');	
			c64Hash.put("000100", '4');	
			c64Hash.put("000101", '5');	
			c64Hash.put("000110", '6');	
			c64Hash.put("000111", '7');	
			c64Hash.put("001000", '8');	
			c64Hash.put("001001", '9');	
			c64Hash.put("001010", 'a');	
			c64Hash.put("001011", 'b');	
			c64Hash.put("001100", 'c');	
			c64Hash.put("001101", 'd');	
			c64Hash.put("001110", 'e');	
			c64Hash.put("001111", 'f');	
			c64Hash.put("010000", 'g');	
			c64Hash.put("010001", 'h');	
			c64Hash.put("010010", 'i');	
			c64Hash.put("010011", 'j');	
			c64Hash.put("010100", 'k');	
			c64Hash.put("010101", 'l');	
			c64Hash.put("010110", 'm');	
			c64Hash.put("010111", 'n');	
			c64Hash.put("011000", 'o');	
			c64Hash.put("011001", 'p');	
			c64Hash.put("011010", 'q');	
			c64Hash.put("011011", 'r');	
			c64Hash.put("011100", 's');	
			c64Hash.put("011101", 't');	
			c64Hash.put("011110", 'u');	
			c64Hash.put("011111", 'v');	
			c64Hash.put("100000", 'w');	
			c64Hash.put("100001", 'x');	
			c64Hash.put("100010", 'y');	
			c64Hash.put("100011", 'z');	
			c64Hash.put("100100", 'A');	
			c64Hash.put("100101", 'B');	
			c64Hash.put("100110", 'C');	
			c64Hash.put("100111", 'D');	
			c64Hash.put("101000", 'E');	
			c64Hash.put("101001", 'F');	
			c64Hash.put("101010", 'G');	
			c64Hash.put("101011", 'H');	
			c64Hash.put("101100", 'I');	
			c64Hash.put("101101", 'J');	
			c64Hash.put("101110", 'K');	
			c64Hash.put("101111", 'L');	
			c64Hash.put("110000", 'M');	
			c64Hash.put("110001", 'N');	
			c64Hash.put("110010", 'O');	
			c64Hash.put("110011", 'P');	
			c64Hash.put("110100", 'Q');	
			c64Hash.put("110101", 'R');	
			c64Hash.put("110110", 'S');	
			c64Hash.put("110111", 'T');	
			c64Hash.put("111000", 'U');	
			c64Hash.put("111001", 'V');	
			c64Hash.put("111010", 'X');	
			c64Hash.put("111011", 'X');	
			c64Hash.put("111100", 'Y');	
			c64Hash.put("111101", 'Z');	
			c64Hash.put("111110", '_');	
			c64Hash.put("111111", '-');	
		}
		
		contentsPath = "";
		terminalHash = new Hashtable<String, PointName>();
		size = new Dimension(0,0);
		gridOffset = new Point(0,0);
		bitmapFilename = svgFilename = null;
	}
	
	public boolean getLoaded() {
		return loaded;
	}
		
	public boolean isGeneric() {
		return generic;
	}
		
	public Dimension getSize() {
		return size;
	}
	
	public Point getTerminalPoint(String id) {
		if (terminalHash == null) return null;
	
		PointName pointName = terminalHash.get(id);
		if (pointName == null) return null;
		
		return pointName.point;
	}
	
	public boolean getTerminalLabelVisible(String id) {
		if (terminalHash == null) return false;
	
		PointName pointName = terminalHash.get(id);
		if (pointName == null) return false;
		
		return pointName.visible;
	}
	
	public String getSvgFilename() {
		return svgFilename;
	}
	
	public String getBitmapFilename() {
		return bitmapFilename;
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
	
	public boolean initialize(String path, EObject newElement) {
		try {
			
//			URL url = FileLocator.find(FritzingDiagramEditorPlugin
//			.getInstance().getBundle(), new Path(
//			"icons/parts/partdescription.xml"), null);
//			url = FileLocator.toFileURL(url);

			if (!loadXMLFromLibrary(path)) return false;
			
			return initialize(newElement);
		}
		catch (Exception ex) {
			
		}
		
		return false;
	}
	
	public String genID() {
		String id = UUID.randomUUID().toString();
		id = id.replaceAll("-", "");
		
		StringBuffer bitString = new StringBuffer();
		for (int i = 0; i < id.length(); i++) {
			bitString.append(bitHash.get(id.charAt(i)));
		}
		
		StringBuffer id64 = new StringBuffer();
		String c64;
		for (int i = 0; i < bitString.length(); i += 6) {
			try {
				c64 = bitString.substring(i, i + 6);
			}
			catch (Exception ex) {
				c64 = bitString.substring(i);
				c64 = zeroes[c64.length()] + c64;
			}
			
			id64.append(c64Hash.get(c64));
		}
		
		
		
		return id64.toString();
	}
				
	public boolean initialize(EObject newElement) {
		if (!this.loaded) return false;
		
		if (newElement instanceof Part) {
			((Part) newElement).setId(genID());
			((Part) newElement).setSpecies(species);
			((Part) newElement).setGenus(genus);
			((Part) newElement).setVersion(version);
			((Part) newElement).setFootprint(footprint);
			((Part) newElement).setDescription(description);
		}
			
		try {
			
			for (Enumeration<String> e = terminalHash.keys(); e.hasMoreElements();) {
				String id = e.nextElement();
				if (id == null || id == "") continue;
								
				PointName pointName = terminalHash.get(id);
				if (pointName == null) continue;

				EObject terminal = FritzingPackage.eINSTANCE.getTerminal()
						.getEPackage().getEFactoryInstance().create(
								FritzingPackage.eINSTANCE.getTerminal());

				EStructuralFeature feature = FritzingPackage.eINSTANCE
						.getPart_Terminals();
				((Collection) newElement.eGet(feature)).add(terminal);
				
				FritzingAbstractExpression expr = FritzingOCLFactory
						.getExpression("\'" + pointName.name + "\'",
								FritzingPackage.eINSTANCE.getTerminal());
				expr.assignTo(FritzingPackage.eINSTANCE.getTerminal_Name(),
						terminal);

				expr = FritzingOCLFactory
				.getExpression("\'" + id + "\'",
						FritzingPackage.eINSTANCE.getTerminal());
				expr.assignTo(FritzingPackage.eINSTANCE.getTerminal_Id(),
				terminal);

			}
			
			EStructuralFeature feature = FritzingPackage.eINSTANCE.getPart_Name();
			FritzingAbstractExpression expr = FritzingOCLFactory
					.getExpression("\'" + label + "\'",
							newElement.eClass());
			expr.assignTo(feature, newElement);
						
			return true;
	
		}
		catch (Exception ex) {
		
		}
		
		return false;
	}
	
	public boolean loadXMLFromLibrary(String path) {
		try {
			URL url = new URL("File://" + FritzingDiagramEditorUtil.getFritzingLocation() + path);
			File f = new File(FritzingDiagramEditorUtil.getFritzingLocation() + path);
			contentsPath = f.getParent() + File.separator;
			return loadXML(url);	
		}
		catch (Exception ex) {
			
		}
		
		return false;
	}
	
	public boolean loadXML(URL xml) {
	    Document document;
	    DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();

	    factory.setIgnoringComments(true);
	    factory.setIgnoringElementContentWhitespace(true);
	    
	    //factory.setValidating(true);   
	    //factory.setNamespaceAware(true);
        try {
            DocumentBuilder builder = factory.newDocumentBuilder();
            document = builder.parse(new File(xml.getFile()));
            parseXML(document);
            loaded = true;
            return true;           
        } 
        catch (SAXParseException spe) {
            // Error generated by the parser
            System.out.println("\n** Parsing error" + ", line " +
                spe.getLineNumber() + ", uri " + spe.getSystemId());
            System.out.println("   " + spe.getMessage());

            // Use the contained exception, if any
            Exception x = spe;

            if (spe.getException() != null) {
                x = spe.getException();
            }

            x.printStackTrace();
        } 
        catch (SAXException sxe) {
            // Error generated during parsing)
            Exception x = sxe;

            if (sxe.getException() != null) {
                x = sxe.getException();
            }

            x.printStackTrace();
        } 
        catch (ParserConfigurationException pce) {
            // Parser with specified options can't be built
            pce.printStackTrace();
        } 
        catch (IOException ioe) {
            // I/O error
        }	
        catch (Exception ex) {
        }
        
        return false;

	}
	protected void parseXML(Document document) {
		XPath xp = XPathFactory.newInstance().newXPath();
		try {
			String gString = (String) xp.evaluate("/part/@generic", document, XPathConstants.STRING);
			generic = !gString.equalsIgnoreCase("false");
			
			species = (String) xp.evaluate("/part/species", document, XPathConstants.STRING);
			genus = (String) xp.evaluate("/part/genus", document, XPathConstants.STRING);
			description = (String) xp.evaluate("/part/description", document, XPathConstants.STRING);
			title = (String) xp.evaluate("/part/title", document, XPathConstants.STRING);
			label = (String) xp.evaluate("/part/label", document, XPathConstants.STRING);
			version = (String) xp.evaluate("/part/version", document, XPathConstants.STRING);
			footprint = (String) xp.evaluate("/part/footprints/footprint/@source", document, XPathConstants.STRING);
			
			parseIcons((NodeList) xp.evaluate("/part/icons/icon", document, XPathConstants.NODESET));
			parseImages((NodeList) xp.evaluate("/part/layers/layer", document, XPathConstants.NODESET));
			
			String defaultUnits = (String) xp.evaluate("/part/defaultUnits", document, XPathConstants.STRING);
			if (defaultUnits == null || defaultUnits == "") {
				defaultUnits = "pixels";
			}
			
			Node gridOffsetNode = (Node) xp.evaluate("/part/gridOffset", document, XPathConstants.NODE);			
			gridOffset = parseLocation(gridOffsetNode, defaultUnits, "x", "y");
			
			Node boundsNode = (Node) xp.evaluate("/part/bounds", document, XPathConstants.NODE);			
			Point sz = parseLocation(boundsNode, defaultUnits, "width", "height");
			
			size = new Dimension(sz.x, sz.y);
			
			Node defaultLayoutNode = (Node) xp.evaluate("/part/connectors", document, XPathConstants.NODE);
			boolean defaultTerminalLabelVisible = parseTerminalLabelLayout(true, defaultLayoutNode);
			

			NodeList nodes = (NodeList) xp.evaluate("/part/connectors/connector", document, XPathConstants.NODESET);
			for (int i = 0; i < nodes.getLength(); i++) {
				Node child = nodes.item(i);
				XPath xpath = XPathFactory.newInstance().newXPath();
				String id = (String) xpath.evaluate("@id", child, XPathConstants.STRING);
				if (id == null || id == "") continue;
											
				String name = (String) xpath.evaluate("@name", child, XPathConstants.STRING);
				if (name == null || id == "") {
					name = id;
				}

				Point p = parseLocation(child, defaultUnits, "x", "y" );
				p.x += gridOffset.x;
				p.y += gridOffset.y;
				boolean visible = parseTerminalLabelLayout(defaultTerminalLabelVisible, child);
				terminalHash.put(id, new PointName(p, name, visible));
			}
			return;
		}
		catch (XPathExpressionException xpee) {
			// alert the user
			return;
		}		
	}
	
	protected boolean parseTerminalLabelLayout(boolean defaultValue, Node node) {
		XPath xpath = XPathFactory.newInstance().newXPath();
		Node child = null;
		try {
			child = (Node) xpath.evaluate("nameLayout", node, XPathConstants.NODE);
			String value = (String) xpath.evaluate("@visible", child, XPathConstants.STRING);
			return !(value.equalsIgnoreCase("false"));
		}
		catch (Exception ex) {
		}
		
		return defaultValue;			
	}
		
	protected void parseImages(NodeList nodeList) {
		XPath xpath = XPathFactory.newInstance().newXPath();
		for (int i = 0; i < nodeList.getLength(); i++) {
			try {
				Node node = nodeList.item(i);
				String type = (String) xpath.evaluate("@type", node, XPathConstants.STRING);
				if (type.equalsIgnoreCase("image")) {
					NodeList imageList = (NodeList) xpath.evaluate("image", node, XPathConstants.NODESET);
					
					// need to add multiple zoom images, etc...
					for (int j = 0; j < imageList.getLength(); j++ ) {
						Node imageNode = imageList.item(j);
						type = (String) xpath.evaluate("@type", imageNode, XPathConstants.STRING);
						if (type.equalsIgnoreCase("bitmap")) {
							bitmapFilename = (String) xpath.evaluate("@source", imageNode, XPathConstants.STRING);
						}
						else if (type.equalsIgnoreCase("svg")) {
							svgFilename = (String) xpath.evaluate("@source", imageNode, XPathConstants.STRING);
						}
					}
					
				}
			}
			catch (Exception ex) {
				ex.printStackTrace();
				// alert the user
			}
		}
	}
	
	protected void parseIcons(NodeList nodeList) {
		XPath xpath = XPathFactory.newInstance().newXPath();
		for (int i = 0; i < nodeList.getLength(); i++) {
			try {
				Node node = nodeList.item(i);
				String size = (String) xpath.evaluate("@size", node, XPathConstants.STRING);
				if (size.equalsIgnoreCase("small")) {
					iconFilename = (String) xpath.evaluate("@source", node, XPathConstants.STRING);
				}
				else if (size.equalsIgnoreCase("large")) {
					largeIconFilename = (String) xpath.evaluate("@source", node, XPathConstants.STRING);
				}
			}
			catch (Exception ex) {
				// alert the user
			}
		}
	}

	protected Point parseLocation(Node node, String defaultUnits, String axis1, String axis2) {
		if (node == null) return new Point(0,0);
		
		try {
			XPath xpath = XPathFactory.newInstance().newXPath();
			Double x = (Double) xpath.evaluate("@" + axis1, node, XPathConstants.NUMBER);
			Double y = (Double) xpath.evaluate("@" + axis2, node, XPathConstants.NUMBER);
			String units = (String) xpath.evaluate("@units", node, XPathConstants.STRING);
			if (units == null || units == "") {
				units = defaultUnits;
			}
			if (units.equalsIgnoreCase("himetric")) {			
			}
			else if (units.equalsIgnoreCase("tenths")) {
				x *= 254;  // 25400 / dpi;  
				y *= 254;  // 25400 / dpi;
			}
			else if (units.equalsIgnoreCase("inches")) {
				x *= 2540;  // 254000 / dpi;
				y *= 2540;  // 254000 / dpi;
			}
//			else if (units.equalsIgnoreCase("pixels")) {
//				x *= 2540 / dpi;  // 2540 / 100
//				y *= 2540 / dpi;  // 2540 / 100
//			}
			else {
				throw new Exception("unknown units " + units);
			}
			
			return new Point(x, y);

		}
		catch (Exception ex) {
			// tell the user something's wrong
		}
		
		return new Point();
	}
	
	static class PointName {
		public Point point;
		public String name;
		public boolean visible;
		
		public PointName(Point point, String name, boolean visible) {
			this.point = point;
			this.name = name;
			this.visible = visible;
		}
	}

}

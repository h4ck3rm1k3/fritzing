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
import org.fritzing.fritzing.diagram.expressions.FritzingAbstractExpression;
import org.fritzing.fritzing.diagram.expressions.FritzingOCLFactory;
import org.fritzing.fritzing.diagram.part.FritzingDiagramEditorUtil;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;
import org.xml.sax.SAXParseException;

public class PartLoader {
	
	protected Hashtable<String, Point> terminalHash;
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
	
	public PartLoader() {
		contentsPath = "";
		terminalHash = new Hashtable<String, Point>();
		size = new Dimension(0,0);
		gridOffset = new Point(0,0);
		bitmapFilename = svgFilename = null;
	}
	
	public boolean getLoaded() {
		return loaded;
	}
	
	public Enumeration<String> getTerminalKeys() {
		if (terminalHash == null) return null;
		
		return terminalHash.keys();
	}
	
	public Dimension getSize() {
		return size;
	}
	
	public Point getTerminalPoint(String name) {
		if (terminalHash == null) return null;
	
		return terminalHash.get(name);
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
	
	public boolean createTerminals(String path, EObject newElement) {
		try {
			
//			URL url = FileLocator.find(FritzingDiagramEditorPlugin
//			.getInstance().getBundle(), new Path(
//			"icons/parts/partdescription.xml"), null);
//			url = FileLocator.toFileURL(url);

			if (!loadXMLFromLibrary(path)) return false;
			
			return createTerminals(newElement);
		}
		catch (Exception ex) {
			
		}
		
		return false;
	}
				
	public boolean createTerminals(EObject newElement) {
		if (!this.loaded) return false;
		
		try {
			
			for (Enumeration<String> e = getTerminalKeys(); e
					.hasMoreElements();) {
				String name = e.nextElement();
				if (name == null || name == "")
					continue;

				EObject terminal = FritzingPackage.eINSTANCE.getTerminal()
						.getEPackage().getEFactoryInstance().create(
								FritzingPackage.eINSTANCE.getTerminal());

				EStructuralFeature feature = FritzingPackage.eINSTANCE
						.getPart_Terminals();
				((Collection) newElement.eGet(feature)).add(terminal);
				
				FritzingAbstractExpression expr = FritzingOCLFactory
						.getExpression("\'" + name + "\'",
								FritzingPackage.eINSTANCE.getTerminal());
				expr.assignTo(FritzingPackage.eINSTANCE.getTerminal_Name(),
						terminal);
			}
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
            ioe.printStackTrace();
        }	
        catch (Exception ex) {
        	ex.printStackTrace();
        }
        
        return false;

	}
	protected void parseXML(Document document) {
		XPath xp = XPathFactory.newInstance().newXPath();
		try {
			species = (String) xp.evaluate("/part/species", document, XPathConstants.STRING);
			genus = (String) xp.evaluate("/part/genus", document, XPathConstants.STRING);
			description = (String) xp.evaluate("/part/description", document, XPathConstants.STRING);
			iconFilename = (String) xp.evaluate("/part/icon", document, XPathConstants.STRING);
			largeIconFilename = (String) xp.evaluate("/part/largeIcon", document, XPathConstants.STRING);
			
			String defaultUnits = (String) xp.evaluate("/part/defaultUnits", document, XPathConstants.STRING);
			if (defaultUnits == null || defaultUnits == "") {
				defaultUnits = "pixels";
			}
			
			Node gridOffsetNode = (Node) xp.evaluate("/part/gridOffset", document, XPathConstants.NODE);			
			gridOffset = parseLocation(gridOffsetNode, defaultUnits, "x", "y");
			
			Node boundsNode = (Node) xp.evaluate("/part/bounds", document, XPathConstants.NODE);			
			Point sz = parseLocation(boundsNode, defaultUnits, "width", "height");
			
			size = new Dimension(sz.x, sz.y);

			// add multiple layers, etc...
			svgFilename = (String) xp.evaluate("/part/children/layer/moduleData/shapeLayer/svgShapeFile", document, XPathConstants.STRING);
			bitmapFilename = (String) xp.evaluate("/part/children/layer/moduleData/shapeLayer/resolutionIndependentImageSet/bitmapImage/bitmapFile", document, XPathConstants.STRING);

			NodeList nodes = (NodeList) xp.evaluate("/part/children/layer/moduleData/connector", document, XPathConstants.NODESET);
			for (int i = 0; i < nodes.getLength(); i++) {
				Node child = nodes.item(i);
				XPath xpath = XPathFactory.newInstance().newXPath();
				String name = (String) xpath.evaluate("internalName", child, XPathConstants.STRING);
				if (name == null || name == "") continue;
				
				// eventually will need to add a child terminal part here
				// they will not be predefined in the generator
							
				Node loc = (Node) xpath.evaluate("connectorLocation", child, XPathConstants.NODE);
				Point p = parseLocation(loc, defaultUnits, "x", "y" );
				p.x += gridOffset.x;
				p.y += gridOffset.y;
				terminalHash.put(name, p);						
			}
			return;
		}
		catch (XPathExpressionException xpee) {
			// alert the user
			return;
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

}

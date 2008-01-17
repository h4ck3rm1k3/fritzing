/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.edit;

import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.UUID;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.eclipse.core.runtime.FileLocator;
import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.draw2d.geometry.Point;
import org.eclipse.draw2d.geometry.PointList;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.emf.ecore.EStructuralFeature;
import org.eclipse.gmf.runtime.draw2d.ui.mapmode.MapModeUtil;
import org.eclipse.jface.resource.ImageDescriptor;
import org.fritzing.fritzing.FritzingFactory;
import org.fritzing.fritzing.FritzingPackage;
import org.fritzing.fritzing.Leg;
import org.fritzing.fritzing.Part;
import org.fritzing.fritzing.Sketch;
import org.fritzing.fritzing.Terminal;
import org.fritzing.fritzing.diagram.edit.parts.Terminal2EditPart;
import org.fritzing.fritzing.diagram.expressions.FritzingAbstractExpression;
import org.fritzing.fritzing.diagram.expressions.FritzingOCLFactory;
import org.fritzing.fritzing.diagram.providers.FritzingElementTypes;
import org.w3c.dom.Attr;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;
import org.xml.sax.SAXParseException;


public class PartLoader {
	
	static protected Hashtable<Character, String> bitHash = new Hashtable<Character, String>();
	static protected Hashtable<String, Character> c64Hash = new Hashtable<String, Character>();
	static protected String[] zeroes = { "000000", "00000", "0000", "000", "00", "0", "" };
	
	protected Hashtable<String, PointName> terminalHash;
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
	protected Hashtable<String, Boolean> trackHash = new Hashtable<String, Boolean>();
	protected Document doc = null;
	protected File documentFile = null;
	
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
		
		nets = new ArrayList<ArrayList<PointName>>();
		contentsPath = "";
		terminalHash = new Hashtable<String, PointName>();
		size = new Dimension(0,0);
		gridOffset = new Point(0,0);
		bitmapFilenames = new Hashtable<Double, String>();
		svgFilename = null;
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
	
	public boolean getTrackVisible(String trackString) {
		Boolean b = trackHash.get(trackString);
		if (b == null) return false;
		
		return b;
	}
		
	public Point getTerminalLegTargetPosition(String id) {
		if (terminalHash == null) return new Point(0,400);
		
		PointName pointName = terminalHash.get(id);
		if (pointName == null) return new Point(0,400);
		
		if (pointName.points.size() < 2) return new Point(0,400);
				
		PointPoint pp0 = pointName.points.get(0);		
		PointPoint pp1 = pointName.points.get(1);
		
		return new Point(pp1.modified.x - pp0.modified.x, pp1.modified.y - pp0.modified.y);	
	}
	
	public Point getTerminalPoint(String id) {
		if (terminalHash == null) return null;
	
		PointName pointName = terminalHash.get(id);
		if (pointName == null) return null;
		
		if (pointName.points.size() < 1) return null;
		
		return pointName.points.get(0).modified;
	}
	
	public String getTerminalType(String id) {
		if (terminalHash == null) return null;
	
		PointName pointName = terminalHash.get(id);
		if (pointName == null) return null;
		
		return pointName.type;
	}
	
	public boolean getTerminalLabelVisible(String id) {
		if (terminalHash == null) return false;
	
		PointName pointName = terminalHash.get(id);
		if (pointName == null) return false;
		
		return pointName.visible;
	}
	
	public boolean getTerminalFemale(String id) {
		if (terminalHash == null) return false;
		
		PointName pointName = terminalHash.get(id);
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
	
	public boolean initialize(String root, String path, EObject newElement) {
		try {
			
//			URL url = FileLocator.find(FritzingDiagramEditorPlugin
//			.getInstance().getBundle(), new Path(
//			"icons/parts/partdescription.xml"), null);
//			url = FileLocator.toFileURL(url);

			if (!loadXMLFromLibrary(root, path, false)) return false;
			
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
		
		//System.out.println("initialize " + newElement + " " + System.currentTimeMillis());
		
		if (newElement instanceof Part) {
			((Part) newElement).setId(genID());
			((Part) newElement).setSpecies(species);
			((Part) newElement).setGenus(genus);
			((Part) newElement).setVersion(version);
			((Part) newElement).setFootprint(footprint);
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
				
				pointName.terminal = terminal;

				EStructuralFeature feature = FritzingPackage.eINSTANCE
						.getPart_Terminals();
				((Collection) newElement.eGet(feature)).add(terminal);
				
				FritzingAbstractExpression expr = FritzingOCLFactory
						.getExpression("\'" + pointName.name + "\'",
								FritzingPackage.eINSTANCE.getTerminal());
				expr.assignTo(FritzingPackage.eINSTANCE.getIElement_Name(),
						terminal);

				expr = FritzingOCLFactory
				.getExpression("\'" + id + "\'",
						FritzingPackage.eINSTANCE.getTerminal());
				expr.assignTo(FritzingPackage.eINSTANCE.getIElement_Id(),
				terminal);
				
				
				if (pointName.type.equalsIgnoreCase("leg")) {
					Leg leg = FritzingFactory.eINSTANCE.createLeg();
					leg.setSource((Terminal) terminal);
					leg.setParent((Terminal) terminal);
					((Terminal) terminal).setLeg(leg);

					Sketch parent = (Sketch) ((Part) newElement).getParent();
					leg.setTarget(((Sketch) parent));				
				}

			}
			
			if (nets.size() > 0) {
				for (int i = 0; i < nets.size(); i++) {
					ArrayList<PointName> net = nets.get(i);
					for (int j = 0; j < net.size() - 1; j++) {
						
						PointName source = net.get(j);
						if (source == null) {
							// alert user
							continue;														
						}
						
						PointName target = net.get(j + 1);
						if (target == null) {
							// alert user
							continue;														
						}
						
						PointName spn = terminalHash.get(source.name);
						if (spn == null) {
							// alert user
							continue;							
						}

						PointName tpn = terminalHash.get(target.name);
						if (tpn == null) {
							// alert user
							continue;							
						}
												
						EObject track = FritzingPackage.eINSTANCE.getTrack()
						.getEPackage().getEFactoryInstance().create(
								FritzingPackage.eINSTANCE.getTrack());
						
						EStructuralFeature feature = FritzingPackage.eINSTANCE.getTrack_Source();
						track.eSet(feature, spn.terminal);

						feature = FritzingPackage.eINSTANCE.getTrack_Target();
						track.eSet(feature, tpn.terminal);
						
						feature = FritzingPackage.eINSTANCE.getTrack_Parent();
						track.eSet(feature, newElement);
						
						feature = FritzingPackage.eINSTANCE
														.getPart_Tracks();
						((Collection) newElement.eGet(feature)).add(track);											
					}
				}
			}
			
			EStructuralFeature feature = FritzingPackage.eINSTANCE.getIElement_Name();
			FritzingAbstractExpression expr = FritzingOCLFactory
					.getExpression("\'" + label + "\'",
							newElement.eClass());
			expr.assignTo(feature, newElement);

			//System.out.println("done initialize " + newElement + " " + System.currentTimeMillis());

			return true;
	
		}
		catch (Exception ex) {
		
		}
		
		return false;
	}
	
	public boolean loadXMLFromLibrary(String root, String path, boolean saveDocument) {
		try {
			URL url = new URL("File://" + root + path);
			File f = new File(root + path);
			contentsPath = f.getParent() + File.separator;
			boolean result = loadXML(url, saveDocument);
			return result;
		}
		catch (Exception ex) {
			
		}
		
		return false;
	}
	
	public Document getDocument() {
		return doc;
	}
	
	public File getDocumentFile() {
		return documentFile;
	}
	
	public boolean loadXML(URL xml, boolean saveDocument) {
	    Document document;
	    DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
	    
	    factory.setIgnoringComments(!saveDocument);
	    factory.setIgnoringElementContentWhitespace(!saveDocument);
	    
	    //factory.setValidating(true);   
	    //factory.setNamespaceAware(true);
        try {
            DocumentBuilder builder = factory.newDocumentBuilder();
            documentFile = new File(xml.getFile());
            document = builder.parse(documentFile);
            parseXML(document);
            loaded = true;
            if (saveDocument) {
            	doc = document;
            }
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
        	ex.printStackTrace();
        }
        
        return false;
	}
	
	
	protected void parseXML(Document document) {

		try {
			Element partNode = document.getDocumentElement();
			if (!partNode.getNodeName().equals("part")) {
				// alert user
				return;								
			}
			
			generic = true;
			Attr attr = partNode.getAttributeNode("generic");
			if (attr != null) {
				String s = attr.getNodeValue();
				if (s != null) {
					generic = !s.equalsIgnoreCase("false");
				}
			}
						
			// do gridoffset and defaultunits first
			String defaultUnits = null;					
			NodeList nodes = partNode.getChildNodes();
			for (int i = 0; i < nodes.getLength(); i++) {
				Node node = nodes.item(i);
				String nodeName = node.getNodeName();
				if (nodeName.equals("defaultUnits")) {
					defaultUnits = node.getTextContent();					
				}
				else if (nodeName.equals("gridOffset")) {
					gridOffset = parseLocation(node, defaultUnits, "x", "y").modified;
				}
			}
			if (defaultUnits == null || defaultUnits == "") {
				defaultUnits = "pixels";
			}
					
			for (int i = 0; i < nodes.getLength(); i++) {
				Node node = nodes.item(i);
				String nodeName = node.getNodeName();
				if (nodeName.equals("species")) {
					species = node.getTextContent();
				}
				else if (nodeName.equals("genus")) {
					genus = node.getTextContent();				
				}
				else if (nodeName.equals("description")) {
					description = node.getTextContent();				
				}
				else if (nodeName.equals("title")) {
					title = node.getTextContent();					
				}
				else if (nodeName.equals("label")) {
					label = node.getTextContent();					
				}
				else if (nodeName.equals("version")) {
					version = node.getTextContent();					
				}
				else if (nodeName.equals("footprints")) {
					parseFootprints(node.getChildNodes());					
				}
				else if (nodeName.equals("icons")) {
					parseIcons(node.getChildNodes());		
				}
				else if (nodeName.equals("layers")) {
					parseLayers(node.getChildNodes());
				}
				else if (nodeName.equals("bounds")) {
					Point sz = parseLocation(node, defaultUnits, "width", "height").modified;					
					size = new Dimension(sz.x, sz.y);
				}
				else if (nodeName.equals("connectors")) {
					boolean defaultTerminalLabelVisible = parseTerminalLabelLayout(true, node);
					NodeList connectors = node.getChildNodes();
					for (int j = 0; j < connectors.getLength(); j++) {
						Node connector = connectors.item(j);
						if (!connector.getNodeName().equals("connector")) continue;
						
						attr = ((Element) connector).getAttributeNode("id");
						if (attr == null) continue;
						
						String id = attr.getNodeValue();
						if (id == null || id == "") continue;
						
						String name = id;
						attr = ((Element) connector).getAttributeNode("name");
						if (attr != null) {						
							String s = attr.getNodeValue();
							if (s != null && s != "") {
								name = s;
							}
						}
						
						String type = "";
						attr = ((Element) connector).getAttributeNode("type");
						if (attr != null) {						
							String s = attr.getNodeValue();
							if (s != null) {
								type = s;
							}
						}
						
						boolean visible = parseTerminalLabelLayout(defaultTerminalLabelVisible, connector);
						PointName pn = new PointName(name, visible, type);
						terminalHash.put(id, pn);

						// treat the terminal's location as the center rather than the top left
						// by offsetting the value from the xml;
						int d = MapModeUtil.getMapMode().DPtoLP(Terminal2EditPart.standardPlateMeasure / 2);
						
						NodeList nl = connector.getChildNodes();
						for (int k = 0; k < nl.getLength(); k++) {
							Node pointNode = nl.item(k);
							if (pointNode.getNodeName().equals("point")) {
								PointPoint pp = parseLocation(pointNode, defaultUnits, "x", "y");
								if (pp == null) {
									// alert user
									continue;
								}
								
								pp.modified.x += gridOffset.x - d;
								pp.modified.y += gridOffset.y - d;
								
								pn.addPoint(pp);
							}
						}							
																		
						
					}
				}
				else if (nodeName.equals("nets")) {
					boolean defaultTrackVisible = parseTrackLayout(false, node);
					NodeList netList = node.getChildNodes();
					for (int j = 0; j < netList.getLength(); j++) {
						Node net = netList.item(j);
						if (!net.getNodeName().equals("net")) continue;
						
						boolean netDefaultTrackVisible = parseTrackLayout(defaultTrackVisible, net);
						NodeList connectors = net.getChildNodes();
						ArrayList<PointName> names = new ArrayList<PointName>();
						for (int k = 0; k < connectors.getLength(); k++) {
							Node connector = connectors.item(k);
							if (!connector.getNodeName().equals("connector")) continue;
							
							NamedNodeMap map = connector.getAttributes();
							if (map == null) continue;
														
							Node idNode = map.getNamedItem("id");
							if (idNode == null) continue;
							
							String id = idNode.getNodeValue();
							if (id == null || id == "") continue;
							
							boolean visible = parseTrackLayout(netDefaultTrackVisible, connector);
							
							// stick the ID into the name field
							PointName pn = new PointName(id, visible, null);
							names.add(pn);						
						}
						
						if (names.size() > 0) {
							for (int k = 0; k < names.size() - 1; k++) {
								PointName source = names.get(k);
								PointName target = names.get(k + 1);
								// the name field has the ID 
								trackHash.put(source.name + target.name, source.visible && target.visible);

							}
							
							nets.add(names);
						}

					}
				}
			}
					
						
			return;
		}
		catch (Exception ex) {
			// alert the user
			return;
		}		
	}
		
	protected boolean parseTerminalLabelLayout(boolean defaultValue, Node node) {
		return parseXLayout(defaultValue, node, "nameLayout");
	}
	
	protected boolean parseTrackLayout(boolean defaultValue, Node node) {
		return parseXLayout(defaultValue, node, "trackLayout");
	}

	protected boolean parseXLayout(boolean defaultValue, Node node, String nodeName) {
		try {
			NodeList children = node.getChildNodes();
			if (children == null) return defaultValue;
			
			for (int i = 0; i < children.getLength(); i++) {
				Node child = children.item(i);
				if (!child.getNodeName().equals(nodeName)) continue;
				
				NamedNodeMap map = child.getAttributes();
				if (map != null) {
					Node att = map.getNamedItem("visible");
					if (att != null) {
						String value = att.getNodeValue();
						return !(value.equalsIgnoreCase("false"));
					}
				}
			
				return defaultValue;
			}
		}
		catch (Exception ex) {
		}
		
		return defaultValue;					
	}

		
	protected void parseLayers(NodeList nodeList) {
		for (int i = 0; i < nodeList.getLength(); i++) {
			try {
				Node node = nodeList.item(i);
				NamedNodeMap map = node.getAttributes();
				if (map == null) continue;
				
				Node typeNode = map.getNamedItem("type");
				if (typeNode == null) continue;
				
				String type = typeNode.getNodeValue();
				if (type == null) continue;
				
				// <layer type="image">
				if (type.equalsIgnoreCase("image")) {
					NodeList children = node.getChildNodes();
					for (int j = 0; j < children.getLength(); j++) {
						Node imageNode = children.item(j);
						if (!imageNode.getNodeName().equals("image")) continue;
									
						map = imageNode.getAttributes();
						if (map == null) continue;
						
						typeNode = map.getNamedItem("type");
						if (typeNode == null) continue;
						
						type = typeNode.getNodeValue();
						if (type == null) continue;						

						Node sourceNode = map.getNamedItem("source");
						if (sourceNode == null) continue;
						
						String source = sourceNode.getNodeValue();
						if (source == null) continue;						

						Node zoomNode = map.getNamedItem("zoom");
						if (zoomNode == null) continue;
						
						String zoomString = zoomNode.getNodeValue();
						double zoom = 1;
						if (zoomString != null) 
							zoom = Double.parseDouble(zoomString);
						
						if (type.equalsIgnoreCase("bitmap")) {
							bitmapFilenames.put(zoom, source);
							
							ImageDescriptor desc = ImageDescriptor.createFromURL(
									FileLocator.toFileURL(
											new URL("file", null, getContentsPath() + source)));
							FritzingElementTypes.getImageRegistry().put(
									getContentsPath() + source, desc);
						}
						else if (type.equalsIgnoreCase("svg")) {
							svgFilename = source;
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
		for (int i = 0; i < nodeList.getLength(); i++) {
			try {
				Node node = nodeList.item(i);
				NamedNodeMap map = node.getAttributes();
				if (map == null) continue;
				
				Node sizeNode = map.getNamedItem("size");
				if (sizeNode == null) continue;

				Node sourceNode = map.getNamedItem("source");
				if (sourceNode == null) continue;
				
				String size = sizeNode.getNodeValue();
				if (size == null) continue;
				
				String source = sourceNode.getNodeValue();
				if (source == null) continue;
								
				if (size.equalsIgnoreCase("small")) {
					iconFilename = source;
				}
				else if (size.equalsIgnoreCase("large")) {
					largeIconFilename = source;
				}
			}
			catch (Exception ex) {
				// alert the user
			}
		}
	}
	
	protected void parseFootprints(NodeList nodeList) {
		for (int i = 0; i < nodeList.getLength(); i++) {
			try {
				Node node = nodeList.item(i);
				NamedNodeMap map = node.getAttributes();
				if (map == null) continue;
				
				Node typeNode = map.getNamedItem("type");
				if (typeNode == null) continue;
				
				Node sourceNode = map.getNamedItem("source");
				if (sourceNode == null) continue;
				
				Node defaultNode = map.getNamedItem("default");
				if (defaultNode == null) continue;
				
				String typeValue = typeNode.getNodeValue();
				if (typeValue == null) continue;
				
				String sourceValue = sourceNode.getNodeValue();
				if (sourceValue == null) continue;
				
				String defaultValue = defaultNode.getNodeValue();
				if (defaultValue == null) continue;
						
				if (defaultValue.equalsIgnoreCase("true")) {
					if (typeValue.equalsIgnoreCase("eagle")) {
						footprint = sourceValue;
					}
				}
			}
			catch (Exception ex) {
				// alert the user
			}
		}
	}
	
	protected Double parseDouble(Node node) {
		if (node == null) return 0.0;
		
		String s = node.getNodeValue();
		if (s == null) return 0.0;
		
		if (s == "") return 0.0;
		
		try {
			return Double.parseDouble(s);	
		}
		catch (Exception ex) {
			return 0.0;
		}
	}

	protected PointPoint parseLocation(Node node, String defaultUnits, String axis1, String axis2) {
		if (node == null) {
			return new PointPoint();
		}
		
		try {
			String units = null;
			NamedNodeMap map = node.getAttributes();
			if (map == null) {
				return new PointPoint();
			}
			
			Node ax1Node = map.getNamedItem(axis1);
			Node ax2Node = map.getNamedItem(axis2);
			Node unitsNode = map.getNamedItem("units");
			Double x = parseDouble(ax1Node);
			Double y = parseDouble(ax2Node);
			Point original = new Point(x, y);
			if (unitsNode != null) {
				units = unitsNode.getNodeValue();
			}
					
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
			
			return new PointPoint(original, new Point(x, y));

		}
		catch (Exception ex) {
			// tell the user something's wrong
		}
		
		return new PointPoint();
	}
	
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
	
	static class PointName {
		public ArrayList<PointPoint> points;
		public String name;
		public boolean visible;
		public EObject terminal;
		public String type;
		
		public PointName(String name, boolean visible, String type) {
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

}

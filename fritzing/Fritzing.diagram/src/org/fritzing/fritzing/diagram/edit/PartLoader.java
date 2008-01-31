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
import java.util.Iterator;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.draw2d.geometry.Point;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.emf.ecore.EStructuralFeature;
import org.eclipse.gmf.runtime.draw2d.ui.mapmode.MapModeUtil;
import org.fritzing.fritzing.FritzingFactory;
import org.fritzing.fritzing.FritzingPackage;
import org.fritzing.fritzing.Leg;
import org.fritzing.fritzing.Part;
import org.fritzing.fritzing.Sketch;
import org.fritzing.fritzing.Terminal;
import org.fritzing.fritzing.diagram.edit.parts.Terminal2EditPart;
import org.fritzing.fritzing.diagram.expressions.FritzingAbstractExpression;
import org.fritzing.fritzing.diagram.expressions.FritzingOCLFactory;
import org.fritzing.fritzing.diagram.utils.UIDGenerator;
import org.w3c.dom.Attr;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;
import org.xml.sax.SAXParseException;

/*
 * Loads a part definition from a specified XML, and initializes an element
 * (usually of type Part) with this definition.
 */
public class PartLoader {

	
	public static PartDefinition loadXMLFromLibrary(String root, String path, boolean saveDocument) {
		try {
			URL url = new URL("file", null, root + path);
			File f = new File(root + path);
			PartDefinition pd = new PartDefinition();
			pd.setContentsPath(f.getParent() + File.separator);
			loadXML(pd, url, saveDocument);
			return pd;
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}
		return null;
	}
	
	protected static boolean initialize(String root, String path, EObject newElement) {
		try {
//			URL url = FileLocator.find(FritzingDiagramEditorPlugin
//			.getInstance().getBundle(), new Path(
//			"icons/parts/partdescription.xml"), null);
//			url = FileLocator.toFileURL(url);
			
			PartDefinition pd = loadXMLFromLibrary(root, path, false);
			if (pd == null) return false;
			
			return initialize(pd, newElement);
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}
		return false;
	}
	
				
	public static boolean initialize(PartDefinition pd, EObject newElement) {
		if (!pd.isLoaded()) return false;
		
		//System.out.println("initialize " + newElement + " " + System.currentTimeMillis());
		
		// Metadata
		if (newElement instanceof Part) {
			((Part) newElement).setId(UIDGenerator.getInstance().genUID());
			((Part) newElement).setSpecies(pd.getSpecies());
			((Part) newElement).setGenus(pd.getGenus());
			((Part) newElement).setVersion(pd.getVersion());
			((Part) newElement).setFootprint(pd.getFootprint());
		}
		
		// Terminals
		try {
			
			for (Enumeration<String> e = pd.getTerminalIds(); e.hasMoreElements();) {
				String id = e.nextElement();
				if (id == null || id == "") continue;
								
				PointName pointName = pd.getTerminalData(id);
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
			
			// Nets/Tracks
			if (pd.hasNets()) {
				Iterator<ArrayList<PointName>> i = pd.getNets();
				while (i.hasNext()) {
					ArrayList<PointName> net = i.next();
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
						
						PointName spn = pd.getTerminalData(source.name);
						if (spn == null) {
							// alert user
							continue;							
						}

						PointName tpn = pd.getTerminalData(target.name);
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
						
						feature = FritzingPackage.eINSTANCE.getPart_Tracks();
						((Collection) newElement.eGet(feature)).add(track);											
					}
				}
			}
			
			EStructuralFeature feature = FritzingPackage.eINSTANCE.getIElement_Name();
			FritzingAbstractExpression expr = FritzingOCLFactory
					.getExpression("\'" + pd.getLabel() + "\'",
							newElement.eClass());
			expr.assignTo(feature, newElement);

			//System.out.println("done initialize " + newElement + " " + System.currentTimeMillis());
			return true;
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}
		return false;
	}
	
	protected static boolean loadXML(PartDefinition pd, URL xml, boolean saveDocument) {
	    Document document;
	    DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
	    
	    factory.setIgnoringComments(!saveDocument);
	    factory.setIgnoringElementContentWhitespace(!saveDocument);
	    
	    //factory.setValidating(true);   
	    //factory.setNamespaceAware(true);
        try {
            DocumentBuilder builder = factory.newDocumentBuilder();
            pd.setDocumentFile(new File(xml.getFile()));
            document = builder.parse(pd.getDocumentFile());
            parseXML(pd, document);
            pd.setLoaded(true);
            if (saveDocument) {
            	pd.setDocument(document);
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
	
	
	protected static void parseXML(PartDefinition pd, Document document) {

		try {
			Element partNode = document.getDocumentElement();
			if (!partNode.getNodeName().equals("part")) {
				// alert user
				return;								
			}
			
			pd.setGeneric(true);
			Attr attr = partNode.getAttributeNode("generic");
			if (attr != null) {
				String s = attr.getNodeValue();
				if (s != null) {
					pd.setGeneric(!s.equalsIgnoreCase("false"));
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
					pd.setGridOffset(parseLocation(node, defaultUnits, "x", "y").modified);
				}
			}
			if (defaultUnits == null || defaultUnits == "") {
				defaultUnits = "pixels";
			}
					
			for (int i = 0; i < nodes.getLength(); i++) {
				Node node = nodes.item(i);
				String nodeName = node.getNodeName();
				if (nodeName.equals("species")) {
					pd.setSpecies(node.getTextContent());
				}
				else if (nodeName.equals("genus")) {
					pd.setGenus(node.getTextContent());				
				}
				else if (nodeName.equals("description")) {
					pd.setDescription(node.getTextContent());				
				}
				else if (nodeName.equals("title")) {
					pd.setTitle(node.getTextContent());					
				}
				else if (nodeName.equals("label")) {
					pd.setLabel(node.getTextContent());					
				}
				else if (nodeName.equals("version")) {
					pd.setVersion(node.getTextContent());					
				}
				else if (nodeName.equals("footprints")) {
					parseFootprints(pd, node.getChildNodes());					
				}
				else if (nodeName.equals("icons")) {
					parseIcons(pd, node.getChildNodes());		
				}
				else if (nodeName.equals("layers")) {
					String s = ((Element) node).getAttribute("zorder");
					if (s != null && s.length() > 0) {
						pd.setZOrder(s);
					}					
					parseLayers(pd, node.getChildNodes());
				}
				else if (nodeName.equals("bounds")) {
					Point sz = parseLocation(node, defaultUnits, "width", "height").modified;					
					pd.setSize(new Dimension(sz.x, sz.y));
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
						pd.addTerminal(id, pn);

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
								
								pp.modified.x += pd.getGridOffset().x - d;
								pp.modified.y += pd.getGridOffset().y - d;
								
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
								pd.setTrackVisible(source.name + target.name, source.visible && target.visible);

							}
							
							pd.addNet(names);
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
		
	protected static boolean parseTerminalLabelLayout(boolean defaultValue, Node node) {
		return parseXLayout(defaultValue, node, "nameLayout");
	}
	
	protected static boolean parseTrackLayout(boolean defaultValue, Node node) {
		return parseXLayout(defaultValue, node, "trackLayout");
	}

	protected static boolean parseXLayout(boolean defaultValue, Node node, String nodeName) {
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

		
	protected static void parseLayers(PartDefinition pd, NodeList nodeList) {
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
							pd.addBitmapFilename(zoom, source);
						}
						else if (type.equalsIgnoreCase("svg")) {
							pd.setSvgFilename(source);
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
	
	protected static void parseIcons(PartDefinition pd, NodeList nodeList) {
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
					pd.setIconFilename(source);
				}
				else if (size.equalsIgnoreCase("large")) {
					pd.setLargeIconFilename(source);
				}
			}
			catch (Exception ex) {
				// alert the user
			}
		}
	}
	
	protected static void parseFootprints(PartDefinition pd, NodeList nodeList) {
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
						pd.setFootprint(sourceValue);
					}
				}
			}
			catch (Exception ex) {
				// alert the user
			}
		}
	}
	
	protected static Double parseDouble(Node node) {
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

	protected static PointPoint parseLocation(Node node, String defaultUnits, String axis1, String axis2) {
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

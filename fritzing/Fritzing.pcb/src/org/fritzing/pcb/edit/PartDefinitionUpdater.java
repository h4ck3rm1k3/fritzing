package org.fritzing.pcb.edit;

import java.io.File;

import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.w3c.dom.Attr;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

public class PartDefinitionUpdater {

	static int filesLoaded;

	/**
	 * @param args
	 */
	public static void main(String[] args) {

		filesLoaded = 0;

		if (args.length < 1) {
			System.out
					.println("first argument should be parent of libraries folder");
			return;
		}

		String root = args[0];
		if (!root.endsWith(File.separator)) {
			root += File.separator;
		}

		changeXMLs(root, "libraries");

		System.out.println("files loaded = " + filesLoaded);
	}

	public static void changeXMLs(String root, String folder) {

		File file = new File(root + folder);
		if (!file.exists())
			return;
		if (!file.isDirectory())
			return;

		File xmlFile = new File(file.getAbsolutePath() + File.separator
				+ "partdescription.xml");
		if (xmlFile.exists()) {
			System.out.println("updating " + xmlFile.getAbsolutePath());
			changeXML(root, folder);
		}

		File[] files = file.listFiles();
		for (int i = 0; i < files.length; i++) {
			String filename = files[i].getName();
			changeXMLs(root, folder + File.separator + filename);
		}

	}

	public static void changeXML(String root, String prefix) {
		PartDefinition partDefinition = PartDefinitionRegistry.getInstance().get(root,
				prefix + File.separator + "partdescription.xml", true);
		if (partDefinition == null) {
			return;
		}

		try {
			// load doc
			Document doc = partDefinition.getDocument();

			// change the xml
			update(doc);

			File docFile = partDefinition.getDocumentFile();
			partDefinition.getDocumentFile().renameTo(new File(docFile.getParent() + File.separator + "old" + docFile.getName()));

			// save doc
			Transformer transformer = TransformerFactory.newInstance()
					.newTransformer();
			transformer.setOutputProperty(OutputKeys.INDENT, "yes");

			StreamResult result = new StreamResult(docFile);
			DOMSource source = new DOMSource(doc);
			transformer.transform(source, result);
			filesLoaded++;

		} catch (Exception ex) {
			ex.printStackTrace();
		}

	}

	public static void addPoint(Document document, Node node, String x, String y) {
		Node point = document.createElement("point");
		Attr attr = document.createAttribute("x");
		attr.setNodeValue(x);
		point.getAttributes().setNamedItem(attr);
		attr = document.createAttribute("y");
		attr.setNodeValue(y);
		point.getAttributes().setNamedItem(attr);
		node.appendChild(point);
	}

	public static String getAttributeValue(NamedNodeMap map, String name) {
		Node attr = map.getNamedItem(name);
		if (attr == null)
			return null;

		String val = attr.getNodeValue();
		return val;

	}

	public static void update(Document document) {
		Element partNode = document.getDocumentElement();
		if (!partNode.getNodeName().equals("part")) {
			// alert user
			return;
		}

		NodeList nodes = partNode.getChildNodes();
		for (int i = 0; i < nodes.getLength(); i++) {
			Node node = nodes.item(i);
			String nodeName = node.getNodeName();
			if (nodeName.equals("connectors")) {
				NodeList connectors = node.getChildNodes();
				for (int j = 0; j < connectors.getLength(); j++) {
					Node connector = connectors.item(j);
					if (!connector.getNodeName().equals("connector"))
						continue;

					NamedNodeMap map = connector.getAttributes();
					String type = getAttributeValue(map, "type");
					if (type == null)
						continue;

					String x = getAttributeValue(map, "x");
					if (x == null)
						continue;

					String y = getAttributeValue(map, "y");
					if (y == null)
						continue;

					map.removeNamedItem("x");
					map.removeNamedItem("y");

					addPoint(document, connector, x, y);

					if (type.equals("leg")) {
						NodeList nl = connector.getChildNodes();
						for (int k = 0; k < nl.getLength(); k++) {
							Node leg = nl.item(k);
							if (leg.getNodeName().equals("leg")) {
								map = leg.getAttributes();

								x = getAttributeValue(map, "dx");
								if (x == null)
									continue;

								y = getAttributeValue(map, "dy");
								if (y == null)
									continue;

								addPoint(document, connector, x, y);

								connector.removeChild(leg);
								break;
							}
						}
					}
				}
			}
		}
	}

}

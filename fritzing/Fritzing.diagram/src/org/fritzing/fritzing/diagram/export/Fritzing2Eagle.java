package org.fritzing.fritzing.diagram.export;

import java.util.ArrayList;
import java.util.List;

import org.eclipse.emf.common.util.EList;
import org.eclipse.gef.EditPart;
import org.eclipse.gmf.runtime.diagram.ui.editparts.ShapeEditPart;
import org.eclipse.gmf.runtime.diagram.ui.parts.IDiagramGraphicalViewer;
import org.eclipse.gmf.runtime.emf.core.util.EMFCoreUtil;
import org.eclipse.gmf.runtime.notation.View;
import org.fritzing.fritzing.Element;
import org.fritzing.fritzing.ILegConnection;
import org.fritzing.fritzing.Leg;
import org.fritzing.fritzing.Part;
import org.fritzing.fritzing.Sketch;
import org.fritzing.fritzing.Terminal;
import org.fritzing.fritzing.Track;
import org.fritzing.fritzing.Wire;
import org.fritzing.fritzing.diagram.edit.parts.SketchEditPart;

public class Fritzing2Eagle {		
	public static String createEagleScript(IDiagramGraphicalViewer viewer) {
		String result = "";
		SketchEditPart sketchEP = (SketchEditPart) viewer.getContents();
		Sketch sketch = (Sketch) ((View) sketchEP.getModel()).getElement();
		
		/* array lists partList and netList hold entries for parts and connections
		 * respectively and together fully describe the Fritzing sketch */
		ArrayList<EagleBRDPart> partList = new ArrayList<EagleBRDPart>();
		ArrayList<EagleBRDNet> netList = new ArrayList<EagleBRDNet>(); 
		
		/* we begin the conversion process by first creating an entry in partList for
		 * each part in the Fritzing sketch.  pass the Fritzing part coordinates here
		 * since we need the viewer to grab layout information about the model. 
		 */
		int genericPart = 1;
		for (Part p: sketch.getParts()) {	
			EagleBRDPart part = new EagleBRDPart(p);
			part.setFritzingPartPos(new CoordPair(		// Fritzing coordinates
					((float)getLayoutInfo(viewer, p).getLocation().x), 
					(float)getLayoutInfo(viewer, p).getLocation().y));
			if (p.getName() == null) {
				part.setEagleLabelPrefix("part" + genericPart);
				genericPart++;
			} else {
				part.setEagleLabelPrefix(p.getName());
			}
			partList.add(part);
		}
		

		/* once the partlist is fully populated, we check for duplicate names of parts in Fritzing
		 * and correct by adding an integer suffix (form is: "R_3").  */
		
		for (int i=0; i<partList.size(); i++) {
			if (dupeNamesExistForPart((EagleBRDPart)partList.get(i), partList)) {
				enumerateDupeParts((EagleBRDPart)partList.get(i), partList);
			}
		}
		
		/* convert Fritzing part coordinates to Eagle part coordinates and do any 
		 * necessary adjustments
		 */		
		for (int i=0; i<partList.size(); i++) {
			EagleBRDPart part = partList.get(i);
			CoordPair fritzingPartPos = part.getFritzingPartPos();
			CoordPair eaglePartPos = new CoordPair(
					(float)((fritzingPartPos.xVal/1000) / 2.54), 
					(float)((fritzingPartPos.yVal/1000) / 2.54));
			part.setEaglePartPos(eaglePartPos);
			
			float xPos = part.getEaglePartPos().xVal;
			float yPos = part.getEaglePartPos().yVal;
			float yLimit = (float)3.2;			
			part.setEaglePartPos(new CoordPair(xPos, (float)(yLimit - yPos)));
			
			if (part.getFritzingSpecies().equalsIgnoreCase("ArduinoDiecimila"))  {
				part.setEaglePartPos(new CoordPair((float)0, (float)0));
				part.setPartPosLock(true);
			}
		}
		
		/* print the part information for debugging purposes */
//		for (int i=0; i<partList.size(); i++) {
//			EagleBRDPart part = partList.get(i);
//			if (part.getExportToPcb()) {
//				System.out.println(">>> (" + 
//					part.getFritzingId() + ") " +
//					part.getEaglePartLabel() + " - " + 
//					part.getEagleFootprint() + " in " + 
//					part.getEagleLibraryName() + " " + 
//					"(" + part.getEaglePartPos().xVal + " " + part.getEaglePartPos().yVal + ")");
//			} else {
//				System.out.println(">>> (" +
//					part.getFritzingId() + ") " + 
//					part.getEaglePartLabel() + " - " + 
//					"NOT EXPORTED TO PCB");
//			}
//		}
		
		/* moving on to wires and nets: 
		 * basic approach is to first create a new entry in the array list "netList" for each
		 * wire in the object model.
		 */
		for (Wire w: sketch.getWires()) {
			if ((w.getSource() == null) && (w.getTarget() == null)) {
				continue;
			}
			// have to pass the partList to the EagleBRDNet constructor so there is a reference
			// to parts named differently between Fritzing and Eagle (with same-named Fritzing
			// parts that have been enumerated, for instance
			EagleBRDNet net = new EagleBRDNet(w, partList);
			netList.add(net);
		}
		
		/* add a netlist entry for any leg in any part that is directly connected to a terminal.
		 * this would be a leg that has been dragged to another terminal without an intervening 
		 * wire or track.  
		 */
		for (int i=0; i<partList.size(); i++) {			
			EagleBRDPart part = partList.get(i);			
			EList <Terminal> terminals = part.p.getTerminals();			
			for (int j=0; j<terminals.size(); j++) {				
				if (terminals.get(j).getLeg() != null) {
					/*
					if (terminals.get(j).getLeg().getTarget().getClass().getSimpleName().equals("TerminalImpl")) {
						EagleBRDNet net = new EagleBRDNet(terminals.get(j).getLeg(), partList);
						netList.add(net);
					}
					*/
					Part targetPart = null;
					Leg leg = terminals.get(j).getLeg();
					ILegConnection target = leg.getTarget();
					System.out.println("target string: " + target.toString());
					if (target != null) {
						if (target instanceof Terminal) {
							targetPart = ((Terminal)target).getParent();
						} else if (target instanceof Leg) {
							Terminal targetSource = (Terminal)((Leg)target).getSource();
							targetPart = targetSource.getParent();
						} else if (target instanceof Sketch) {
							System.out.println("!!! sketch part !!!");
							continue;
						}
					} else {
						System.out.println("!!!! null target !!!!");
					}
					
					EagleBRDPart eagleSourcePart = part;
					
					EagleBRDPart eagleTargetPart = null;
					for (int k=0; k<partList.size(); k++) {
						if (targetPart.getId().equals(partList.get(k).getFritzingId())) {
							eagleTargetPart = partList.get(k);
						}
					}
					System.out.println("source: " + eagleSourcePart.getEaglePartLabel() + "." + leg.getSource().getId());
					System.out.println("target: " + eagleTargetPart.getEaglePartLabel() + "." + leg.getTarget().getId());
					
					String eagleSourcePinId = leg.getSource().getId();
					String eagleTargetPinId = leg.getTarget().getId();
					
					if (eagleTargetPinId == null) {
						// indicates that the leg is probably directly connected to another leg
						if (leg.getTarget() instanceof Leg) {
							ILegConnection targetTest = leg.getTarget();
							System.out.println("source part pin id: " + ((Terminal)((Leg)targetTest).getSource()).getId());
//							System.out.println("target part pin id: " + ((Terminal)((Leg)targetTest).getTarget()).getId());
							eagleTargetPinId = ((Terminal)((Leg)targetTest).getSource()).getId();
						}
					}
					
					PartPinPair sourcePartPin = new PartPinPair(eagleSourcePart, eagleSourcePinId);
					PartPinPair targetPartPin = new PartPinPair(eagleTargetPart, eagleTargetPinId);
					EagleBRDNet net = new EagleBRDNet(sourcePartPin, targetPartPin);
					netList.add(net);
 				}				
			}			
		}
		
		/* add in the breadboard tracks */
		for (int i=0; i<partList.size(); i++) {
			EagleBRDPart part = partList.get(i);
//			if (part.getFritzingSpecies().equalsIgnoreCase("breadboardstandard")) {
				EList <Track> tracks = part.getFritzingTracks();
				for (int j=0; j<tracks.size(); j++) {
					Track t = tracks.get(j);
					if ((t.getSource() == null) && (t.getTarget() == null)) {
						continue;
					}
					EagleBRDNet net = new EagleBRDNet(t, partList);
					netList.add(net);
				}
//			}			
		}
		
		/* now enumerate the new net names.  we are ignoring the names provided by Fritzing
		 * for the moment as an experiment to see which method causes less confusion for users
		 */
		int genericNet = 1;
		for (int i=0; i<netList.size(); i++) {
			netList.get(i).setNetName("N$" + genericNet);
			genericNet++;
		}
		
		/* combine the entries for those nets which are meant to be connected */
		for (int i=0; i<netList.size(); i++) {
			EagleBRDNet netOne = netList.get(i);
			for (int j=0; j<netOne.getPinList().size(); j++) {
				PartPinPair pin = netOne.getPin(j);
				for (int k=i+1; k<netList.size(); k++) {
					EagleBRDNet netTwo = netList.get(k);
					if (netList.get(k).pinIsPresent(pin)) {
//						add all pins from the second net entry to the first entry
//						add the wire from the second net entry to the first entry
						netOne.addPinList(netTwo.getPinList());						
//						remove the second net entry from netList
						netList.remove(k);
						continue;
					}
				}
			}
		}
			
		/* scrub each net entry for duplicate pin entries */
		for (int i=0; i<netList.size(); i++) {
			netList.get(i).removeDuplicatePinEntries();
		}
		
		
		/* print the net information for debugging purposes */
		for (int i=0; i<netList.size(); i++) {
//			System.out.println("&&& " + netList.get(i).getNetName() + " " + netList.get(i).getPinListAsString());
		}
		
		BRDScriptExporter exporter = new BRDScriptExporter();
		result = exporter.export(partList, netList);
//		System.out.println(result);
		
		return result;
	}
	
	@SuppressWarnings("unchecked")
	private static ShapeEditPart getLayoutInfo(IDiagramGraphicalViewer viewer, Element e) {
		List<EditPart> editParts = viewer.findEditPartsForElement(
				EMFCoreUtil.getProxyID(e), ShapeEditPart.class);
		return (ShapeEditPart) editParts.get(0);
	}
	
	public static boolean dupeNamesExistForPart(EagleBRDPart part, ArrayList<EagleBRDPart> partList) {
		boolean result = false;
		String id = part.getFritzingId();
		String label = part.getFritzingLabel();
		int labelSuffix = part.getEagleLabelSuffix();
		for (int i=0; i<partList.size(); i++) {
			String tempId = ((EagleBRDPart)partList.get(i)).getFritzingId();
			if (id.equals(tempId)) {
				continue;
			}
			String tempLabel = ((EagleBRDPart)partList.get(i)).getFritzingLabel();

			if (label.equals(tempLabel) && labelSuffix == 0) {
				result = true;
			}
		}
		return result;
	}
	
	public static void enumerateDupeParts(EagleBRDPart part, ArrayList<EagleBRDPart> partList) {
		int n = 1;
		String label = part.getFritzingLabel();
		for (int i=0; i<partList.size(); i++) {
			String tempLabel = ((EagleBRDPart)partList.get(i)).getFritzingLabel();
			if (label.equals(tempLabel)) {
				((EagleBRDPart)partList.get(i)).setEagleLabelSuffix(n);
				n++;
			}
		}
	}
}

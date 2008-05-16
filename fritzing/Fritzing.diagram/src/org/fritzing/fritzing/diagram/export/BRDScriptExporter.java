package org.fritzing.fritzing.diagram.export;

import java.io.File;
import java.util.ArrayList;

import org.fritzing.fritzing.diagram.part.FritzingDiagramEditorPlugin;
import org.fritzing.fritzing.diagram.part.FritzingDiagramEditorUtil;
import org.fritzing.fritzing.diagram.preferences.EaglePreferencePage;


public class BRDScriptExporter {	
	// the following header text string is prepended to all schematic-
	// generating script files we produce - it sets up the schematic 
	// editing environment, opens a schematic sheet, and generally gets
	// things all set up
	private String headerText = "GRID INCH 0.005 \n" + // finer grid to enable exact placement of wires
		// the DISPLAY command chooses the visible layers.
		// only visible layers can be selected, connected to, or 
		// otherwise acted on.
		// layers must first be entered into the layer setup 
		// (here accomplished with 'LAYER <number> <name>')
		// first, turn on all needed schematic layers in case they are 
		// not already available.
//		"LAYER 91 Nets; \n" + 	
//		"LAYER 92 Busses; \n" + 
//		"LAYER 93 Pins; \n" + 
//		"LAYER 94 Symbols; \n" + 
//		"LAYER 95 Names; \n" + 
//		"LAYER 96 Values; \n" + 
		// act on the layers already entered in the layer setup AND
		// turn OFF visibility of the PINS layer
//		"DISPLAY -PINS \n" +	 
//		" \n" + 
		// load the drawing to be acted on - here schematic sheet 1 (.S1)
//		"EDIT .S1 \n" +
		// load the board drawing to be acted on (B1)
//		"EDIT .B1 \n" + 
		// set the bend angle for wires in the schematic sheet 
		// bend angle 2 used here produces straight-line connections at
		// arbitrary angles
//		"SET WIRE_BEND 2; \n" + 
		// set the wire style visual appearance - we want continuous lines
//		"CHANGE STYLE 'Continuous' \n" + 
		// tell the schematic editor to use the Fritzing Eagle library
		// it is not loaded by default
	/*TODO update "USE" directive with a list of the actual packages used	 */
//		"USE '" + FritzingDiagramEditorUtil.getFritzingLocation() +
//				"eagle/lbr/fritzing.lbr';" + 
		"\n";
	
	// the following footer text is appended to all schematic-generating
	// script files we produce.  these commands are used primarily to return
	// the Eagle environment to its default state
	// the GRID LAST instruction tells Eagle to return the grid units
	// to their previous state.  simple.
	private String footerText = "GRID LAST; \n";
	
	// getHeaderText() and getFooterText() return the header and footer
	// strings respectively
	public String getHeaderText() {
		return headerText;		
	}
	
	public String getLibraryEntries(ArrayList <String> eagleLibs) {
		String result = "";
		
		return result;
	}
	
	public String getFooterText() {
		return footerText;
	}
	
	public String getPartEntry(EagleBRDPart part) {
		String result = "";
		// place a schematic symbol 
		// Eagle syntax is "ADD <libraryPart>@<libraryName> 'partName' 'gateName' R<rotationAngle> (<componentCoords>)"
		/* Look for the specified part library in three different places:
		 * 1. in the part's folder
		 * 2. in fritzing/eagle/lbr
		 * 3. in the eagle libraries folder
		 */
		String library = part.getPartLocation() + part.getEagleLibraryName() + ".lbr";
		if (! new File(library).exists()) {
			library = FritzingDiagramEditorUtil.getFritzingLocation()
				+ "eagle" + File.separator + "lbr" +  File.separator + part.getEagleLibraryName() + ".lbr"; 
			if (! new File(library).exists()) {
				library = FritzingDiagramEditorPlugin.getInstance()
					.getPluginPreferences().getString(EaglePreferencePage.EAGLE_LOCATION)
					+ File.separator + "lbr" + File.separator + part.getEagleLibraryName() + ".lbr";
				if (! new File(library).exists()) {
					System.out.println("Could not find library " + part.getEagleLibraryName());
				}
			}
		}
		result = "ADD '" + 
			part.getEagleFootprint() + 
			"@" + library + "' " +
			"'" + part.getEaglePartLabel() + "' " +
			part.getEagleRotationPrefix() + part.getEagleRotationVal() + " " + 
			"(" + part.getEaglePartPos().xVal + " " + part.getEaglePartPos().yVal + "); \n";
		return result;
	}
	
	public String getNetEntry(EagleBRDNet net) {
		/* place a SIGNAL to connect terminals*/
		String result = "";
		String netName = net.getNetName();
		String pinListing = "";
		for (int i=0; i<net.getPinList().size(); i++) {
			if (net.getPin(i).getEagleBRDPart().getExportToPcb() == true) {
				pinListing = pinListing.concat(
					"'" + net.getPin(i).getEagleBRDPart().getEaglePartLabel() + "' " +
					"'" + net.getPin(i).getPinName() + "' ");
			}
		}
		
		if (pinListing.equals("") == false) {
			result = "SIGNAL '" + netName + "' " + pinListing + "; \n";
		}
//		String result = "SIGNAL '" + net.netName + "' " + 
//			"'" + ((Terminal)net.source).getParent().getName() + "' " + 
//			"'" + net.source.getName() + "' " +
//			"'" + ((Terminal)net.target).getParent().getName() + "' " + 
//			"'" + net.target.getName() + "'; \n";
//			"...;";
		return result;
	}
	
	public String export(ArrayList<EagleBRDPart> partList, ArrayList<EagleBRDNet> netList) {
		// this should take a file handle and the pre-populated part and net listArrays as arguments
		// iterate through each entry, printing corresponding lines plus header and footer text
		String result = "";
		result = result.concat(getHeaderText());
		
		/*
		ArrayList <String> eagleLibsList = new ArrayList();
		for (int i=0; i<partList.size(); i++) {
			String libName = ((EagleBRDPart)partList.get(i)).getEagleLibraryName();
			String libLocation = ((EagleBRDPart)partList.get(i)).getLibraryLocation();
			if (libName.equals("fritzing") == false) {
				for (int j=0; j<eagleLibsList.size(); j++) {
					if (libLocation.equals((String)eagleLibsList.get(j)) == false) {
						eagleLibsList.add(libLocation + ".lbr");
					}
				}
			}
		}
		for (int i=0; i<eagleLibsList.size(); i++) {
			result = result.concat("USE '" + (String)eagleLibsList.get(i) + "'; \n");
		}*/
		
		for (int i=0; i<partList.size(); i++) {
			if (partList.get(i).getExportToPcb() == true) {
				result = result.concat(getPartEntry((EagleBRDPart)partList.get(i)));
			}			
		}
		
		for (int i=0; i<netList.size(); i++) {
			result = result.concat(getNetEntry((EagleBRDNet)netList.get(i)));
		}
		
		result = result.concat(getFooterText());
		
		return result;
	}
}

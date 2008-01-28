package org.fritzing.fritzing.diagram.export;

import java.util.ArrayList;

import org.eclipse.emf.common.util.EList;
import org.fritzing.fritzing.Leg;
import org.fritzing.fritzing.Part;
import org.fritzing.fritzing.Terminal;
import org.fritzing.fritzing.Track;


public class EagleBRDPart {
	Part p;
	private String eagleFootprint = "";
	private String eagleLibraryName = "";
	private String eaglePartLabelPrefix = "";
	private int eaglePartLabelSuffix = 0;			// used to correct same-named parts 
	private String eagleRotationPrefix = "R";
	private String eagleRotationVal = "0.000";
	private CoordPair fritzingPartPos = new CoordPair();
	private CoordPair eaglePartPos = new CoordPair();
	private boolean locked = false;					// should the part's position be locked?
	private boolean exportToPcb = true;
	
	public EagleBRDPart(Part p) {
		this.p = p;
		
		if (p.getFootprint() != null) {
			String footprintStrings[] = p.getFootprint().split("/");	
			String libraryName = footprintStrings[0].split(".lbr")[0];
			String footprintName = footprintStrings[1];
			
			this.eagleFootprint = footprintName.toUpperCase();
			this.eagleLibraryName = libraryName;
			
			setExportToPcb(true);
		} else {
			setExportToPcb(false);
		}
	}
	
	public EagleBRDPart() {
		
	}
	
	public String getFritzingId() {
		return(p.getId());
	}
	
	public String getFritzingLabel() {
		return(p.getName());
	}
	
	public String getFritzingGenus() {
		return(p.getGenus());
	}
	
	public String getFritzingSpecies() {
		return(p.getSpecies());
	}
	
	public EList<Track> getFritzingTracks() {
		return(p.getTracks());
	}

	/*
	public EList<Leg> getLegs() {
		EList <Terminal> terminals = p.getTerminals();
		EList <Leg> result = new EList() <Leg>;
//		System.out.println(p.getName() + ":");
		for (int i=0; i<terminals.size(); i++) {
			System.out.println("terminal class: " + terminals.get(i).getClass().getSimpleName());
			if (terminals.get(i).getLeg() != null) {
				System.out.println("leg source " + terminals.get(i).getLeg().getSource());
				System.out.println("leg target " + terminals.get(i).getLeg().getTarget());
				System.out.println("leg class " + terminals.get(i).getLeg().getClass());
				System.out.println("leg source name " + terminals.get(i).getLeg().getSource().getName());
				System.out.println("leg target name " + terminals.get(i).getLeg().getTarget().getName());
				
			}			
		}		
		return result;
	}
	*/
	
	public String getEagleFootprint() {
		return(this.eagleFootprint);
	}
	
	public String getEagleLibraryName() {
		return(this.eagleLibraryName);
	}
	
	public String getEagleLabelPrefix() {
		return(this.eaglePartLabelPrefix);
	}
	
	public void setEagleLabelPrefix(String newPrefix) {
		this.eaglePartLabelPrefix = newPrefix;
	}
	
	public int getEagleLabelSuffix() {
		return(this.eaglePartLabelSuffix);
	}
	
	public void setEagleLabelSuffix(int newSuffix) {
		this.eaglePartLabelSuffix = newSuffix;
	}
	
	public String getEaglePartLabel() {
		String eaglePartLabel = "";
		if (eaglePartLabelSuffix != 0) {
			eaglePartLabel = this.eaglePartLabelPrefix + "_" + this.eaglePartLabelSuffix;
		} else {
			eaglePartLabel = this.eaglePartLabelPrefix;
		}
		return eaglePartLabel;
	}
	
	public String getEagleRotationPrefix() {
		return(this.eagleRotationPrefix);
	}
	
	public String getEagleRotationVal() {
		return(this.eagleRotationVal);
	}
	
	public void setFritzingPartPos(CoordPair fritzingPartPos) {
		this.fritzingPartPos = fritzingPartPos;
	}
	
	public CoordPair getFritzingPartPos() {
		return(fritzingPartPos);
	}
	
	public void setEaglePartPos(CoordPair eaglePartPos) {
		this.eaglePartPos = eaglePartPos;
	}
	
	public CoordPair getEaglePartPos() {
		return(eaglePartPos);
	}
	
	public void setPartPosLock(boolean lockOrNot) {
		this.locked = lockOrNot;
	}
	
	public boolean getPartPosLock() {
		return this.locked;
	}
	
	public void setExportToPcb(boolean exportOrNot) {
		this.exportToPcb = exportOrNot;
	}
	
	public boolean getExportToPcb() {
		return this.exportToPcb;
	}
}

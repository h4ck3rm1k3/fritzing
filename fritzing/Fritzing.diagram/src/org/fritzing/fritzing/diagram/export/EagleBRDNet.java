package org.fritzing.fritzing.diagram.export;

import java.util.ArrayList;

import org.fritzing.fritzing.Leg;
import org.fritzing.fritzing.Terminal;
import org.fritzing.fritzing.Wire;

public class EagleBRDNet {
	String netName = "";	
	ArrayList<Wire> wireList = new ArrayList<Wire>();
	ArrayList<PartPinPair> pinList = new ArrayList<PartPinPair>();
	
	public EagleBRDNet(Wire w, ArrayList<EagleBRDPart> partList) {
		wireList.add(w);
		String sourceClass = w.getSource().getClass().getSimpleName();
		String targetClass = w.getTarget().getClass().getSimpleName();
		String sourcePartName = "";
		String sourcePinName = "";
		String targetPartName = "";
		String targetPinName = "";
		
		if (sourceClass.equals("LegImpl")) {					
//			sourcePartName = ((Leg)w.getSource()).getParent().getParent().getName();
			String sourcePartId = ((Leg)w.getSource()).getParent().getParent().getId();
			for (int i=0; i<partList.size(); i++) {
				if (partList.get(i).getFritzingId().equals(sourcePartId)) {
					sourcePartName = partList.get(i).getEaglePartLabel();
				}
			}
			sourcePinName = ((Leg)w.getSource()).getParent().getName();
		}
		if (sourceClass.equals("TerminalImpl")) {
//			sourcePartName = ((Terminal)w.getSource()).getParent().getName();
			String sourcePartId = ((Terminal)w.getSource()).getParent().getId();
			for (int i=0; i<partList.size(); i++) {
				if (partList.get(i).getFritzingId().equals(sourcePartId)) {
					sourcePartName = partList.get(i).getEaglePartLabel();
				}
			}
			sourcePinName = w.getSource().getName();
		}
		if (targetClass.equals("LegImpl")) {
//			targetPartName = ((Leg)w.getTarget()).getParent().getParent().getName();
			String targetPartId = ((Leg)w.getTarget()).getParent().getParent().getId();
			for (int i=0; i<partList.size(); i++) {
				if (partList.get(i).getFritzingId().equals(targetPartId)) {
					targetPartName = partList.get(i).getEaglePartLabel();
				}
			}
			targetPinName = ((Leg)w.getTarget()).getParent().getName();
		}
		if (targetClass.equals("TerminalImpl")) {
//			targetPartName = ((Terminal)w.getTarget()).getParent().getName();
			String targetPartId = ((Terminal)w.getTarget()).getParent().getId();
			for (int i=0; i<partList.size(); i++) {
				if (partList.get(i).getFritzingId().equals(targetPartId)) {
					targetPartName = partList.get(i).getEaglePartLabel();
				}
			}
			targetPinName = w.getTarget().getName();
		}
		
		PartPinPair sourcePartAndPin = new PartPinPair(sourcePartName, sourcePinName);
		PartPinPair targetPartAndPin = new PartPinPair(targetPartName, targetPinName);
		
		addPin(sourcePartAndPin);
		addPin(targetPartAndPin);
	}
	
	public void setNetName(String netName) {
		this.netName = netName;
	}
	
	public String getNetName() {
		return this.netName;
	}
	
	public void addPin(PartPinPair pin) {
		pinList.add(pin);
	}
	
	public void addPinList(ArrayList<PartPinPair> newPinList) {
		for (int i=0; i<newPinList.size(); i++) {
			this.pinList.add(newPinList.get(i));
		}
	}
	
	public PartPinPair getPin(int i) {
		return(this.pinList.get(i));
	}
	
	public ArrayList<PartPinPair> getPinList() {
		return pinList;
	}
	
	public String getPinListAsString() {
		String result = "";
		for (int i=0; i<pinList.size(); i++) {
			result = result.concat(" " + pinList.get(i).getPartName() + "." + pinList.get(i).getPinName());
		}
		return result;
	}
	
	public Wire getWire(int i) {
		return(this.wireList.get(i));
	}
	
	public void addWire(Wire w) {
		this.wireList.add(w);
	}
	
	public boolean pinIsPresent(PartPinPair pin) {
		boolean result = false;
		for (int i=0; i<pinList.size(); i++) {
			if (pin.equals((PartPinPair)pinList.get(i))) {
				result = true;
			}
		}
		return result;
	}
}

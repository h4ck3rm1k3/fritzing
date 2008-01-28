package org.fritzing.fritzing.diagram.export;

import java.util.ArrayList;

import org.fritzing.fritzing.Leg;
import org.fritzing.fritzing.Terminal;
import org.fritzing.fritzing.Track;
import org.fritzing.fritzing.Wire;

public class EagleBRDNet {
	String netName = "";	
	ArrayList<Wire> wireList = new ArrayList<Wire>();
	ArrayList<PartPinPair> pinList = new ArrayList<PartPinPair>();
	
	public EagleBRDNet(Leg l, ArrayList<EagleBRDPart> partList) {
		System.out.println("leg parent name: " + l.getParent().getName());
		
		System.out.println("leg source: " + l.getSource());
		System.out.println("leg source name: " + l.getSource().getName());
		System.out.println("leg target: " + l.getTarget());
		System.out.println("leg target name: " + l.getTarget().getName());
		
		/*
		EagleBRDPart sourcePart = new EagleBRDPart();
		EagleBRDPart targetPart = new EagleBRDPart();		
		
		String sourcePartId = l.getParent().getId();
		String targetPartId = l.getTarget().getId();
		
		sourcePart = (EagleBRDPart)l.getSource();
		targetPart = (EagleBRDPart)l.getTarget();
		
		if (sourcePart.equals(targetPart)) {
			return;
		}
		String sourcePinId = l.getSource().getId();
		String targetPinId = l.getTarget().getId();
		
		addPin(new PartPinPair(sourcePart, sourcePinId));
		addPin(new PartPinPair(targetPart, targetPinId));		
		*/
	}
	
	public EagleBRDNet(Track t, ArrayList<EagleBRDPart> partList) {
		String parentPartId = t.getParent().getId();
		EagleBRDPart parentPart = new EagleBRDPart();
		for (int i=0; i<partList.size(); i++) {
			if (partList.get(i).getFritzingId().equals(parentPartId)) {
				parentPart = partList.get(i);
			}
		}
		
		String sourcePinName = t.getSource().getId();
		String targetPinName = t.getTarget().getId();
		
		PartPinPair sourcePartAndPin = new PartPinPair(parentPart, sourcePinName);
		PartPinPair targetPartAndPin = new PartPinPair(parentPart, targetPinName);
		
		addPin(sourcePartAndPin);
		addPin(targetPartAndPin);
	}
	
	public EagleBRDNet(Wire w, ArrayList<EagleBRDPart> partList) {
		wireList.add(w);
		String sourceClass = w.getSource().getClass().getSimpleName();
		String targetClass = w.getTarget().getClass().getSimpleName();
		String sourcePinName = "";
		String targetPinName = "";
		EagleBRDPart sourcePart = new EagleBRDPart();
		EagleBRDPart targetPart = new EagleBRDPart();
		
		if (sourceClass.equals("LegImpl")) {					
			String sourcePartId = ((Leg)w.getSource()).getParent().getParent().getId();
			for (int i=0; i<partList.size(); i++) {
				if (partList.get(i).getFritzingId().equals(sourcePartId)) {
					sourcePart = partList.get(i);
				}
			}
			sourcePinName = ((Leg)w.getSource()).getParent().getId();
		}
		if (sourceClass.equals("TerminalImpl")) {
			String sourcePartId = ((Terminal)w.getSource()).getParent().getId();
			for (int i=0; i<partList.size(); i++) {
				if (partList.get(i).getFritzingId().equals(sourcePartId)) {
					sourcePart = partList.get(i);
				}
			}
			sourcePinName = w.getSource().getId();
		}
		if (targetClass.equals("LegImpl")) {
			String targetPartId = ((Leg)w.getTarget()).getParent().getParent().getId();
			for (int i=0; i<partList.size(); i++) {
				if (partList.get(i).getFritzingId().equals(targetPartId)) {
					targetPart = partList.get(i);
				}
			}
			targetPinName = ((Leg)w.getTarget()).getParent().getId();
		}
		if (targetClass.equals("TerminalImpl")) {
			String targetPartId = ((Terminal)w.getTarget()).getParent().getId();
			for (int i=0; i<partList.size(); i++) {
				if (partList.get(i).getFritzingId().equals(targetPartId)) {
					targetPart = partList.get(i);
				}
			}
			targetPinName = w.getTarget().getId();
		}
		
		PartPinPair sourcePartAndPin = new PartPinPair(sourcePart, sourcePinName);
		PartPinPair targetPartAndPin = new PartPinPair(targetPart, targetPinName);
		
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
			result = result.concat(" " + pinList.get(i).getEagleBRDPart().getEaglePartLabel() + "." + pinList.get(i).getPinName());
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
	
	public void removeDuplicatePinEntries() {
		for (int i=0; i<pinList.size(); i++) {
			PartPinPair pinOne = pinList.get(i);
			for (int j=i+1; j<pinList.size(); j++) {
				PartPinPair pinTwo = pinList.get(j);
				if (pinOne.equals(pinTwo)) {
					pinList.remove(j);
					continue;
				}
			}
		}
	}
	

}

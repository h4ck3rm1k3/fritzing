package org.fritzing.pcb.export;

public class PartPinPair {
	private EagleBRDPart part;
	private String pinName = "";
	
	public PartPinPair(EagleBRDPart part, String pinName) {
		this.part = part;
		this.pinName = pinName;
	}
	
	public boolean equals(PartPinPair pin) {
		boolean result = false;
		if (pin.getEagleBRDPart().getFritzingId().equals(this.getEagleBRDPart().getFritzingId())) {
			if (pin.getPinName().equals(this.getPinName())) {
				result = true;
			}
		}
		return result;
	}
	
	public EagleBRDPart getEagleBRDPart() {
		return part;
	}
	
	public String getPartPinPairEaglePartLabel() {
		return this.part.getEaglePartLabel();
	}
	
	public String getPinName() {
		return this.pinName;
	}
}

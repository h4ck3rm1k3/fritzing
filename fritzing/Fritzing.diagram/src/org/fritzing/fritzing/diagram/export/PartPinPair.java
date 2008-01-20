package org.fritzing.fritzing.diagram.export;

public class PartPinPair {
	private String partName = "";
	private String pinName = "";
	
	public PartPinPair(String partName, String pinName) {
		this.partName = partName;
		this.pinName = pinName;
	}
	
	public boolean equals(PartPinPair pin) {
		boolean result = false;
		if (pin.getPartName().equals(this.partName) && pin.getPinName().equals(this.pinName)) {
			result = true;
		} else {
			result = false;
		}
		return result;
	}
	
	public String getPartName() {
		return this.partName;
	}
	
	public String getPinName() {
		return this.pinName;
	}
}

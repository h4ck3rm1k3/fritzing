package org.fritzing.pcb.export;


public class EagleSCRPart {
	/* replicate_schematic.brd - type syntax:
	 * 		ADD LED@fritzing-0001a 'LED1' R0.000 (1.050 1.450);
	 * what it needs: 
	 * start with "ADD"
	 * Library part and library - e.g. LED@fritzing-0001a
	 * 		from sketch object model where it is formatted as...
	 * TODO - do a better job describing the sources and formats for this information
	 * Part name - e.g. 'LED1'
	 * Rotation - e.g. R90.000
	 * 		front-end does not currently implement - will just fill with R0.000 for now (think it has to be three decimal places)
	 * Coordinates - e.g. (1.050 1.450)
	 * Semicolon to close it out
	 */

	public String fritzingPartId = "";	
	public String eagleFootprint = "";
	public String eagleLibraryName = "";
	public String fritzingPartLabel = "";
	
//	start deprecated variables
	public String partType = "";
	public String libraryName = "";
	public String partName = "";
	public String gateNumber = "G$1";
//  end deprecated variables
	
	public String rotationPrefix = "R";
	public String rotationVal = "0.000";
	public CoordPair fritzingPartPos = new CoordPair();
	public CoordPair fritzingPartPosInch = new CoordPair();
	public CoordPair partPos = new CoordPair();
	public int locked = 0;			// should the part's position be locked?
	
	// begin deprecated overloaded constructor
	public EagleSCRPart(String partName, String partType, String libraryName, CoordPair fritzingPartPos) {
		super();
		this.partName = partName;
		this.partType = partType; 
		this.libraryName = libraryName;
		this.fritzingPartPos = fritzingPartPos;
		this.fritzingPartPosInch = new CoordPair((float)((fritzingPartPos.xVal/1000) / 2.54), (float)((fritzingPartPos.yVal/1000) / 2.54));
		this.partPos = fritzingPartPosInch;
	}
	// end deprecated overloaded constructor
	
	
	
	public void setPosition(CoordPair partPos) {
		this.partPos = partPos;
	}
	
	// we provide the 'locked' flag and a few methods to control the so-called "locked" status
	// of a part.  some parts, once placed, should not be moved (e.g. the Arduino in very simple cases,
	// or buttons, LEDs, etc. if a user would like them in a particular place)
	// this lock is soft - the class' elements do not respect the lock flag so it is the responsibility
	// of the calling routine to check
	public int posIsLocked() {
		return this.locked;
	}
	
	public void lockPos() {
		locked = 1;
	}
	
	public void unlockPos() {
		locked = 0;
	}
}


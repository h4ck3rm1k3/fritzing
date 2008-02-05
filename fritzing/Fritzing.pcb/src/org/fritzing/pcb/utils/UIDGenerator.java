package org.fritzing.pcb.utils;

import java.util.Hashtable;
import java.util.UUID;

/*
 * There's also one at org.eclipse.emf.ecore.util.EcoreUtil.generateUUID() ..
 */

public class UIDGenerator {
	
	static protected Hashtable<Character, String> bitHash = new Hashtable<Character, String>();
	static protected Hashtable<String, Character> c64Hash = new Hashtable<String, Character>();
	static protected String[] zeroes = { "000000", "00000", "0000", "000", "00", "0", "" };
	
	/**
	 * Singleton
	 */
	private static UIDGenerator singletonInstance = new UIDGenerator();

	/**
	 * Return singleton instance.
	 * 
	 * @return the PartLoader registry
	 */
	public static UIDGenerator getInstance() {
		return singletonInstance;
	}
	
	public UIDGenerator () {
			bitHash.put('0', "0000");
			bitHash.put('1', "0001");
			bitHash.put('2', "0010");
			bitHash.put('3', "0011");
			bitHash.put('4', "0100");
			bitHash.put('5', "0101");
			bitHash.put('6', "0110");
			bitHash.put('7', "0111");
			bitHash.put('8', "1000");
			bitHash.put('9', "1001");
			bitHash.put('a', "1010");
			bitHash.put('b', "1011");
			bitHash.put('c', "1100");
			bitHash.put('d', "1101");
			bitHash.put('e', "1110");
			bitHash.put('f', "1111");
			bitHash.put('A', "1010");
			bitHash.put('B', "1011");
			bitHash.put('C', "1100");
			bitHash.put('D', "1101");
			bitHash.put('E', "1110");
			bitHash.put('F', "1111");
			c64Hash.put("000000", '0');	
			c64Hash.put("000001", '1');	
			c64Hash.put("000010", '2');	
			c64Hash.put("000011", '3');	
			c64Hash.put("000100", '4');	
			c64Hash.put("000101", '5');	
			c64Hash.put("000110", '6');	
			c64Hash.put("000111", '7');	
			c64Hash.put("001000", '8');	
			c64Hash.put("001001", '9');	
			c64Hash.put("001010", 'a');	
			c64Hash.put("001011", 'b');	
			c64Hash.put("001100", 'c');	
			c64Hash.put("001101", 'd');	
			c64Hash.put("001110", 'e');	
			c64Hash.put("001111", 'f');	
			c64Hash.put("010000", 'g');	
			c64Hash.put("010001", 'h');	
			c64Hash.put("010010", 'i');	
			c64Hash.put("010011", 'j');	
			c64Hash.put("010100", 'k');	
			c64Hash.put("010101", 'l');	
			c64Hash.put("010110", 'm');	
			c64Hash.put("010111", 'n');	
			c64Hash.put("011000", 'o');	
			c64Hash.put("011001", 'p');	
			c64Hash.put("011010", 'q');	
			c64Hash.put("011011", 'r');	
			c64Hash.put("011100", 's');	
			c64Hash.put("011101", 't');	
			c64Hash.put("011110", 'u');	
			c64Hash.put("011111", 'v');	
			c64Hash.put("100000", 'w');	
			c64Hash.put("100001", 'x');	
			c64Hash.put("100010", 'y');	
			c64Hash.put("100011", 'z');	
			c64Hash.put("100100", 'A');	
			c64Hash.put("100101", 'B');	
			c64Hash.put("100110", 'C');	
			c64Hash.put("100111", 'D');	
			c64Hash.put("101000", 'E');	
			c64Hash.put("101001", 'F');	
			c64Hash.put("101010", 'G');	
			c64Hash.put("101011", 'H');	
			c64Hash.put("101100", 'I');	
			c64Hash.put("101101", 'J');	
			c64Hash.put("101110", 'K');	
			c64Hash.put("101111", 'L');	
			c64Hash.put("110000", 'M');	
			c64Hash.put("110001", 'N');	
			c64Hash.put("110010", 'O');	
			c64Hash.put("110011", 'P');	
			c64Hash.put("110100", 'Q');	
			c64Hash.put("110101", 'R');	
			c64Hash.put("110110", 'S');	
			c64Hash.put("110111", 'T');	
			c64Hash.put("111000", 'U');	
			c64Hash.put("111001", 'V');	
			c64Hash.put("111010", 'X');	
			c64Hash.put("111011", 'X');	
			c64Hash.put("111100", 'Y');	
			c64Hash.put("111101", 'Z');	
			c64Hash.put("111110", '_');	
			c64Hash.put("111111", '-');
	}
	
	public String genUID() {
		String id = UUID.randomUUID().toString();
		id = id.replaceAll("-", "");
		
		StringBuffer bitString = new StringBuffer();
		for (int i = 0; i < id.length(); i++) {
			bitString.append(bitHash.get(id.charAt(i)));
		}
		
		StringBuffer id64 = new StringBuffer();
		String c64;
		for (int i = 0; i < bitString.length(); i += 6) {
			try {
				c64 = bitString.substring(i, i + 6);
			}
			catch (Exception ex) {
				c64 = bitString.substring(i);
				c64 = zeroes[c64.length()] + c64;
			}
			id64.append(c64Hash.get(c64));
		}
		
		return id64.toString();
	}
}

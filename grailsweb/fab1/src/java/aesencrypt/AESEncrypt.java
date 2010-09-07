
/*
 Encrypt and decrypt using the AES private key algorithm:  http://forums.sun.com/thread.jspa?threadID=5268311
*/

package aesencrypt;

import java.security.*;
import javax.crypto.*;
import javax.crypto.spec.SecretKeySpec;
import javax.crypto.spec.IvParameterSpec;
import java.security.spec.AlgorithmParameterSpec;
import org.apache.commons.codec.binary14.Base64;  		// hack because grails uses commons.codec 1.3 and we need 1.4
import java.util.Arrays;
import java.lang.reflect.Array;
 
public class AESEncrypt {
	public static final int block_size = 16;
	
	public static byte[] decrypt (String encoded_key, String encoded_encrypted_text) throws Exception {
	    
		byte [] newPlainText = null;
		try {
			Base64 base64 = new Base64(true);			// url safe (hope it's the same as Python's)
			byte[] key_bytes = base64.decode(encoded_key);
			
			//System.out.println("key size " + key_bytes.length);
			
			byte[] encrypted_text_plus = base64.decode(encoded_encrypted_text);
			
			byte[] encrypted_text = AESEncrypt.copyOfRange(encrypted_text_plus,  block_size, encrypted_text_plus.length);
			byte[] iv = AESEncrypt.copyOfRange(encrypted_text_plus, 0, block_size);
			
			SecretKeySpec skeySpec = new SecretKeySpec(key_bytes, "AES");
			Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");   //      NoPadding  
			AlgorithmParameterSpec paramSpec = new IvParameterSpec(iv);
			cipher.init(Cipher.DECRYPT_MODE, skeySpec, paramSpec);
			newPlainText = cipher.doFinal(encrypted_text);
			return newPlainText;
		}
		catch (Exception ex) {
			return null;
		}
	}
	
	public static byte[] decode(String encoded_string) {
		Base64 base64 = new Base64(true);			// url safe (hope it's the same as Python's)
		return base64.decode(encoded_string);
	}
	

	// Copied from Arrays.copyOfRange in openjdk 1.6 (not available in 1.5)
	public static byte[] copyOfRange(byte[] original, int from, int to) {
	   int newLength = to - from;
	   if (newLength < 0)
		   throw new IllegalArgumentException(from + " > " + to);
	   byte[] copy = new byte[newLength];
	   System.arraycopy(original, from, copy, 0,
		                Math.min(original.length - from, newLength));
	   return copy;
	}
}

		
		
	    
/*	
	    
        //
        byte[] plainText = "LOGIN=2222=v2-0-b7=SMST=smst=ASI".getBytes("utf-8");
        //
        // Get a DES private key
        System.out.println( "\nAES key" );
 
        String strKey = "75de8a33d3f18f1c29d86fa42b1894c7";
        byte[] keyBytes = hexToBytes(strKey);
 
        // skeyspec is the key to encrypt and decrypt
        SecretKeySpec skeySpec = new SecretKeySpec(keyBytes, "AES");
 
        System.out.println("Key: " + asHex(key.getEncoded()));
        System.out.println( "Finish generating AES key" );
        //
        // Creates the DES Cipher object (specifying the algorithm, mode, and padding).
        Cipher cipher = Cipher.getInstance("AES/ECB/PKCS7Padding");
        // Print the provider information
        System.out.println( "\n" + cipher.getProvider().getInfo() );
        //
        System.out.println( "\nStart encryption" );
        // Initializes the Cipher object.
        cipher.init(Cipher.ENCRYPT_MODE, skeySpec);
        // Encrypt the plaintext using the public key
        byte[] cipherText = cipher.doFinal(plainText);
        
        System.out.println( "Finish encryption: cipherText: " + asHex(cipherText));
        //
        System.out.println( "\nStart decryption" );
        // Initializes the Cipher object.
        cipher.init(Cipher.DECRYPT_MODE, skeySpec);
        // Decrypt the ciphertext using the same key
        byte[] newPlainText = cipher.doFinal(cipherText);
        System.out.println( "Finish decryption: " );
        System.out.print( asHex(newPlainText) );
    }
 */

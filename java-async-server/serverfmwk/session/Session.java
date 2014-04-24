package serverfmwk.session;

import javax.crypto.spec.SecretKeySpec;
import javax.crypto.KeyGenerator;

import java.io.*;

public class Session {
    private Integer id;
    private SecretKeySpec keySpec;

    public Session(Integer id, byte keySize) throws Exception {
	/* generate the keybits for this session */
	KeyGenerator kgen = KeyGenerator.getInstance("AES");
	if (keySize == 1) 
	    kgen.init(128);
	else if (keySize == 2)
	    kgen.init(192);
	else 
	    kgen.init(256);
	
	this.id = id;
	this.keySpec = new SecretKeySpec(kgen.generateKey().getEncoded(), "AES");
    }

    public byte[] getHello() {
	/* hello message, 1 byte keySize + Key bits + 4 byte session id */
	byte []raw = this.keySpec.getEncoded();
	byte []hello  = new byte[1 + raw.length + 4];

	if (raw.length == 16) 
	    hello[0] = (byte)1;
	else if (raw.length == 24)
	    hello[0] = (byte)2;
	else
	    hello[0] = (byte)3;

	int i = 0;
	for (i = 0; i < raw.length; i++) 
	    hello[i + 1] = raw[i];

	hello[i + 1] = (byte) ((this.id >> 24) & 0x000000ff);
	hello[i + 2] = (byte) ((this.id >> 16) & 0x000000ff);
	hello[i + 3] = (byte) ((this.id >>  8) & 0x000000ff);
	hello[i + 4] = (byte) ((this.id      ) & 0x000000ff);
	
	return hello;
    }

    private void showKey(byte[] array) {
	for (byte b : array) 
	    System.out.print(b + ",");
	System.out.println("");
    }
}
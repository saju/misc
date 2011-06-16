import AESKey;
import AES;


/* test AES encrypt-decrypt cycle */
public static function testAES():String {
    var i:uint = 0;
    
    /** create a 128 bit key and initialize the encryption decryption key schedules **/
    var keyBits:ByteArray = genAESKeyBits();

    var key:AESKey = new AESKey();
    var status:uint = key.generateKeys(keyBits);
    if (status != errors.OK) 
		return "testAES Key generation error : " + status.toString();

    /** create a 128 bit block; AES.BLOCK_SIZE == 128 **/
    var str:String = "abcdefghijklmnop";
    var plaintext:ByteArray = new ByteArray();
    plaintext.writeUTFBytes(str);
    plaintext.position = 0;

    /** Initialize the cipher core without padding, plaintext should be multiples of 128 bits **/
    var cipher:AES = new AES(false);

    var ciphertext:ByteArray = new ByteArray();
    status = cipher.encrypt(plaintext, ciphertext, key);
    if (status != errors.OK) 
		return "testAES AES encrypt() failure : " + status.toString();

    var plaintext2:ByteArray = new ByteArray();
    status = cipher.decrypt(ciphertext, plaintext2, key);
    if (status != errors.OK) 
		return "testAES AES decrypt() failure : " + status.toString();

    var str2:String = plaintext2.readUTFBytes(plaintext2.length);
    if (str2 != str) 
		return "testAES AES crypt cycle failed !!! msgs don't match";

    plaintext2.position = 0;
    plaintext2.length = 0;

    /*** rerun with random plaintext length and Padding ON ***/

    var cipher1:AES = new AES(true); 

    /** generate random plaintext of random length **/
    for (i = 0; i < uint(Math.random() * 1000); i++)
		plaintext2[i] = uint(Math.random() * 10); 

    var ciphertext2:ByteArray = new ByteArray();
    status = cipher1.encrypt(plaintext2, ciphertext2, key);
    if (status != errors.OK)
		return "testAES AES encrypt(pad) failed : " + status.toString();

    var plaintext3:ByteArray = new ByteArray();
    status = cipher1.decrypt(ciphertext2, plaintext3, key);
    if (status != errors.OK)
		return "testAES AES decrypt(pad) failed : " + status.toString();

    var str3:String = plaintext2.readUTFBytes(plaintext2.length);
    var str4:String = plaintext3.readUTFBytes(plaintext3.length);
    if (str3 != str4)
		return "testAES AES crypt cycle with padding failed !!! msgs don't match";
    return "OK";
}

private static function genAESKeyBits():ByteArray {
    var keyBits:ByteArray = new ByteArray();
    for (var i:uint = 0; i < 4; i++)
		keyBits.writeUnsignedInt(uint(Math.random() * 1000)) ;
    keyBits.position = 0;
    return keyBits;
}
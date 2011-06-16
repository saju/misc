/**
   This is a AS3 port of the AES key schedule routines from openssl's AES impl
   http://cvs.openssl.org/getfile/openssl/crypto/aes/aes_core.c?v=1.7.2.1.2.3

   this is public domain code

   Saju Pillai (saju.pillai@gmail.com)
**/
package crypto {

    import flash.utils.ByteArray;

    import AES;

    public class AESKey {
	public var rounds:uint;
	public var encKey:Array;
	public var decKey:Array;

	/**
	   Generates the expanded encryption and decryption key schedules
	**/
	public function generateKeys(raw:ByteArray):int {
	    var status:uint;

	    if (raw.length == 16) 
		rounds = 10;
	    else if (raw.length == 24)
		rounds = 12;
	    else if (raw.length == 32)
		rounds = 14;
	    else 
		return 1;
	    
	    encKey = new Array();
	    status = encryption_key_schedule(raw, encKey);
	    if (status != errors.OK)
		return status;
	    raw.position = 0;
	    decKey = new Array();
	    status = decryption_key_schedule(raw, decKey);
	    if (status != errors.OK)
		return status;
	    raw.position = 0;

	    return 0;
	}


	/**
	   expand the cipher bits into the AES encryption key expansion schedule
	**/
	private function encryption_key_schedule(raw:ByteArray, rk:Array):int {
	    var idx:uint = 0, i:uint = 0, temp:uint;
	    
	    rk[0] = raw.readUnsignedInt();
	    rk[1] = raw.readUnsignedInt();
	    rk[2] = raw.readUnsignedInt();
	    rk[3] = raw.readUnsignedInt();

	    if (rounds == 10) {
		while (1) {
		    temp = rk[3 + idx];
		    rk[4 + idx] = rk[idx] ^
			(AES.Te2[(temp >> 16) & 0xff] & 0xff000000) ^
			(AES.Te3[(temp >>  8) & 0xff] & 0x00ff0000) ^
			(AES.Te0[(temp      ) & 0xff] & 0x0000ff00) ^
			(AES.Te1[(temp >> 24) & 0xff] & 0x000000ff) ^
			AES.rcon[i];
		    rk[5 + idx] = rk[1 + idx] ^ rk[4 + idx];
		    rk[6 + idx] = rk[2 + idx] ^ rk[5 + idx];
		    rk[7 + idx] = rk[3 + idx] ^ rk[6 + idx];
		    if (++i == 10) 
			return 0;
		    idx += 4;
		}
	    }
	    
	    rk[4] = raw.readUnsignedInt();
	    rk[5] = raw.readUnsignedInt();

	    if (rounds == 12) {
		while (1) {
		    temp = rk[5 + idx];
		    rk[6 + idx] = rk[idx] ^
			(AES.Te2[(temp >> 16) & 0xff] & 0xff000000) ^
			(AES.Te3[(temp >>  8) & 0xff] & 0x00ff0000) ^
			(AES.Te0[(temp      ) & 0xff] & 0x0000ff00) ^
			(AES.Te1[(temp >> 24) & 0xff] & 0x000000ff) ^
			AES.rcon[i];
		    rk[7 + idx] = rk[1 + idx] ^ rk[6 + idx];
		    rk[8 + idx] = rk[2 + idx] ^ rk[7 + idx];
		    rk[9 + idx] = rk[3 + idx] ^ rk[8 + idx];
		    if (++i == 8) 
			return 0;
		    rk[10 + idx] = rk[4 + idx] ^ rk[9 + idx];
		    rk[11 + idx] = rk[5 + idx] ^ rk[10 + idx];
		    idx += 6;
		} 
	    }

	    rk[6] = raw.readUnsignedInt();
	    rk[7] = raw.readUnsignedInt();
	    
	    if (rounds == 14) {
		while (1) {
		    temp = rk[7 + idx];
		    rk[8 + idx] = rk[idx] ^
			(AES.Te2[(temp >> 16) & 0xff] & 0xff000000) ^
			(AES.Te3[(temp >>  8) & 0xff] & 0x00ff0000) ^
			(AES.Te0[(temp      ) & 0xff] & 0x0000ff00) ^
			(AES.Te1[(temp >> 24) & 0xff] & 0x000000ff) ^
			AES.rcon[i];
		    rk[9+idx] = rk[1+idx] ^ rk[8+idx];
		    rk[10+idx] = rk[2+idx] ^ rk[9+idx];
		    rk[11+idx] = rk[3+idx] ^ rk[10+idx];
		    if (++i == 7) 
			return 0;

		    temp = rk[11 + idx];
		    rk[12 + idx] = rk[4+idx] ^
			(AES.Te2[(temp >> 24) & 0xff] & 0xff000000) ^
			(AES.Te3[(temp >> 16) & 0xff] & 0x00ff0000) ^
			(AES.Te0[(temp >>  8) & 0xff] & 0x0000ff00) ^
			(AES.Te1[(temp      ) & 0xff] & 0x000000ff);
		    rk[13+idx] = rk[5+idx] ^ rk[12+idx];
		    rk[14+idx] = rk[6+idx] ^ rk[13+idx];
		    rk[15+idx] = rk[7+idx] ^ rk[14+idx];

		    idx += 8;
		}
	    }
	    return 1;
	} 


	/**
	   expand the cipher bits into the decryption key schedule
	**/
	private function decryption_key_schedule(raw:ByteArray, rk:Array):int {
	    var status:uint, i:uint, j:uint, temp:uint;

	    /* first run the encryption key schedule */
	    status = encryption_key_schedule(raw, rk);
	    if (status != 0)
		return status;

	    /* invert the order of the round keys: */
	    for (i = 0, j = 4*rounds; i < j; i += 4, j -= 4) {
		temp = rk[i    ]; rk[i    ] = rk[j    ]; rk[j    ] = temp;
		temp = rk[i + 1]; rk[i + 1] = rk[j + 1]; rk[j + 1] = temp;
		temp = rk[i + 2]; rk[i + 2] = rk[j + 2]; rk[j + 2] = temp;
		temp = rk[i + 3]; rk[i + 3] = rk[j + 3]; rk[j + 3] = temp;
	    }

	    /* apply the inverse MixColumn transform to all round keys but the first and the last: */
	    for (i = 1, j = 0; i < rounds; i++) {
		j += 4;
		rk[j] =
		    AES.Td0[AES.Te1[(rk[j] >> 24) & 0xff] & 0xff] ^
		    AES.Td1[AES.Te1[(rk[j] >> 16) & 0xff] & 0xff] ^
		    AES.Td2[AES.Te1[(rk[j] >>  8) & 0xff] & 0xff] ^
		    AES.Td3[AES.Te1[(rk[j]      ) & 0xff] & 0xff];
		rk[1 + j] =
		    AES.Td0[AES.Te1[(rk[1+j] >> 24) & 0xff] & 0xff] ^
		    AES.Td1[AES.Te1[(rk[1+j] >> 16) & 0xff] & 0xff] ^
		    AES.Td2[AES.Te1[(rk[1+j] >>  8) & 0xff] & 0xff] ^
		    AES.Td3[AES.Te1[(rk[1+j]      ) & 0xff] & 0xff];
		rk[2 + j] =
		    AES.Td0[AES.Te1[(rk[2+j] >> 24) & 0xff] & 0xff] ^
		    AES.Td1[AES.Te1[(rk[2+j] >> 16) & 0xff] & 0xff] ^
		    AES.Td2[AES.Te1[(rk[2+j] >>  8) & 0xff] & 0xff] ^
		    AES.Td3[AES.Te1[(rk[2+j]      ) & 0xff] & 0xff];
		rk[3 + j] =
		    AES.Td0[AES.Te1[(rk[3+j] >> 24) & 0xff] & 0xff] ^
		    AES.Td1[AES.Te1[(rk[3+j] >> 16) & 0xff] & 0xff] ^
		    AES.Td2[AES.Te1[(rk[3+j] >>  8) & 0xff] & 0xff] ^
		    AES.Td3[AES.Te1[(rk[3+j]      ) & 0xff] & 0xff];
	    }

	    return 0;
	}
    } 
}
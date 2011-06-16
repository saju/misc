/**
   PKCS5 padding support
   this is public domain code

   Saju Pillai (saju.pillai@gmail.com)
**/
package crypto {

    import flash.utils.ByteArray;

    public class Pad {
	public static function addPKCS5Padding(plain:ByteArray, block_size:uint):int {
	    var bytes:uint = block_size - plain.length % block_size;
	    
	    if (bytes == block_size) /* already aligned on byte boundary */
		return 0;
	    
	    for (var i:uint = 0; i < bytes; i++)
		plain[plain.length] = bytes;

            return 0;
	}

	public static function removePKCS5Padding(plain:ByteArray, block_size:uint):int {
	    if (plain.length % block_size != 0)
		return 1;

	    var byte:uint = plain[plain.length - 1];
	    for (var i:uint = plain.length-1; i > plain.length-1 - byte; i--) {
		if (uint(plain[i]) != byte)  /* there is no padding to remove here or this msg is wrongly padded
					   either way we don't care */
		    return 0;
	    }
	    /* drop "byte" bytes of padding */
	    plain.length -= byte;
	    return 0;
	}
    }
}
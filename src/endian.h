
/*
 * AF, 20.Nov.2018
 * convert host to little endian short
 * found at https://stackoverflow.com/questions/1873352/how-do-i-convert-a-value-from-host-byte-order-to-little-endian
 */
unsigned short htoles( unsigned short val);  /* hotToLittleEndian Short */
unsigned int   htolew( unsigned int val);    /* hotToLittleEndian Word  */

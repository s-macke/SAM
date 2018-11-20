

unsigned short swaps( unsigned short val)
{
    return ((val & 0xff) << 8) | ((val & 0xff00) >> 8);
}

/* host to little endian */

#if PLATFORM_IS_BIG_ENDIAN

unsigned short htoles( unsigned short val)
{
    /* need to swap bytes on a big endian platform */
    return swaps( val);
}
unsigned int htolew( unsigned int val)
{

    /* need to swap bytes and words on a big endian platform */
	int RetVal;
	((unsigned char *)&RetVal)[0]=((unsigned char *)&val)[3];
	((unsigned char *)&RetVal)[1]=((unsigned char *)&val)[2];
	((unsigned char *)&RetVal)[2]=((unsigned char *)&val)[1];
	((unsigned char *)&RetVal)[3]=((unsigned char *)&val)[0];

	return RetVal;
}


#else

unsigned short htoles( unsigned short val)
{
    /* no-op on a little endian platform */
    return val;
}

unsigned int htolew( unsigned int val)
{
    /* no-op on a little endian platform */
    return val;
}

#endif

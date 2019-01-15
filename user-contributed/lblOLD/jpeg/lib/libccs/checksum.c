/*	C H E C K S U M . C
#
@ Checksum routine for Internet Protocol family headers (C Version)
@
*/
short
checksum( register unsigned short*	buf, register int	len)
{
register int	sum = len&1 ? ((unsigned char *)buf)[--len]
#ifdef	LETTLE_ENDIAN
	<< 8
#endif
		: 0;

	/*	The algorithm is simple, add sequential 16 bit words to it,
	* and at the end, fold back all the carry bits from the top 16 bits
	* into the lower 16 bits.
	* To use 32-bit word will be faster for our own usage.
	*/
	while (len > 0)  {
		sum += *buf++;
		len -= 2;
	}

	/*
	* add back carry outs from top 16 bits to low 16 bits
	*/
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);	/* add carry	*/
	return	~sum;		/* truncate to 16 bits	*/
}



/*
 * pack.c - Routines to pack and unpack the bitmasks
 *
 *	Bryan Skene, LBL, 1/92
 *	-for use with Segal
 *
 */

#include "common.h"

#include "hips.h"
#include "load_save.h"

#define Realloc(x,y,z) (z *)realloc((char *)x,(unsigned)(y * sizeof(z)))

/*****************************************************/
void
Load_1_from_8(bit_mask, byte_mask)
u_char **bit_mask;
u_char **byte_mask;
{
	void put_val();

	int i, j;

	for(j = 0; j < segal.rows; j++)
	for(i = 0; i < segal.cols; i++) {
		put_val(bit_mask, i, j, byte_mask[j][i]);
	}
}

/*****************************************************/
void
Load_8_from_1(byte_mask, bit_mask)
u_char **byte_mask;
u_char **bit_mask;
{
	int get_val();

	int i, j;

	for(j = 0; j < segal.rows; j++)
	for(i = 0; i < segal.cols; i++)
		byte_mask[j][i] = get_val(bit_mask, i, j);
}

/*****************************************************/
void
put_val(bit_map, x, y, value)
u_char **bit_map;
int x, y;
int value;
{
	int index_byte, byte_val, byte_offset;
	double offset;

	offset = (double) (x + y * segal.rows);

	index_byte = (int) floor(offset/8.);
	byte_offset = (int) offset - (index_byte * 8);
	byte_val = bit_map[0][index_byte];

	if(value == 0)
		bit_map[0][index_byte] = ((128 >> byte_offset) ^ 127) & byte_val;
	else
		bit_map[0][index_byte] = (128 >> byte_offset) | byte_val;
}

/*****************************************************/
int
get_val(bit_map, x, y)
u_char **bit_map;
int x, y;
{
	int index_byte, byte_val, byte_offset, target_bit;
	double offset;

	offset = (double) (x + y * segal.rows);

	index_byte = (int) floor(offset/8.);
	byte_offset = (int) offset - (index_byte * 8);
	byte_val = bit_map[0][index_byte];
	target_bit = 7 - byte_offset;

	if((byte_val >> target_bit) & 1 == 1)
		return(PVAL);
	else
		return(0);
}

/*****************************************************/

/*****************************************************/


/*
 * Copyright (c) 1991 The Turing Institute
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_csinegen.c - subroutines to generate a test image containing sinusoids
 *               of varying spatial frequencies and amplitudes. Note that all
 *               frequency values must be given in radians/pixel and all
 *               angular values in radians. These subroutines generate
 *               images in which each row contains full cycles of each
 *               spatial frequency at a single amplitude.
 *
 * David Wilson - 14/1/92
 *
 */


#include <hipl_format.h>
#include <math.h>




int
h_csinegen (header,
	    base_frequency,
	    num_freq_divisions,
	    freq_scale,
	    num_cycles,
	    base_amplitude,
	    num_amp_divisions,
	    amp_scale,
	    phase,
	    pad_window)

struct header  *header;
float          base_frequency;
int            num_freq_divisions;
float          freq_scale;
int            num_cycles;
float          base_amplitude;
int            num_amp_divisions;
float          amp_scale;
float          phase;
int            pad_window;

{

    switch (header->pixel_format)
    {
	case PFFLOAT:  return (h_csinegen_f (header,
					     base_frequency,
					     num_freq_divisions,
					     freq_scale,
					     num_cycles,
					     base_amplitude,
					     num_amp_divisions,
					     amp_scale,
					     phase,
					     pad_window));

	case PFDOUBLE:  return (h_csinegen_d (header,
					      base_frequency,
					      num_freq_divisions,
					      freq_scale,
					      num_cycles,
					      base_amplitude,
					      num_amp_divisions,
					      amp_scale,
					      phase,
					      pad_window));

	default:       return (perr (HE_FMTSUBR,
				     "h_sinegen",
				     hformatname(header->pixel_format)));
    }

}




int
h_csinegen_f (header,
	     base_frequency,
	     num_freq_divisions,
	     freq_scale,
	     num_cycles,
	     base_amplitude,
	     num_amp_divisions,
	     amp_scale,
	     phase,
	     pad_window)

struct header  *header;
float          base_frequency;
int            num_freq_divisions;
float          freq_scale;
int            num_cycles;
float          base_amplitude;
int            num_amp_divisions;
float          amp_scale;
float          phase;
int            pad_window;

{

    return (h_csinegen_F ((float *) header->firstpix,
		          header->rows,
			  header->cols,
			  base_frequency,
			  num_freq_divisions,
			  freq_scale,
			  num_cycles,
			  base_amplitude,
			  num_amp_divisions,
			  amp_scale,
			  phase,
			  pad_window));
}




int
h_csinegen_d (header,
	      base_frequency,
	      num_freq_divisions,
	      freq_scale,
	      num_cycles,
	      base_amplitude,
	      num_amp_divisions,
	      amp_scale,
	      phase,
	      pad_window)

struct header  *header;
float          base_frequency;
int            num_freq_divisions;
float          freq_scale;
int            num_cycles;
float          base_amplitude;
int            num_amp_divisions;
float          amp_scale;
float          phase;
int            pad_window;

{

    return (h_csinegen_D ((double *) header->firstpix,
		          header->rows,
			  header->cols,
			  base_frequency,
			  num_freq_divisions,
			  freq_scale,
			  num_cycles,
			  base_amplitude,
			  num_amp_divisions,
			  amp_scale,
			  phase,
			  pad_window));
}




int
h_csinegen_F (image,
	      num_rows,
	      num_cols,
	      base_frequency,
	      num_freq_divisions,
	      freq_scale,
	      num_cycles,
	      base_amplitude,
	      num_amp_divisions,
	      amp_scale,
	      phase,
	      pad_window)

float  *image;
int    num_rows;
int    num_cols;
float  base_frequency;
int    num_freq_divisions;
float  freq_scale;
int    num_cycles;
float  base_amplitude;
int    num_amp_divisions;
float  amp_scale;
float  phase;
int    pad_window;

{

    int     division;
    int     start_col, end_col;
    int     row, col;

    float   *image_ptr;
    float   *unity_signal, *unity_ptr;
    float   frequency, amplitude;
    

    /*
    ** Set a pointer to the last row of the image. This will be used as a
    ** temporary asve for a sinusoid signal wth unity gain.
    */

    unity_signal = image + (num_rows - 1) * num_cols;

    /*
    ** Generate the unity signal, covering each spatial frequency
    ** division.
    */

    start_col = 0;
    end_col = 0;
    unity_ptr = unity_signal;
    frequency = base_frequency;

    for (division = 0; division < num_freq_divisions; division++)
    {
	/*
	** Determine the column width filled up by the sinusoid
	** at this spatial frequency.
	*/

        start_col = end_col;
	if (division == 0)
	{
	    end_col += (num_cycles + pad_window) * floor (H_2PI / frequency);
	}
	else if (division == num_freq_divisions - 1)
        {
            end_col = num_cols;
	}
	else
	{
	    end_col += num_cycles * floor (H_2PI / frequency);
	}

        for (col = 0; col < end_col - start_col; col++)
        {
	    *unity_ptr = (float) sin (col * frequency + phase);
	    unity_ptr++;
        }

        frequency *= freq_scale;
    }

    /*
    ** Generate the image rows by scaling the unity sinusoid signal
    ** at the correct amplitude. Replicate the amplitude at the pad
    ** window.
    */

    image_ptr = image;
    amplitude = base_amplitude;

    for (row = 0; row < num_rows; row++)
    {
	unity_ptr = unity_signal;

        for (col = 0; col < num_cols; col++)
	{
	    *image_ptr = amplitude * (*unity_ptr);
	    unity_ptr++;
	    image_ptr++;
	}

        if (row >= pad_window &&
	    row < num_rows - pad_window)
	{
	    amplitude *= amp_scale;
	}
    }

    return(HIPS_OK);
}




int
h_csinegen_D (image,
	      num_rows,
	      num_cols,
	      base_frequency,
	      num_freq_divisions,
	      freq_scale,
	      num_cycles,
	      base_amplitude,
	      num_amp_divisions,
	      amp_scale,
	      phase,
	      pad_window)

double  *image;
int    num_rows;
int    num_cols;
float  base_frequency;
int    num_freq_divisions;
float  freq_scale;
int    num_cycles;
float  base_amplitude;
int    num_amp_divisions;
float  amp_scale;
float  phase;
int    pad_window;

{

    int      division;
    int      start_col, end_col;
    int      row, col;

    float    frequency, amplitude;

    double   *image_ptr;
    double   *unity_signal, *unity_ptr;
    

    /*
    ** Set a pointer to the last row of the image. This will be used as a
    ** temporary asve for a sinusoid signal wth unity gain.
    */

    unity_signal = image + (num_rows - 1) * num_cols;

    /*
    ** Generate the unity signal, covering each spatial frequency
    ** division.
    */

    start_col = 0;
    end_col = 0;
    unity_ptr = unity_signal;
    frequency = base_frequency;

    for (division = 0; division < num_freq_divisions; division++)
    {
	/*
	** Determine the column boundaries filled up by the sinusoid
	** at this spatial frequency.
	*/

        start_col = end_col;
	if (division == 0)
	{
	    end_col += (num_cycles + pad_window) * floor (H_2PI / frequency);
	}
	else if (division == num_freq_divisions - 1)
        {
            end_col = num_cols;
	}
	else
	{
	    end_col += num_cycles * floor (H_2PI / frequency);
	}

        for (col = 0; col < end_col - start_col; col++)
        {
	    *unity_ptr = (float) sin (col * frequency + phase);
	    unity_ptr++;
        }

        frequency *= freq_scale;
    }

    /*
    ** Generate the image rows by scaling the unity sinusoid signal
    ** at the correct amplitude. Replicate the amplitude within the
    ** pad window.
    */

    image_ptr = image;
    amplitude = base_amplitude;

    for (row = 0; row < num_rows; row++)
    {
	unity_ptr = unity_signal;

        for (col = 0; col < num_cols; col++)
	{
	    *image_ptr = amplitude * (*unity_ptr);
	    unity_ptr++;
	    image_ptr++;
	}

        if (row >= pad_window &&
	    row < num_rows - pad_window)
	{
	    amplitude *= amp_scale;
	}
    }

    return(HIPS_OK);
}

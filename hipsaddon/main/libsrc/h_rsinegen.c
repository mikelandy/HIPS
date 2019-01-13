/*
 * Copyright (c) 1991 The Turing Institute
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_rsinegen.c - subroutines to generate a test image containing sinusoids
 *                of varying spatial frequencies and amplitudes. Note that all
 *                of frequency values must be given in radians/pixel and all
 *                of angular values in radians. Each row in the generated
 *                image contains a frequency ramp from the lowest
 *                frequency to the highest frequency. Each column in the
 *                generated image contains an amplitude ramp.
 *
 * David Wilson - 14/1/92
 *
 */


#include <hipl_format.h>
#include <math.h>




int
h_rsinegen (header,
	    from_frequency,
	    to_frequency,
	    from_amplitude,
	    to_amplitude,
	    phase,
	    pad_window)

struct header  *header;
float          from_frequency;
float          to_frequency;
float          from_amplitude;
float          to_amplitude;
float          phase;
int            pad_window;

{

    switch (header->pixel_format)
    {
	case PFFLOAT:  return (h_rsinegen_f (header,
					     from_frequency,
					     to_frequency,
					     from_amplitude,
					     to_amplitude,
					     phase,
					     pad_window));

	case PFDOUBLE:  return (h_rsinegen_d (header,
					     from_frequency,
					     to_frequency,
					     from_amplitude,
					     to_amplitude,
					     phase,
					     pad_window));

	default:       return (perr (HE_FMTSUBR,
				     "h_rsinegen",
				     hformatname(header->pixel_format)));
    }

}




int
h_rsinegen_f (header,
	      from_frequency,
	      to_frequency,
	      from_amplitude,
	      to_amplitude,
	      phase,
	      pad_window)

struct header  *header;
float          from_frequency;
float          to_frequency;
float          from_amplitude;
float          to_amplitude;
float          phase;
int            pad_window;

{

    return (h_rsinegen_F ((float *) header->firstpix,
		          header->rows,
			  header->cols,
		          from_frequency,
		          to_frequency,
		          from_amplitude,
		          to_amplitude,
		          phase,
			  pad_window));

}




int
h_rsinegen_d (header,
	      from_frequency,
	      to_frequency,
	      from_amplitude,
	      to_amplitude,
	      phase,
	      pad_window)

struct header  *header;
float          from_frequency;
float          to_frequency;
float          from_amplitude;
float          to_amplitude;
float          phase;
int            pad_window;

{

    return (h_rsinegen_D ((double *) header->firstpix,
		          header->rows,
			  header->cols,
		          from_frequency,
		          to_frequency,
		          from_amplitude,
		          to_amplitude,
		          phase,
			  pad_window));

}




int
h_rsinegen_F (image,
	      num_rows,
	      num_cols,
	      from_frequency,
	      to_frequency,
	      from_amplitude,
	      to_amplitude,
	      phase,
	      pad_window)

float  *image;
int    num_rows;
int    num_cols;
float  from_frequency;
float  to_frequency;
float  from_amplitude;
float  to_amplitude;
float  phase;
int    pad_window;

{

    int     row, col;
    int     num_ramp_cols, last_ramp_col;

    float   *image_ptr;
    float   *unity_signal, *unity_ptr;
    float   frequency, amplitude;
    float   increment;
    

    /*
    ** Set a pointer to the last row of the image. This will be used as a
    ** temporary save for a sinusoid signal wth unity gain.
    */

    unity_signal = image + (num_rows - 1) * num_cols;

    /*
    ** Generate the unity signal, covering each spatial frequency.
    ** This includes the pad window. Note that the frequency is
    ** not ramped within the pad window and also that that the
    ** signal is effectively shifted so that the specified phase begins
    ** outwith the pad window.
    */

    num_ramp_cols = num_cols - (2 *  pad_window);

    unity_ptr = unity_signal;
    frequency = from_frequency;
    increment = (to_frequency - from_frequency) / (num_ramp_cols - 1);

    /*
    ** Shift cols by -pad_windows to correct phase.
    */

    last_ramp_col = num_cols - (2 * pad_window);

    for (col = -pad_window; col < num_cols; col++)
    {

	*unity_ptr = (float) sin (col * frequency + phase);
	unity_ptr++;

        if (col >= 0 &&
	    col < last_ramp_col)
	{
            frequency += increment;
	}
    }

    /*
    ** Generate the image rows by scaling the unity sinusoid signal
    ** at the correct amplitude.
    */

    image_ptr = image;
    amplitude = from_amplitude;
    increment = (to_amplitude - from_amplitude) / (num_rows - 1);

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
	    amplitude += increment;
	}
    }

    return(HIPS_OK);
}




int
h_rsinegen_D (image,
	      num_rows,
	      num_cols,
	      from_frequency,
	      to_frequency,
	      from_amplitude,
	      to_amplitude,
	      phase,
	      pad_window)

double  *image;
int    num_rows;
int    num_cols;
float  from_frequency;
float  to_frequency;
float  from_amplitude;
float  to_amplitude;
float  phase;
int    pad_window;

{

    int      row, col;
    int      num_ramp_cols, last_ramp_col;

    double   *image_ptr;
    double   *unity_signal, *unity_ptr;
    double   frequency, amplitude;
    double   increment;
    

    /*
    ** Set a pointer to the last row of the image. This will be used as a
    ** temporary asve for a sinusoid signal wth unity gain.
    */

    unity_signal = image + (num_rows - 1) * num_cols;

    /*
    ** Generate the unity signal, covering each spatial frequency.
    ** This includes the pad window. Note that the frequency is
    ** not ramped within the pad window and also that that the
    ** signal is effectively shifted so that the specified phase begins
    ** outwith the pad window.
    */

    num_ramp_cols = num_cols - (2 * pad_window);

    unity_ptr = unity_signal;
    frequency = from_frequency;
    increment = (to_frequency - from_frequency) / (num_ramp_cols - 1);

    /*
    ** Shift cols by -pad_windows to correct phase.
    */
    last_ramp_col = num_cols - (2 * pad_window);

    for (col = -pad_window; col < num_cols - pad_window; col++)
    {

	*unity_ptr = (float) sin (col * frequency + phase);
	unity_ptr++;

        if (col >= 0 &&
	    col < last_ramp_col)
	{
            frequency += increment;
	}
    }

    /*
    ** Generate the image rows by scaling the unity sinusoid signal
    ** at the correct amplitude.
    */

    image_ptr = image;
    amplitude = from_amplitude;
    increment = (to_amplitude - from_amplitude) / (num_rows - 1);

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
	    amplitude += increment;
	}
    }

    return(HIPS_OK);
}

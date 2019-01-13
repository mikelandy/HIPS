/*
   server.c
*/
/***********************************************************************/
/************* routines for server version of isobuild  ****************/
/***********************************************************************/
/*
 * for use with isoserv program:  written by David Robertson, LBL
 *					and  Brian Tierney, LBL
 */

/* $Id: server.c,v 1.6 1992/01/31 02:05:45 tierney Exp $ */

/* $Log: server.c,v $
 * Revision 1.6  1992/01/31  02:05:45  tierney
 * *** empty log message ***
 *
 * Revision 1.5  1992/01/30  20:06:09  davidr
 * before Brian's changes
 *
 * Revision 1.4  1992/01/10  01:59:31  davidr
 * works with triserv now
 *
 * Revision 1.3  1992/01/07  23:15:43  davidr
 * playing around with what to do with false
 * surfaces
 *
 * Revision 1.2  1991/12/19  01:42:20  davidr
 * added RCS identification markers
 * */

static char rcsid[] = "$Id: server.c,v 1.6 1992/01/31 02:05:45 tierney Exp $";

#include "isobuild.h"
#include "server.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/signal.h>
#include <ctype.h>

struct movie_control {
    int       num_frames;
    float     TVAL[2];
    float     d_TVAL;
    int       SX[2];
    int       SY[2];
    int       SZ[2];
    int       EX[2];
    int       EY[2];
    int       EZ[2];
    float     d_SX;
    float     d_SY;
    float     d_SZ;
    float     d_EX;
    float     d_EY;
    float     d_EZ;
};

struct movie_control movie;

char      render_host[80];
int       render_portnum;
int       render_sock;
int       ui_sock;
int       render_start = 1;

int       tessel_param_changed;

/***********************************************************************/

surfserv()
{
    int       sock;
    struct sockaddr_in server;
    char      proc_num;
    int       rval;
    FILE     *fp = NULL;
    FILE     *open_surf_file(), *gen_highres();
    void      catch_sigs();


    catch_sigs();		/* catch main interrupts to send msg to
				 * client */


    /* create socket  */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
	perror("opening stream socket");
	exit(1);
    }
    /* name socket using wildcards */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = ntohs(PORT_NUM);
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) 0, 0);
    if (bind(sock, (char *) &server, sizeof(server))) {
	perror("binding stream socket");
	exit(1);
    }
    printf("socket has port #%d\n", ntohs(server.sin_port));

    /* Start accepting connections */
    listen(sock, 5);
    do {
	ui_sock = accept(sock, 0, 0);
	if (ui_sock == -1)
	    perror("accept");
	else
	    do {
		if ((rval = read(ui_sock, &proc_num, sizeof(char))) < 0) {
		    Status("NULLPROC read failure");
		}
#ifdef SERVER_DEBUG
		Status("Contact from the client");
#endif

		if (rval > 0) {
		    if (proc_num == FILEPROC)
			fp = open_surf_file();
		    else if (proc_num == PARAMPROC && fp != NULL)
			gen_update();
		    else if (proc_num == MOVIEPROC && fp != NULL)
			gen_movie();
		    else if (proc_num == HIGHRESPROC && fp != NULL)
			fp = gen_highres();
		}
	    } while (rval != 0);
	if (fp != NULL)
	    fclose(fp);
	fp = NULL;
	close(ui_sock);
	close(render_sock);
	render_start = 1;
    } while (TRUE);
    /* need to kill this process to stop it */
}


/************************************************************************/

gen_update()
{
    int       inbuf[PARAM_READ_LENGTH], tval;
    void      tessellate_volume(), tessellate_blocks();

#ifdef SERVER_DEBUG
    Status(" in gen_update.");
#endif

    read(ui_sock, inbuf, PARAM_READ_LENGTH * SUNINT);
    SEG_METHOD = inbuf[0];
    tval = (Data_type) inbuf[1];
    SX = inbuf[2];
    SY = inbuf[3];
    SZ = inbuf[4];
    EX = inbuf[5];
    EY = inbuf[6];
    EZ = inbuf[7];
    tessel_param_changed = inbuf[11];
    if (inbuf[13] == 0)
	DIVIDING = 1;
    else
	MARCH_NORMALS = inbuf[13] - 1;

    TVAL = tval * (data_max - data_min) / 255.;

#ifdef SERVER_DEBUG
    fprintf(stderr, " recieved the following settings: \n");
    fprintf(stderr, " SX: %d, SY: %d, SZ: %d, EX: %d, EY: %d, EZ: %d \n",
	    SX, SY, SZ, EX, EY, EZ);
    fprintf(stderr, "threshold value: %d , scaled: %.2f \n", tval, (float) TVAL);
    fprintf(stderr, "segment: %d \n", SEG_METHOD);
#endif

    if (IN_MASK_FNAME != NULL && !DIVIDING)
	SEG_METHOD = 4;

    if (SEG_METHOD > 1 && SKIP_BLOCKS)
/*
 * Can't use skip-block option with flood fill segmentation
 */
	SKIP_BLOCKS = 0;

    if (SKIP_BLOCKS)
	tessellate_blocks();
    else
	tessellate_volume();

}

/*******************************************************************/
FILE     *
open_surf_file()
{
    char      s_infile[80];
    FILE     *in_fp, *open_data_file();
    int       rval, msg, len, size;
    int       buf[FILEPROC_READ_LENGTH];
    void      alloc_data_arrays(), compute_normal_array(), get_max_min();
    int       hostlen;

#ifdef SERVER_DEBUG
    fprintf(stderr, " in open_surf_file: \n");
#endif

    rval = read(ui_sock, buf, FILEPROC_READ_LENGTH * SUNINT);
#ifdef SERVER_DEBUG
    fprintf(stderr, "read %d bytes \n", rval);
#endif
    if (rval == 0) {
	Error("problem reading from socket.");
    }
    len = buf[0];
    hostlen = buf[1];
    strncpy(s_infile, (char *) (buf + 2), len);
    strncpy(render_host, (char *) (buf + 22), hostlen);
    /* KLUDGE:  change! */
    render_portnum = buf[43] + 10000;

    s_infile[len] = '\0';

    render_host[hostlen] = '\0';

    if ((in_fp = open_data_file(s_infile)) == NULL)
	return (in_fp);

    alloc_data_arrays(xdim, ydim, zdim);

    /* read in entire data set */
    size = xdim * ydim * zdim;
    if (read_hips_data(in_fp, data, size) < 0)
	return (NULL);

    if (IN_MASK_FNAME != NULL && !DIVIDING) {
	/* read in mask data */
	if (read_mask_file(size) < 0)
	    return (NULL);
    }
    if (SKIP_BLOCKS)
	num_blocks = block_setup();
    else {
	/* find minimum and maximum values in the file */
	get_max_min(size, &data_max, &data_min);
	if (VERBOSE)
	    fprintf(stderr, "Minimum data value: %.1f, Maximum data value: %.1f \n",
		    (float) data_min, (float) data_max);
    }

    if (PRE_NORMALS)		/* compute normals for entire data set */
	compute_normal_array();


#ifdef SERVER_DEBUG
    fprintf(stderr, " sending initial header \n");
#endif

    msg = 1;
    buf[0] = msg;
    buf[1] = xdim;
    buf[2] = ydim;
    buf[3] = zdim;


    write(ui_sock, buf, FILEPROC_WRITE_LENGTH * SUNINT);

    return (in_fp);
}

/**************************************************************/
FILE     *
open_data_file(s_infile)
    char     *s_infile;
{
    FILE     *in_fp = NULL;
    char      old_infile[80];
    int       msg;
    int       buf[4];

    if (strcmp(s_infile, old_infile) == 0) {
	fprintf(stderr, "file already loaded %s\n", s_infile);
	msg = 0;
	buf[0] = buf[1] = buf[2] = buf[3] = msg;
	write(ui_sock, buf, FILEPROC_WRITE_LENGTH * SUNINT);
	return (in_fp);
    }
#ifdef SERVER_DEBUG
    fprintf(stderr, "trying to open %s \n", s_infile);
#endif

    in_fp = fopen(s_infile, "r");
    if (in_fp == NULL) {
	fprintf(stderr, "no such data file %s\n", s_infile);
	msg = 0;
	buf[0] = buf[1] = buf[2] = buf[3] = msg;
	write(ui_sock, buf, FILEPROC_WRITE_LENGTH * SUNINT);
#ifdef SERVER_DEBUG
	fprintf(stderr, "error message sent to client: %d \n", buf[0]);
#endif
	return (in_fp);
    }
    IN_FNAME = s_infile;	/* set global */
#ifdef DEBUG
    fprintf(stderr, "opening file %s \n", IN_FNAME);
#endif

    if (get_size(in_fp) < 0)
	return (NULL);

    return (in_fp);
}

/*******************************************************************/
void
alloc_data_arrays(xsize, ysize, zsize)
    int       xsize, ysize, zsize;
{
    static int old_xdim = 0, old_ydim = 0, old_zdim = 0;
    Grid_type ***alloc_3d_grid_array();
    Data_type ***alloc_3d_data_array();
    CUBE_TRIANGLES **alloc_2d_cube_array();
    FLOAT_VECT **alloc_2d_vector_array();
    NORMAL_VECT ***alloc_3d_normal_array();

    if (xsize != old_xdim || ysize != old_ydim || zsize != old_zdim) {
	BLOCK_SIZE = 0;		/* force block_size recomputation */

	if (grid != NULL) {
	    free_3d_grid_array(grid);
	    grid = NULL;
	}
	if (data != NULL) {
	    free_3d_data_array(data);
	    data = NULL;
	}
	if (normals != NULL) {
	    free_3d_normal_array(normals);
	    normals = NULL;
	}
	if (block_info_array != NULL) {
	    free_block_info_array(block_info_array);
	    block_info_array = NULL;
	}
	if (prevslice != NULL) {
	    free_2d_cube_array(prevslice);
	    prevslice = NULL;
	}
	if (currslice != NULL) {
	    free_2d_cube_array(currslice);
	    currslice = NULL;
	}
	if (gradient_curr_slice != NULL) {
	    free_2d_vector_array(gradient_curr_slice);
	    gradient_curr_slice = NULL;
	}
	if (gradient_next_slice != NULL) {
	    free_2d_vector_array(gradient_next_slice);
	    gradient_next_slice = NULL;
	}
    } else
	return;

    old_xdim = xsize;
    old_ydim = ysize;
    old_zdim = zsize;

    if (grid == NULL)
	grid = alloc_3d_grid_array(zsize, ysize, xsize);

    if (data == NULL)
	data = alloc_3d_data_array(zsize, ysize, xsize);

    if (PRE_NORMALS && normals == NULL)
	normals = alloc_3d_normal_array(zsize, ysize, xsize);

    if (DUP_CHECK && prevslice == NULL) {	/* for checking for dup verts */
	prevslice = alloc_2d_cube_array(ysize, xsize);
	currslice = alloc_2d_cube_array(ysize, xsize);
    }
    if (DIVIDING && DO_CUBE_CENTER_NORMALS) {
	gradient_curr_slice = alloc_2d_vector_array(ydim, xdim);
	gradient_next_slice = alloc_2d_vector_array(ydim, xdim);
    }
    return;
}

/************************************************************************/

gen_movie()
{
    int       inbuf[MOVIE_READ_LENGTH];
    void      tessellate_volume(), tessellate_blocks(), get_increments();
    int       i;
    float     tval;

#ifdef SERVER_DEBUG
    fprintf(stderr, " in gen_movie: \n");
#endif

    read(ui_sock, inbuf, MOVIE_READ_LENGTH * SUNINT);

    movie.num_frames = inbuf[1];
    SEG_METHOD = inbuf[2];
    movie.TVAL[0] = (Data_type) inbuf[3];
    movie.TVAL[1] = (Data_type) inbuf[4];
    movie.SX[0] = inbuf[5];
    movie.SX[1] = inbuf[6];
    movie.SY[0] = inbuf[7];
    movie.SY[1] = inbuf[8];
    movie.SZ[0] = inbuf[9];
    movie.SZ[1] = inbuf[10];
    movie.EX[0] = inbuf[11];
    movie.EX[1] = inbuf[12];
    movie.EY[0] = inbuf[13];
    movie.EY[1] = inbuf[14];
    movie.EZ[0] = inbuf[15];
    movie.EZ[1] = inbuf[16];
    /* CHANGE:  need MARCH_NORMALS */


#ifdef SERVER_DEBUG
    fprintf(stderr, " recieved the following movie settings: \n");
    fprintf(stderr, "threshold start: %d, end: %d \n",
	    movie.TVAL[0], movie.TVAL[1]);
    fprintf(stderr, "segment: %d  num_frames: %d\n",
	    SEG_METHOD, movie.num_frames);
#endif

    get_increments();

    if (!tessel_param_changed)
	movie.num_frames = 1;
    for (i = 0; i <= movie.num_frames; i++) {
	tval = movie.TVAL[0] + (movie.d_TVAL * (float) i);
	TVAL = tval * (data_max - data_min) / 255.;
	SX = movie.SX[0] + (int) (movie.d_SX * i);
	SY = movie.SY[0] + (int) (movie.d_SY * i);
	SZ = movie.SZ[0] + (int) (movie.d_SZ * i);
	EX = movie.EX[0] + (int) (movie.d_EX * i);
	EY = movie.EY[0] + (int) (movie.d_EY * i);
	EZ = movie.EZ[0] + (int) (movie.d_EZ * i);

#ifdef SERVER_DEBUG
	fprintf(stderr, "calling tessellate_volume with the following args: \n");
	fprintf(stderr, " SX: %d, SY: %d, SZ: %d, EX: %d, EY: %d, EZ: %d \n",
		SX, SY, SZ, EX, EY, EZ);
#endif

	if (SEG_METHOD > 0 && SKIP_BLOCKS)
/*
 * Can't use skip-block option with flood fill segmentation
 */
	    SKIP_BLOCKS = 0;

	if (SKIP_BLOCKS)
	    tessellate_blocks();
	else
	    tessellate_volume();

    }
}

/********************************************************/
void
get_increments()
{

    if (movie.num_frames >= 2) {
	movie.d_TVAL = ((float) (movie.TVAL[1] - movie.TVAL[0]) /
			(float) (movie.num_frames));
	movie.d_SX = ((float) (movie.SX[1] - movie.SX[0]) /
		      (float) (movie.num_frames));
	movie.d_SY = ((float) (movie.SY[1] - movie.SY[0]) /
		      (float) (movie.num_frames));
	movie.d_SZ = ((float) (movie.SZ[1] - movie.SZ[0]) /
		      (float) (movie.num_frames));
	movie.d_EX = ((float) (movie.EX[1] - movie.EX[0]) /
		      (float) (movie.num_frames));
	movie.d_EY = ((float) (movie.EY[1] - movie.EY[0]) /
		      (float) (movie.num_frames));
	movie.d_EZ = ((float) (movie.EZ[1] - movie.EZ[0]) /
		      (float) (movie.num_frames));
    }
    return;
}

/********************************************************/
FILE     *
gen_highres()
{
    /* not yet implemented !! */
    return (NULL);
}

/********************************************************/
/* connect to rendering server */

connect_to_renderer(r_host, r_portnum)
    char     *r_host;		/* symbolic or Internet dot host name of
				 * server */
    int       r_portnum;	/* port number */
{
    struct sockaddr_in r_server;
    struct hostent *hp, *gethostbyname();
    int       is_int_addr;	/* is in Internet address form */
    unsigned long internet_addr;/* Internet address */


    /* check whether symbolic name or Internet address */
    check_host_type(r_host, &is_int_addr, &internet_addr);
    /* if "dotted" Internet address */
    if (is_int_addr)
	r_server.sin_addr.s_addr = internet_addr;
    else {			/* symbolic name */
	if ((hp = gethostbyname(r_host)) == NULL) {
	    fprintf(stderr, "can't find server address '%s'\n", r_host);
	    exit(0);
	}
	memcpy((caddr_t) & r_server.sin_addr, hp->h_addr, hp->h_length);
    }

    /* create socket */
    render_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (render_sock < 0) {
	perror("opening stream renderer socket");
	exit(1);
    }
    fprintf(stderr, "Connecting to host %s at port %d, render socket # %d \n",
	    r_host, r_portnum, render_sock);

    /* connect socket using name specified by command line. */
    r_server.sin_family = AF_INET;
    r_server.sin_port = htons(r_portnum);

    /* establish connection */
    if (connect(render_sock, &r_server, sizeof(r_server)) < 0) {
	perror("connecting stream renderer socket");
	exit(1);
    }
}

/********************************************************/

check_host_type(host, inter_addr, coded)
    char     *host;		/* reference to server */
    int      *inter_addr;	/* Internet address or not */
    unsigned long *coded;	/* Internet address */

{
    int       i;
    int       j = 0;
    int       parse_ptr = 0;
    char      parse_addr[4][5];
    int       temp_int;

    *coded = 0;
    *inter_addr = 0;
    /* see if contains 4 numbers and 3 periods */
    for (i = 0; i < strlen(host); i++) {
	if (host[i] == '.') {
	    if (j == 0) {
		*inter_addr = 0;
		break;
	    } else {
		parse_addr[parse_ptr][j] = '\0';
		j = 0;
		++parse_ptr;
		if (parse_ptr > 3) {
		    *inter_addr = 0;
		    break;
		}
	    }
	} else if (j < 3) {
	    if (!isdigit(host[i])) {
		*inter_addr = 0;
		break;
	    }
	    parse_addr[parse_ptr][j++] = host[i];
	    *inter_addr = 1;
	} else {
	    *inter_addr = 0;
	    break;
	}
    }
    if (*inter_addr && (parse_ptr == 3)) {
	parse_addr[3][j] = '\0';
	if ((temp_int = atoi(parse_addr[0])) > 255)
	    *inter_addr = 0;
	else {
	    *coded = (unsigned char) temp_int;
	    /* convert to long form */
	    for (i = 1; i < 4; i++) {
		*coded <<= 8;
		if ((temp_int = atoi(parse_addr[i])) > 255) {
		    *inter_addr = 0;
		    break;
		} else
		    *coded |= (unsigned char) temp_int;
	    }
	}
    }
}

/********************************************************/
void
write_points_to_socket(plist, npoints, more)
    POINT_PTR plist;
    int       npoints, more;
{
    int       write_size;
    int       buf[2];		/* header output buffer */
    static int tot_points = 0;
    int       num_written, total_to_send;
    char     *out_ptr;

    if (render_start) {
	connect_to_renderer(render_host, render_portnum);
	render_start = 0;
    }
    tot_points += npoints;
    if (!more) {
	fprintf(stderr, "Writing %d points.\n", tot_points);
	tot_points = 0;
    }
    buf[0] = more;
    buf[1] = npoints;
    write(render_sock, buf, 2 * sizeof(int));

    write_size = npoints * sizeof(POINT);

    total_to_send = write_size;
    out_ptr = (char *) plist;

    while (total_to_send > 0) {
	num_written = write(render_sock, out_ptr, total_to_send);
	total_to_send -= num_written;
	if (total_to_send > 0)
	    out_ptr += num_written;
    }
}

/********************************************************/
void
write_polys_to_socket(vertex_list, norm_list, conn_list, nverts, nconn, more)
    VERT_PTR  vertex_list;
    NORM_PTR  norm_list;
    int      *conn_list;
    int       nverts, nconn, more;
{
    int       write_size;
    int       buf[4];		/* header output buffer */
    static int tot_verts = 0;
    int       num_written, total_to_send;
    char     *out_ptr;
    int       i, j;

    if (render_start) {
	connect_to_renderer(render_host, render_portnum);
	render_start = 0;
    }
    tot_verts += nverts;
    if (!more) {
	fprintf(stderr, "Writing %d vertices.\n", tot_verts);
	tot_verts = 0;
    }
/* #define SDEBUG */
#ifdef SDEBUG
    fprintf(stderr, "\nWriting header to socket: %d nverts, %d nconn, %d more \n",
	    nverts, nconn, more);

    for (i = 0; i < 10; i++) {
	fprintf(stderr, "vertex %d: %f,%f,%f \n",
		i, vertex_list[i].x, vertex_list[i].y, vertex_list[i].z);
    }
    if (nconn > 0) {
	j = 0;
	for (i = 0; i < 10; i++) {
	    fprintf(stderr, "\nedge %d: %d,%d,%d \n", i,
		    conn_list[j++], conn_list[j++], conn_list[j++]);
	}
    }
#endif

    buf[0] = more;
    buf[1] = nverts;
    buf[2] = nconn;
    if (MARCH_NORMALS)
	buf[3] = 1;
    else
	buf[3] = 0;
    write(render_sock, buf, 4 * sizeof(int));

    /* write all vertices */
    write_size = nverts * sizeof(VERTEX);

    total_to_send = write_size;
    out_ptr = (char *) vertex_list;

    while (total_to_send > 0) {
	num_written = write(render_sock, out_ptr, total_to_send);
	total_to_send -= num_written;
	if (total_to_send > 0)
	    out_ptr += num_written;
    }

    if (MARCH_NORMALS) {
	write_size = nverts * sizeof(NORMAL);

	total_to_send = write_size;
	out_ptr = (char *) norm_list;

	while (total_to_send > 0) {
	    num_written = write(render_sock, out_ptr, total_to_send);
	    total_to_send -= num_written;
	    if (total_to_send > 0)
		out_ptr += num_written;
	}
    }
    if (nconn > 0) {
	/* write edge list */
	write_size = nconn * sizeof(int);
	total_to_send = write_size;
	out_ptr = (char *) conn_list;

	while (total_to_send > 0) {
	    num_written = write(render_sock, out_ptr, total_to_send);
	    total_to_send -= num_written;
	    if (total_to_send > 0)
		out_ptr += num_written;
	}
    }
}

/********************************************************/

void
catch_sigs()
{
    void      exit_handler();

    signal(SIGHUP, exit_handler);
    signal(SIGQUIT, exit_handler);
    signal(SIGINT, exit_handler);
/*
    signal(SIGBUS, exit_handler);
    signal(SIGSEGV, exit_handler);
    signal(SIGFPE, exit_handler);
*/
    signal(SIGILL, exit_handler);
}

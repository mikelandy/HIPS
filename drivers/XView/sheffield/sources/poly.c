/* Polygon event handling */

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <xdisp.h>

void draw_poly_side();
int vtopoly();
void touch_poly();
void delete_polygon();
void name_poly_callback();


/**********************
 * add_poly_vertex()
 **********************/

void add_poly_vertex(r,c)
  int	r;
  int	c;

/* If no current polygon is defined, create one... else add a new vertex
   to existing partial definition, and draw side */

{
  vertex_t *v;

    if (current_polygon == NULL) {
	current_polygon = (polygon_t *)malloc(sizeof(polygon_t));
	current_polygon->name = NULL;
	current_polygon->nvertex = 0;
	current_polygon->vertices = NULL;
	current_polygon->next = polygon_list;
	polygon_list = current_polygon;
	}
    else {
    	draw_poly_side(current_polygon->vertices->r,
			current_polygon->vertices->c,r,c,lgc);
	}	

    v = (vertex_t *)malloc(sizeof(vertex_t));

    v->r = r;
    v->c = c;

    v->next = current_polygon->vertices;
    current_polygon->vertices = v;
    current_polygon->nvertex++;
}


/**********************
 * add_final_vertex()
 **********************/

void add_final_vertex()

/* Final vertex is actually the first vertex. This routine, called when
   a polygon definition is completed, just draws the final side. */
{
  vertex_t *v = current_polygon->vertices;

    while(v->next != NULL) 
	v = v->next;

    draw_poly_side(current_polygon->vertices->r,
			current_polygon->vertices->c,v->r,v->c,lgc);
    current_polygon = NULL;
}


/**********************
 * draw_poly_side()
 **********************/

void draw_poly_side(r1,c1,r2,c2,gc)
  int	r1;
  int	c1;
  int	r2;
  int	c2;
  GC	gc;
{
  int x1,y1,x2,y2;

    rctoxy(r1,c1,x1,y1);
    rctoxy(r2,c2,x2,y2);

    if (draw) if (XtIsRealized(draw))
    	XDrawLine(dpy,XtWindow(draw),gc,x1,y1,x2,y2);
}


/**********************
 * touch_polys()
 **********************/

void touch_polys()

/* refresh polygons on screen after a redisplay of draw area, if showing */
{
  polygon_t *p = polygon_list;
  vertex_t *v;
  int x,y;

    if (!showing_polys) return;

    while (p != NULL) {
    	v = p->vertices;
	rctoxy(v->r,v->c,x,y);
	if (p->name != NULL)
	    XDrawImageString(dpy,XtWindow(draw),lgc,x,y,
			p->name,strlen(p->name));
	touch_poly(p,lgc);
	p = p->next;
	}
}


/**********************
 * touch_poly()
 **********************/

void touch_poly(p,gc)
  polygon_t 	*p;
  GC		gc;

/* refresh an individual polygon with a given graphics context */
{
  vertex_t *v;

    v = p->vertices;

    while (v->next != NULL) {
	draw_poly_side(v->r,v->c,v->next->r,v->next->c,gc);
	v = v->next;
	}
    draw_poly_side(v->r,v->c,p->vertices->r,p->vertices->c,gc);
}


/**********************
 * delete_poly()
 **********************/

void delete_poly(r,c)
  int	r;
  int	c;
{
    delete_polygon(r,c);
    XClearArea(dpy,XtWindow(draw),0,0,0,0,True);
}



/**********************
 * delete_polygon()
 **********************/

void delete_polygon(r,c)
  int	r;
  int	c;

/* delete polygon if r,c is close enough to a vertex */

{
    polygon_t *p,*pp;
    vertex_t *v,*vv;

    if (!vtopoly(r,c,&p,&v)) return;

    vv = p->vertices;
    while (vv != NULL) {
	v = vv;
	vv = vv->next;
	free(v);
	}

    if (p == polygon_list)
	polygon_list = p->next;
    else {
    	pp = polygon_list;
    	while (pp->next != p)
	    pp = pp->next;
	pp->next = p->next;
	}
    free(p);
}
    

/**********************
 * vtopoly()
 **********************/

int vtopoly(r,c,p,v)
  int		r;
  int		c;
  polygon_t	**p;
  vertex_t	**v;

/* determine if r,c is close enough to a vertex. Return non-zero if so,
   (together with polygon and vertex), zero if not */

{
    polygon_t *pp;
    vertex_t *vv;

    pp = polygon_list;

    while (pp != NULL) {
	vv = pp->vertices;
	while (vv != NULL) {
	    if (abs(vv->r - r) < POLY_GRAVITY_STRENGTH && 
                abs(vv->c - c) < POLY_GRAVITY_STRENGTH) {
		*p = pp;
		*v = vv;
		return(1);
		}
	    vv = vv->next;
	    }
	pp = pp->next;
	}

    return(0);
}



/**********************
 * edit_poly_start()
 **********************/

void edit_poly_start(r,c)
  int	r;
  int	c;

/* edit polygon if r,c is close enough to a vertex */

{
    polygon_t *p;
    vertex_t *v,*vv;

    if (!vtopoly(r,c,&p,&v)) return;

    poly_edit_data.p = p;
    poly_edit_data.v = v;
    in_poly_edit = 1;
}



/**********************
 * edit_poly_track()
 **********************/

void edit_poly_track(r,c)
  int	r;
  int	c;

/* update current vertex and draw new sides */

{
  polygon_t *p = poly_edit_data.p;
  vertex_t *v = poly_edit_data.v;

    touch_poly(p,rgc);
    v->r = r;
    v->c = c;
    touch_poly(p,rgc);
}



/**********************
 * edit_poly_end()
 **********************/

void edit_poly_end(r,c)
  int	r;
  int	c;

/* complete edit & refresh all polygons */

{
    poly_edit_data.v->r = r;
    poly_edit_data.v->c = c;

    XClearArea(dpy,XtWindow(draw),0,0,0,0,True);
    in_poly_edit = 0;
}


/******************
 * clear_polys()
 ******************/

void clear_polys()
{
    polygon_t *p;
    vertex_t *v;

    p = polygon_list;
    while (p != NULL) {
	v = p->vertices;
	delete_polygon(v->r,v->c);
	p = p->next;
	}

    XClearArea(dpy,XtWindow(draw),0,0,0,0,True);
}


/******************
 * name_poly()
 ******************/

void name_poly(r,c)
  int r;
  int c;
{
  polygon_t *p;
  vertex_t *v,*vv;

    if (!vtopoly(r,c,&p,&v)) return;

    poly_edit_data.p = p;
    poly_edit_data.v = v;

    ask("Name Poly","name:",def_poly_name,name_poly_callback);
}


/**********************
 * name_poly_callback()
 **********************/

void name_poly_callback(reply)
  char *reply;
{
    strcpy(def_poly_name,reply);

    if (poly_edit_data.p->name != NULL)
	free(poly_edit_data.p->name);

    poly_edit_data.p->name = (char *)malloc(strlen(reply));
    strcpy(poly_edit_data.p->name,reply);

    touch_polys();
}

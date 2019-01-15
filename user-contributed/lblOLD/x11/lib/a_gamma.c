/* Apply GAMMA . C
#
%
%	Apply gamma to a colormap with 3 channels
%	CMAPnew = celling[255. * (CMAPold / 255. ** 1 / r)]
%
% Author:	Jin Guojun
*/

#include "panel.h"
#include "math.h"	/* for pow()	*/

apply_3c_gamma(img, org_cmap, new_gamma)
register Image	*img;
cmap_t	**org_cmap;
double	new_gamma;
{
static double	last_gamma;
static	cmap_t	*gammap;
register int	i, j;
double	gamma = img->gamma ? 1.0 / img->gamma : 1.0;
	if (new_gamma)	gamma /= new_gamma;
	if (!gammap)
	    gammap = (cmap_t *)NZALLOC(256, sizeof(cmap_t), "gammap");
	if (gamma != 1.0)	{
	    if (last_gamma != gamma)
		for (i=256; i--;)
			gammap[i] = (0.5 + 255.0 * pow(i / 255.0, gamma));
	    last_gamma = gamma;
	    for (i=3; i--;)
		for (j=img->cmaplen; j--;)
		    img->in_cmap[i][j] = gammap[org_cmap[i][j]];
	} else if (img->in_cmap[0] != org_cmap[0])
		memcpy(img->in_cmap[0], org_cmap[0], img->cmaplen * 3);
}


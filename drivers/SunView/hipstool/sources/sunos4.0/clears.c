/* clears.c
 * Max Rible
 *
 * Clear procedures for hipstool functions.
 * All clear procedures take no arguments and return void.
 */

#include "hipstool.h"

void
no_clear()
{
}

void
box_clear()
{
    save_menu_funcs[SAVE_BOX_SUBIMAGE].active = 0;

    update_save_menu();
}

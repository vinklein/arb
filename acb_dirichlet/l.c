/*
    Copyright (C) 2016 Fredrik Johansson
    Copyright (C) 2016 Pascal Molin

    This file is part of Arb.

    Arb is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <http://www.gnu.org/licenses/>.
*/

#include "acb_dirichlet.h"

void
acb_dirichlet_l(acb_t res, const acb_t s,
    const dirichlet_group_t G, const dirichlet_char_t chi, slong prec)
{
    /* this cutoff is probably too conservative when q is large */
    if (arf_cmp_d(arb_midref(acb_realref(s)), 8 + 0.5 * prec / log(prec)) >= 0)
    {
        acb_dirichlet_l_euler_product(res, s, G, chi, prec);
    }
    else
    {
        acb_dirichlet_l_hurwitz(res, s, G, chi, prec);
    }
}

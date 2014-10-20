/*=============================================================================

    This file is part of ARB.

    ARB is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    ARB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ARB; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

=============================================================================*/
/******************************************************************************

    Copyright (C) 2014 Fredrik Johansson

******************************************************************************/

#include "acb_modular.h"

static void
acb_mul_4th_root(acb_t y, const acb_t x, int r, long prec)
{
    r &= 7;

    if (r == 0)
    {
        acb_set(y, x);
    }
    else if (r == 4)
    {
        acb_neg(y, x);
    }
    else if (r == 2)
    {
        acb_mul_onei(y, x);
    }
    else if (r == 6)
    {
        acb_mul_onei(y, x);
        acb_neg(y, y);
    }
    else
    {
        fmpq_t t;
        fmpq_init(t);
        fmpq_set_si(t, r, 4);
        arb_sin_cos_pi_fmpq(acb_imagref(y), acb_realref(y), t, prec);
        acb_mul(y, y, x, prec);
        fmpq_clear(t);
    }
}

void
acb_modular_theta_1234(acb_t theta1, acb_t theta2,
    acb_t theta3, acb_t theta4, const acb_t z, const acb_t tau,
    long prec)
{
    fmpq_t t;
    psl2z_t g;
    arf_t one_minus_eps;
    acb_t z_prime, tau_prime, q, q4, w, A, B;
    acb_struct thetas[4];
    int w_is_unit, R[4], S[4], C;

    psl2z_init(g);
    fmpq_init(t);
    arf_init(one_minus_eps);
    acb_init(z_prime);
    acb_init(tau_prime);
    acb_init(q);
    acb_init(q4);
    acb_init(w);
    acb_init(thetas + 0);
    acb_init(thetas + 1);
    acb_init(thetas + 2);
    acb_init(thetas + 3);
    acb_init(A);
    acb_init(B);

    /* move tau to the fundamental domain */
    arf_set_ui_2exp_si(one_minus_eps, 63, -6);
    acb_modular_fundamental_domain_approx(tau_prime, g, tau,
        one_minus_eps, prec);

    /* compute transformation parameters */
    acb_modular_theta_transform(R, S, &C, g);

    if (C == 0)
    {
        acb_set(z_prime, z);
    }
    else
    {
        /* B = 1/(c*tau+d) (temporarily) */
        acb_mul_fmpz(B, tau, &g->c, prec);
        acb_add_fmpz(B, B, &g->d, prec);
        acb_inv(B, B, prec);

        /* -z/(c*tau+d) */
        acb_mul(z_prime, z, B, prec);
        acb_neg(z_prime, z_prime);

        /* A = sqrt(i/(c*tau+d)) */
        acb_mul_onei(A, B);
        acb_sqrt(A, A, prec);

        /* B = exp(-pi i c z^2/(c*tau+d)) */
        if (acb_is_zero(z))
        {
            acb_one(B);
        }
        else
        {
            acb_mul(B, z_prime, z, prec);
            acb_mul_fmpz(B, B, &g->c, prec);
            acb_exp_pi_i(B, B, prec);
        }
    }

    /* compute q_{1/4}, q */
    acb_mul_2exp_si(q4, tau_prime, -2);
    acb_exp_pi_i(q4, q4, prec);
    acb_pow_ui(q, q4, 4, prec);

    /* compute w */
    acb_exp_pi_i(w, z_prime, prec);
    w_is_unit = arb_is_zero(acb_imagref(z_prime));

    /* evaluate theta functions of transformed variables */
    acb_modular_theta_1234_sum(thetas + 0, thetas + 1, thetas + 2, thetas + 3,
        w, w_is_unit, q, 1, prec);
    acb_mul(thetas + 0, thetas + 0, q4, prec);
    acb_mul(thetas + 1, thetas + 1, q4, prec);

    /* multiply by roots of unity */
    acb_mul_4th_root(theta1, thetas + S[0], R[0], prec);
    acb_mul_4th_root(theta2, thetas + S[1], R[1], prec);
    acb_mul_4th_root(theta3, thetas + S[2], R[2], prec);
    acb_mul_4th_root(theta4, thetas + S[3], R[3], prec);

    if (C != 0)
    {
        acb_mul(A, A, B, prec);
        acb_mul(theta1, theta1, A, prec);
        acb_mul(theta2, theta2, A, prec);
        acb_mul(theta3, theta3, A, prec);
        acb_mul(theta4, theta4, A, prec);
    }

    psl2z_clear(g);
    fmpq_clear(t);
    arf_clear(one_minus_eps);
    acb_clear(z_prime);
    acb_clear(tau_prime);
    acb_clear(q);
    acb_clear(q4);
    acb_clear(w);
    acb_clear(thetas + 0);
    acb_clear(thetas + 1);
    acb_clear(thetas + 2);
    acb_clear(thetas + 3);
    acb_clear(A);
    acb_clear(B);
}

void
acb_modular_theta_1234_notransform(acb_t theta1, acb_t theta2,
    acb_t theta3, acb_t theta4, const acb_t z, const acb_t tau,
    long prec)
{
    acb_t q, q4, w;
    int w_is_unit;

    acb_init(q);
    acb_init(q4);
    acb_init(w);

    /* compute q_{1/4}, q */
    acb_mul_2exp_si(q4, tau, -2);
    acb_exp_pi_i(q4, q4, prec);
    acb_pow_ui(q, q4, 4, prec);

    /* compute w */
    acb_exp_pi_i(w, z, prec);
    w_is_unit = arb_is_zero(acb_imagref(z));

    /* evaluate theta functions */
    acb_modular_theta_1234_sum(theta1, theta2, theta3, theta4,
        w, w_is_unit, q, 1, prec);
    acb_mul(theta1, theta1, q4, prec);
    acb_mul(theta2, theta2, q4, prec);

    acb_clear(q);
    acb_clear(q4);
    acb_clear(w);
}

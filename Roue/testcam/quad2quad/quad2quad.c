/*
Copyright (C) 2020 Antoine Rousseau

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include "m_pd.h"

#include <stdio.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>

typedef struct _quad2quad
{
    t_object x_obj;
    gsl_vector *source;
    gsl_vector *destination;
    gsl_matrix *projection_matrix;
} t_quad2quad;

t_class *quad2quad_class;

void quad2quad_setup(void);

static void quad2quad_list(t_quad2quad *x,t_symbol *s, int argc, t_atom *argv)
{
    double x_data[3];
    double y_data[3];
    t_atom at[2];
    gsl_vector_view xvec, yvec;
    //gsl_matrix_view m;
    //post("quad2quad_list argc=%d", argc);
    if(argc != 2) pd_error(x, "quad2quad: wrong arguments count");

    x_data[0] = atom_getfloat(argv);
    x_data[1] = atom_getfloat(argv + 1);
    x_data[2] = 1.0;
    xvec = gsl_vector_view_array(x_data, 3);
    yvec = gsl_vector_view_array(y_data, 3);
    //m = gsl_matrix_vew(
    gsl_blas_dgemv(CblasNoTrans, 1.0, x->projection_matrix, &xvec.vector, 0.0, &yvec.vector);
    SETFLOAT(&at[0], y_data[0]/y_data[2]);
    SETFLOAT(&at[1], y_data[1]/y_data[2]);
    outlet_list(x->x_obj.ob_outlet, &s_list, 2, at);
}

static void quad2quad_source(t_quad2quad *x,t_symbol *s, int argc, t_atom *argv)
{
    //post("quad2quad_source argc=%d", argc);
    if(argc == 3) {
        int index = atom_getfloat(argv);
        if(index < 0) index = 0;
        if(index > 3) index = 3;
        gsl_vector_set(x->source, index * 2, atom_getfloat(argv + 1));
        gsl_vector_set(x->source, index * 2 + 1, atom_getfloat(argv + 2));
    } else if(argc == 8) {
        int i;
        for(i = 0; i < 8; i++) gsl_vector_set(x->source, i, atom_getfloat(argv++));
    } else pd_error(x, "quad2quad source: wrong arguments count");
}

static void quad2quad_dest(t_quad2quad *x,t_symbol *s, int argc, t_atom *argv)
{
    //post("quad2quad_dest argc=%d", argc);
    if(argc == 3) {
        int index = atom_getfloat(argv);
        if(index < 0) index = 0;
        if(index > 3) index = 3;
        gsl_vector_set(x->destination, index * 2, atom_getfloat(argv + 1));
        gsl_vector_set(x->destination, index * 2 + 1, atom_getfloat(argv + 2));
    } else if(argc == 8) {
        int i;
        for(i = 0; i < 8; i++) gsl_vector_set(x->destination, i, atom_getfloat(argv++));
    } else pd_error(x, "quad2quad destination: wrong arguments count");
}

static void quad2quad_bang(t_quad2quad *x)
{
    //post("quad2quad_bang");
    int i, j, s, status;
    gsl_matrix *A = gsl_matrix_alloc(8, 8);
    gsl_matrix *invA = gsl_matrix_alloc(8, 8);
    gsl_permutation *p = gsl_permutation_alloc(8);
    gsl_vector_view outvec = gsl_vector_view_array(x->projection_matrix->data, 8);
    
    for(i = 0; i < 4; i++)
    {
        gsl_matrix_set(A, i * 2, 0, gsl_vector_get(x->source, i * 2));
        gsl_matrix_set(A, i * 2, 1, gsl_vector_get(x->source, i * 2 + 1));
        gsl_matrix_set(A, i * 2, 2, 1.0);
        gsl_matrix_set(A, i * 2, 3, 0.0);
        gsl_matrix_set(A, i * 2, 4, 0.0);
        gsl_matrix_set(A, i * 2, 5, 0.0);
        gsl_matrix_set(A, i * 2, 6, -1.0 * gsl_vector_get(x->source, i * 2) * gsl_vector_get(x->destination, i * 2));
        gsl_matrix_set(A, i * 2, 7, -1.0 * gsl_vector_get(x->source, i * 2 + 1) * gsl_vector_get(x->destination, i * 2));

        gsl_matrix_set(A, i * 2 + 1, 0, 0.0);
        gsl_matrix_set(A, i * 2 + 1, 1, 0.0);
        gsl_matrix_set(A, i * 2 + 1, 2, 0.0);
        gsl_matrix_set(A, i * 2 + 1, 3, gsl_vector_get(x->source, i * 2));
        gsl_matrix_set(A, i * 2 + 1, 4, gsl_vector_get(x->source, i * 2 + 1));
        gsl_matrix_set(A, i * 2 + 1, 5, 1.0);
        gsl_matrix_set(A, i * 2 + 1, 6, -1.0 * gsl_vector_get(x->source, i * 2) * gsl_vector_get(x->destination, i * 2 + 1));
        gsl_matrix_set(A, i * 2 + 1, 7, -1.0 * gsl_vector_get(x->source, i * 2 + 1) * gsl_vector_get(x->destination, i * 2 + 1));
    }

#ifdef Q2Q_DEBUG
    printf("\nsource vector:\n");
    for(i = 0; i < 8; i++)
            printf(i==7?"%f\n":"%f ", gsl_vector_get(x->source, i));

    printf("\ndestination vector:\n");
    for(i = 0; i < 8; i++)
            printf(i==7?"%f\n":"%f ", gsl_vector_get(x->destination, i));

    printf("\nA matrix:\n");
    for(i = 0; i < 8; i++)
        for(j = 0; j < 8; j++)
            printf(j==7?"%f\n":"%f ", gsl_matrix_get(A, i, j));
#endif

    status = gsl_linalg_LU_decomp (A, p, &s);
    if(status) {
        pd_error(x, "gsl_linalg_LU_decomp %s", gsl_strerror(status));
        return;
    }
    
    /*status = gsl_linalg_LU_invert (A, p, invA);
    if(status) {
        pd_error(x, "gsl_linalg_LU_invert %s", gsl_strerror(status));
        return;
    }
    
    status = gsl_blas_dgemv(CblasNoTrans, 1.0, invA, x->destination, 0.0, &outvec.vector);
    if(status) {
        pd_error(x, "gsl_blas_dgemv %s", gsl_strerror(status));
        return;
    }*/
    status = gsl_linalg_LU_solve(A, p, x->destination, &outvec.vector);
    if(status) {
        pd_error(x, "gsl_linalg_LU_solve %s", gsl_strerror(status));
        return;
    }

    gsl_matrix_set(x->projection_matrix, 2, 2, 1.0);

#ifdef Q2Q_DEBUG
    printf("\ninvA matrix:\n");
    for(i = 0; i < 8; i++)
        for(j = 0; j < 8; j++)
            printf(j==7?"%f\n":"%f ", gsl_matrix_get(invA, i, j));

    printf("\nout vector:\n");
    for(i = 0; i < 8; i++)
            printf(i==7?"%f\n":"%f ", gsl_vector_get(&outvec.vector, i));

    printf("\nprojection matrix:\n");
    for(i = 0; i < 3; i++)
        for(j = 0; j < 3; j++)
            printf(j==2?"%f\n":"%f ", gsl_matrix_get(x->projection_matrix, i, j));
#endif

    gsl_matrix_free(A);
    gsl_matrix_free(invA);
    gsl_permutation_free (p);

}

static void *quad2quad_new(void)
{
    t_quad2quad *x = (t_quad2quad *)pd_new(quad2quad_class);
    outlet_new(&x->x_obj, 0);
    x->projection_matrix = gsl_matrix_alloc(3, 3);
    x->source = gsl_vector_alloc(8);
    x->destination = gsl_vector_alloc(8);
    return (void *)x;
}

static void quad2quad_free(t_quad2quad *x)
{
    gsl_matrix_free(x->projection_matrix);
    gsl_vector_free(x->source);
    gsl_vector_free(x->destination);
}

void quad2quad_setup(void)
{
    quad2quad_class = class_new(gensym("quad2quad"),(t_newmethod)quad2quad_new,
                              (t_method)quad2quad_free, sizeof(t_quad2quad), 0, 0);

    class_addlist(quad2quad_class, quad2quad_list);
    class_addbang(quad2quad_class, quad2quad_bang);
    class_addmethod(quad2quad_class, (t_method)quad2quad_source, gensym("source"), A_GIMME, 0);
    class_addmethod(quad2quad_class, (t_method)quad2quad_dest, gensym("destination"), A_GIMME, 0);
}    




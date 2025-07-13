#include "common.h"

void set_matrix(fixed dst[restrict], const fixed src[restrict]) {
  memcpy32(dst, (void *)src, 12);
  /* for (int i=0; i<12; i++){
    dst[i]=src[i];
  } */
}

void set_matrix_sp(int a0, int a1, int a2, int a3, fixed dst[restrict], const fixed src[restrict]) {
  if (a0) {
    memcpy32(dst, (void *)src, 3);
  }
  if (a1) {
    memcpy32(dst + 3, (void *)(src + 3), 3);
  }
  if (a2) {
    memcpy32(dst + 6, (void *)(src + 6), 3);
  }
  if (a3) {
    memcpy32(dst + 9, (void *)(src + 9), 3);
  }
}

void translate_matrix(fixed xpos, fixed ypos, fixed zpos, fixed matrix[restrict]) {
  matrix[0] += xpos;
  matrix[4] += ypos;
  matrix[8] += zpos;
}

void scale_matrix(fixed xscale, fixed yscale, fixed zscale, fixed matrix[restrict]) {
  for (int i = 0; i <= 3; i++) {
    if (xscale) {
      matrix[i] = fp_mul(matrix[i], xscale);
    }
    if (yscale) {
      matrix[i + 4] = fp_mul(matrix[i + 4], yscale);
    }
    if (zscale) {
      matrix[i + 8] = fp_mul(matrix[i + 8], zscale);
    }
  }
}

RAM_CODE void rotate_matrix(u16 xrot, u16 yrot, u16 zrot, fixed matrix[restrict], int view) {
  fixed cxr = lu_cos(xrot); // .16
  fixed sxr = lu_sin(xrot);
  fixed cyr = lu_cos(yrot);
  fixed syr = lu_sin(yrot);
  fixed czr = lu_cos(zrot);
  fixed szr = lu_sin(zrot);
  
  for (int i = 0; i <= 3; i++) {
    fixed *xr = &matrix[i];
    fixed *yr = &matrix[i + 4];
    fixed *zr = &matrix[i + 8];
    
    if (view) {
      rotate_matrix_vertex_view(cxr, cyr, czr, sxr, syr, szr, xr, yr, zr);
    } else {
      rotate_matrix_vertex(cxr, cyr, czr, sxr, syr, szr, xr, yr, zr);
    }
  }
}

// rotate the matrix in local space

void rotate_matrix_vertex(fixed cxr, fixed cyr, fixed czr, fixed sxr, fixed syr, fixed szr, fixed *xr, fixed *yr, fixed *zr) {
  fixed xt, yt, zt;
  fixed xs = *xr;
  fixed ys = *yr;
  fixed zs = *zr;
  yt = fp_mul(ys, cxr) - fp_mul(zs, sxr); // x
  zt = fp_mul(ys, sxr) + fp_mul(zs, cxr);
  ys = yt;
  zs = zt;
  zt = fp_mul(zs, cyr) - fp_mul(xs, syr); // y
  xt = fp_mul(zs, syr) + fp_mul(xs, cyr);
  zs = zt;
  xs = xt;
  xt = fp_mul(xs, czr) - fp_mul(ys, szr); // z
  yt = fp_mul(xs, szr) + fp_mul(ys, czr);
  xs = xt;
  ys = yt;
  *xr = xs;
  *yr = ys;
  *zr = zs;
}

// rotate the matrix against the camera

RAM_CODE void rotate_matrix_vertex_view(fixed cxr, fixed cyr, fixed czr, fixed sxr, fixed syr, fixed szr, fixed *xr, fixed *yr, fixed *zr) {
  fixed xt, yt, zt;
  fixed xs = *xr;
  fixed ys = *yr;
  fixed zs = *zr;
  zt = fp_mul(zs, cyr) - fp_mul(xs, syr); // x
  xt = fp_mul(zs, syr) + fp_mul(xs, cyr);
  zs = zt;
  xs = xt;
  yt = fp_mul(ys, cxr) - fp_mul(zs, sxr); // y
  zt = fp_mul(ys, sxr) + fp_mul(zs, cxr);
  ys = yt;
  zs = zt;
  xt = fp_mul(xs, czr) - fp_mul(ys, szr); // z
  yt = fp_mul(xs, szr) + fp_mul(ys, czr);
  xs = xt;
  ys = yt;
  *xr = xs;
  *yr = ys;
  *zr = zs;
}
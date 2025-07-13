void set_matrix(fixed dst[], const fixed src[]);
void set_matrix_sp(int a0, int a1, int a2, int a3, fixed dst[], const fixed src[]);
void translate_matrix(fixed xpos, fixed ypos, fixed zpos, fixed matrix[]);
void scale_matrix(fixed xscale, fixed yscale, fixed zscale, fixed matrix[]);
void rotate_matrix(u16 xrot, u16 yrot, u16 zrot, fixed matrix[], int view);
void rotate_matrix_vertex(fixed cxr, fixed cyr, fixed czr, fixed sxr, fixed syr, fixed szr, fixed *xr, fixed *yr, fixed *zr);
void rotate_matrix_vertex_view(fixed cxr, fixed cyr, fixed czr, fixed sxr, fixed syr, fixed szr, fixed *xr, fixed *yr, fixed *zr);
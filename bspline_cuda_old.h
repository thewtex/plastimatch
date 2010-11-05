    void bspline_cuda_score_i_mse(Bspline_parms* parms,
			  			Bspline_state *bst,
						Bspline_xform* bxf,
						Volume* fixed,
						Volume* moving,
						Volume* moving_grad,
						Dev_Pointers_Bspline* dev_ptrs);


    void bspline_cuda_score_h_mse(Bspline_parms* parms,
			  			Bspline_state *bst,
						Bspline_xform* bxf,
						Volume* fixed,
						Volume* moving,
						Volume* moving_grad,
						Dev_Pointers_Bspline* dev_ptrs);


    void bspline_cuda_score_g_mse(
				  Bspline_parms *parms, 
				  Bspline_state *bst,
				  Bspline_xform* bxf, 
				  Volume *fixed, 
				  Volume *moving, 
				  Volume *moving_grad);

    void bspline_cuda_score_f_mse(
				  Bspline_parms *parms, 
				  Bspline_state *bst,
				  Bspline_xform* bxf, 
				  Volume *fixed, 
				  Volume *moving, 
				  Volume *moving_grad);

    void bspline_cuda_score_e_mse_v2(
				     Bspline_parms *parms, 
				     Bspline_state *bst,
				     Bspline_xform* bxf, 
				     Volume *fixed, 
				     Volume *moving, 
				     Volume *moving_grad);

    void bspline_cuda_score_e_mse(
				  Bspline_parms *parms, 
				  Bspline_state *bst,
				  Bspline_xform* bxf, 
				  Volume *fixed, 
				  Volume *moving, 
				  Volume *moving_grad);

    void bspline_cuda_score_d_mse(
				  Bspline_parms *parms, 
				  Bspline_state *bst,
				  Bspline_xform* bxf, 
				  Volume *fixed, 
				  Volume *moving, 
				  Volume *moving_grad);

    void bspline_cuda_score_c_mse(
				  Bspline_parms *parms, 
				  Bspline_state *bst,
				  Bspline_xform* bxf, 
				  Volume *fixed, 
				  Volume *moving, 
				  Volume *moving_grad);



    // Initialize the GPU to execute bspline_cuda_score_i_mse().
    void bspline_cuda_initialize_i(
				   Dev_Pointers_Bspline *dev_ptrs,
				   Volume *fixed,
				   Volume *moving,
				   Volume *moving_grad,
				   Bspline_xform *bxf,
				   Bspline_parms *parms);

    // Initialize the GPU to execute bspline_cuda_score_h_mse().
    void bspline_cuda_initialize_h(
				   Dev_Pointers_Bspline *dev_ptrs,
				   Volume *fixed,
				   Volume *moving,
				   Volume *moving_grad,
				   Bspline_xform *bxf,
				   Bspline_parms *parms);
    //
    // Initialize the GPU to execute bspline_cuda_score_g_mse().
    void bspline_cuda_initialize_g(
				   Volume *fixed,
				   Volume *moving,
				   Volume *moving_grad,
				   Bspline_xform *bxf,
				   Bspline_parms *parms);

    // Initialize the GPU to execute bspline_cuda_score_f_mse().
    void bspline_cuda_initialize_f(
				   Volume *fixed,
				   Volume *moving,
				   Volume *moving_grad,
				   Bspline_xform *bxf,
				   Bspline_parms *parms);

    // Initialize the GPU to execute bspline_cuda_score_e_mse_v2().
    void bspline_cuda_initialize_e_v2(
				      Volume *fixed,
				      Volume *moving,
				      Volume *moving_grad,
				      Bspline_xform *bxf,
				      Bspline_parms *parms);

    // Initialize the GPU to execute bspline_cuda_score_e_mse().
    void bspline_cuda_initialize_e(
				   Volume *fixed,
				   Volume *moving,
				   Volume *moving_grad,
				   Bspline_xform *bxf,
				   Bspline_parms *parms);

    // Initialize the GPU to execute bspline_cuda_score_d_mse().
    void bspline_cuda_initialize_d(
				   Volume *fixed,
				   Volume *moving,
				   Volume *moving_grad,
				   Bspline_xform *bxf,
				   Bspline_parms *parms);

    // Allocate memory on the GPU and copy all necessary data to the GPU.
    void bspline_cuda_initialize(
				 Volume *fixed,
				 Volume *moving,
				 Volume *moving_grad,
				 Bspline_xform *bxf,
				 Bspline_parms *parms);

    void bspline_cuda_copy_coeff_lut(
        Bspline_xform *bxf
    );

    void bspline_cuda_copy_grad_to_host(
        float* host_grad
    );

    void bspline_cuda_i_stage_1(
		   Volume* fixed,
		   Volume* moving,
		   Volume* moving_grad,
		   Bspline_xform* bxf,
		   Bspline_parms* parms,
		   Dev_Pointers_Bspline* dev_ptrs);


    void bspline_cuda_h_stage_1(
		   Volume* fixed,
		   Volume* moving,
		   Volume* moving_grad,
		   Bspline_xform* bxf,
		   Bspline_parms* parms,
		   Dev_Pointers_Bspline* dev_ptrs);


    void bspline_cuda_calculate_run_kernels_g(
					      Volume *fixed,
					      Volume *moving,
					      Volume *moving_grad,
					      Bspline_xform *bxf,
					      Bspline_parms *parms,
					      int run_low_mem_version, 
					      int debug);

    void bspline_cuda_calculate_run_kernels_f(
					      Volume *fixed,
					      Volume *moving,
					      Volume *moving_grad,
					      Bspline_xform *bxf,
					      Bspline_parms *parms);

    void bspline_cuda_calculate_score_e(
					Volume *fixed,
					Volume *moving,
					Volume *moving_grad,
					Bspline_xform *bxf,
					Bspline_parms *parms);

    void bspline_cuda_run_kernels_e_v2(
				       Volume *fixed,
				       Volume *moving,
				       Volume *moving_grad,
				       Bspline_xform *bxf,
				       Bspline_parms *parms,
				       int sidx0,
				       int sidx1,
				       int sidx2);

    void bspline_cuda_run_kernels_e(
				    Volume *fixed,
				    Volume *moving,
				    Volume *moving_grad,
				    Bspline_xform *bxf,
				    Bspline_parms *parms,
				    int sidx0,
				    int sidx1,
				    int sidx2);

    void bspline_cuda_run_kernels_d(
				    Volume *fixed,
				    Volume *moving,
				    Volume *moving_grad,
				    Bspline_xform *bxf,
				    Bspline_parms *parms,
				    int p0,
				    int p1,
				    int p2);

    void bspline_cuda_run_kernels_c(
				    Volume *fixed,
				    Volume *moving,
				    Volume *moving_grad,
				    Bspline_xform *bxf,
				    Bspline_parms *parms,
				    float *host_diff,
				    float *host_dc_dv_x,
				    float *host_dc_dv_y,
				    float *host_dc_dv_z,
				    float *host_score);

    void bspline_cuda_clear_score();

    void bspline_cuda_clear_grad();

    void bspline_cuda_clear_dc_dv();

    void bspline_cuda_compute_score_d(
				      int *vox_per_rgn,
				      int *volume_dim,
				      float *host_score);


    void bspline_cuda_h_stage_2(
			Bspline_parms* parms,
			Bspline_xform* bxf,
			Volume* fixed,
			int* vox_per_rgn,
			int* volume_dim,
			float* host_score,
			float* host_grad,
			float* host_grad_mean,
			float* host_grad_norm,
			Dev_Pointers_Bspline* dev_ptrs);


    void bspline_cuda_final_steps_f(
				    Bspline_parms* parms, 
				    Bspline_xform* bxf,
				    Volume *fixed,
				    int   *vox_per_rgn,
				    int   *volume_dim,
				    float *host_score,
				    float *host_grad,
				    float *host_grad_mean,
				    float *host_grad_norm);

    void bspline_cuda_final_steps_e_v2(
				       Bspline_parms* parms, 
				       Bspline_xform* bxf,
				       Volume *fixed,
				       int   *vox_per_rgn,
				       int   *volume_dim,
				       float *host_score,
				       float *host_grad,
				       float *host_grad_mean,
				       float *host_grad_norm);

    void bspline_cuda_final_steps_e(
				    Bspline_parms* parms, 
				    Bspline_xform* bxf,
				    Volume *fixed,
				    int   *vox_per_rgn,
				    int   *volume_dim,
				    float *host_score,
				    float *host_grad,
				    float *host_grad_mean,
				    float *host_grad_norm);

    void bspline_cuda_final_steps_d(
				    Bspline_parms* parms, 
				    Bspline_xform* bxf,
				    Volume *fixed,
				    int   *vox_per_rgn,
				    int   *volume_dim,
				    float *host_score,
				    float *host_grad,
				    float *host_grad_mean,
				    float *host_grad_norm);

    void bspline_cuda_calculate_gradient_c (
					    Bspline_parms* parms, 
					    Bspline_state* bst,
					    Bspline_xform* bxf,
					    Volume *fixed,
					    float *host_grad_norm,
					    float *host_grad_mean);


    void bspline_cuda_clean_up_i(Dev_Pointers_Bspline* dev_ptrs);

    void bspline_cuda_clean_up_h(Dev_Pointers_Bspline* dev_ptrs);

    void bspline_cuda_clean_up_g();

    void bspline_cuda_clean_up_f();

    void bspline_cuda_clean_up_d();

    void bspline_cuda_clean_up();

    void CUDA_deinterleave( int num_values, float* input, float* out_x, float* out_y, float* out_z);

    void CUDA_pad_64( float** input, int* vol_dims, int* tile_dims);

    void CUDA_pad( float** input, int* vol_dims, int* tile_dims);

    void CUDA_bspline_mse_condense_64( Dev_Pointers_Bspline* dev_ptrs, int* vox_per_rgn, int num_tiles);

    void CUDA_bspline_mse_condense( Dev_Pointers_Bspline* dev_ptrs, int* vox_per_rgn, int num_tiles);


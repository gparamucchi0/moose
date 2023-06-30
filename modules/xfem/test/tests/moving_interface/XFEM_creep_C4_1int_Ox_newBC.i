#test for implementing regular creep model on 1 side of the interface and not the other

[GlobalParams]
    order = FIRST
    family = LAGRANGE
    temperature = 1473.15
    displacements = 'disp_x disp_y'
[]

[Mesh]
    #[gmg]
    #    type = CartesianMeshGenerator
    #    dim = 2
    #    dx = '0.0006'
    #    dy = '0.0006'
    #    ix = '151'
    #    iy = '5'
    #[]

    [gmg]
        type = FileMeshGenerator
        file = cladding_new_bc.e
    []

    [top_left_node]
        type = ExtraNodesetGenerator
        new_boundary = 'top_left'
        coord = '0.005 6e-4'
        input = gmg
    []

    [bottom_left_node]
        type = ExtraNodesetGenerator
        new_boundary = 'bottom_left'
        coord = '0.005 0'
        input = top_left_node
    []
[]

[XFEM]
    qrule = volfrac
    output_cut_plane = true
[]

[UserObjects]
    [value_uo_ox_a]
        type = NodeValueAtXFEMInterface
        variable = 'u'
        interface_mesh_cut_userobject = 'moving_line_segment_ox_a'
        execute_on = 'nonlinear'
        level_set_var = ls_ox_a 
    []
    [moving_line_segment_ox_a]
        type = InterfaceMeshCut2DUserObjectZr
        mesh_file = different_but_why.e
        interface_velocity_function = '-3e-6'
        heal_always = true
        is_C4 = true 
        oxa_interface = true
    []
[]

[Functions]
    [p]
        type = PiecewiseLinear
        x = '0 6e-4'
        y = '1 1'
    []
[]

[Variables]
    [u]
    []
[]

[ICs]
    [ic_u]
        type = FunctionIC
        variable = u
        function = 'if (x<0.005590, 0.0075,0.45)'
    []
[]

[AuxVariables]
    [ls_ox_a]
        order = FIRST
        family = LAGRANGE
    []
[]

[Constraints]
    [u_constraint_ox_a]
        type = XFEMEqualValueAtInterfaceC4aox
        geometric_cut_userobject = 'moving_line_segment_ox_a'
        use_displaced_mesh = false
        variable = u
        alpha = 1e5
    []
    [disp_x_constraint]
        type = XFEMSingleVariableConstraint
        geometric_cut_userobject = 'moving_line_segment_ox_a'
        use_displaced_mesh = false
        variable = disp_x
        alpha = 1e5
    []
    [disp_y_constraint]
        type = XFEMSingleVariableConstraint
        geometric_cut_userobject = 'moving_line_segment_ox_a'
        use_displaced_mesh = false
        variable = disp_y
        alpha = 1e5
    []
[]

[Kernels]
    [diff]
        type = MatDiffusion
        variable = u
        diffusivity = 'diffusion_coefficient'
    []
    [time]
        type = TimeDerivative
        variable = u
    []
[]

[Modules/TensorMechanics/Master]
    [all]
        strain = FINITE
        #use_automatic_differentiation = true
        incremental = true
        add_variables = true
        generate_output = 'stress_xx stress_yy stress_xy strain_yy strain_xy strain_xx creep_strain_xx creep_strain_yy creep_strain_xy'
    []
[]

[AuxKernels]
    [ls_ox_a]
        type = MeshCutLevelSetAux
        mesh_cut_user_object = 'moving_line_segment_ox_a'
        variable = ls_ox_a
    []
[]

[Materials]
    [diffusivity_alpha]
        type = C4DiffusionCoefAlpha
        prop_names = alpha_diffusion_coefficient
    []
    [diffusivity_oxide]
        type = GenericConstantMaterial
        prop_names = oxide_diffusion_coefficient
        prop_values = 10e-6
    []
    [diff_combined]
        type = LevelSetBiMaterialReal
        levelset_negative_base = 'alpha'
        levelset_positive_base = 'oxide'
        level_set_var = ls_ox_a
        prop_name = diffusion_coefficient
        outputs = exodus
    []


    [elasticity_tensor_alpha]
        type = ComputeIsotropicElasticityTensor
        base_name = 'alpha'
        youngs_modulus = 1.01e11
        poissons_ratio = 0.33
    []
    [elasticity_tensor_oxide]
        type = ComputeIsotropicElasticityTensor
        base_name = 'oxide'
        youngs_modulus = 1.75e11
        poissons_ratio = 0.27
    []
    [combined_elasticity_tensor]
        type = LevelSetBiMaterialRankFour
        level_set_var = ls_ox_a
        levelset_negative_base = 'alpha'
        levelset_positive_base = 'oxide'
        prop_name = Jacobian_mult #elasticity_tensor
    []

    [radial_return_stress]
        type = ComputeMultipleInelasticStress
        base_name = 'alpha'
        inelastic_models = 'power_law_creep_a'
        tangent_operator = elastic
    []
    [power_law_creep_a]
        type = PowerLawCreepStressUpdate
        coefficient = 1.2e-25
        n_exponent = 5.0
        activation_energy = 2.5e5
    []
    [stress_oxide]
        type = ComputeFiniteStrainElasticStress
        base_name = 'oxide'
    []
    [combined_stress]
        type = LevelSetBiMaterialRankTwo
        levelset_negative_base = 'alpha'
        levelset_positive_base = 'oxide'
        level_set_var = ls_ox_a
        prop_name = stress
    []

    [strain_alpha]
        type = ComputeFiniteStrain
        base_name = 'alpha'
    []
    [strain_oxide]
        type = ComputeFiniteStrain
        base_name = 'oxide'
    []

[]

[BCs]
    [left_u]
        type = NeumannBC
        variable = u
        value = 0
        boundary = 2
    []

    [right_u]
        type = DirichletBCRightC4Zr
        variable = u
        boundary = 4
    []
    [bottom_left_disp_x]
        type = DirichletBC
        variable = disp_x
        boundary = bottom_left
        value = 0.0
    []
    [bottom_disp_y]
        type = DirichletBC
        variable = disp_y
        boundary = 3
        value = 0.0
    []
    [top_disp_y]
        type = DirichletBC
        variable = disp_y
        boundary = 1
        value = 0.0
    []
    [top_left_disp_x]
        type = DirichletBC
        variable = disp_x
        boundary = top_left
        value = 0.0
    []
    
    [left_pressure]
        type = Pressure
        variable = disp_x
        boundary = 2
        factor = 10e6
        function = p   
    []
[]

[Executioner]
    type = Transient
    solve_type = 'PJFNK'
    
    line_search ='none'
    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
    
    #automatic_scaling = true
    #scaling_group_variables = 'disp_x disp_y; u' 

    l_tol = 1e-3
    nl_max_its = 15
    nl_rel_tol = 1e-6
    nl_abs_tol = 1e-6
  
    start_time = 20
    dt = 0.25 
    num_steps = 10
    max_xfem_update = 1

[]
  
  
[Outputs]
    execute_on = timestep_end
    exodus = true

    [console]
      type = Console
      output_linear = true
    []
    csv = true
    perf_graph = true
[]



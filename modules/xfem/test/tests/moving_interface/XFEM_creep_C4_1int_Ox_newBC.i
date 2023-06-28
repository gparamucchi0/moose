#test for implementing regular creep model on 1 side of the interface and not the other

[GlobalParams]
    order = FIRST
    family = LAGRANGE
    temperature = 1473.15
    displacements = 'disp_x disp_y'
[]

[Mesh]
    [gmg]
        type = GeneratedMeshGenerator
        dim = 2
        xmin = 5e-3
        xmax = 0.0056
        ymin = 0
        ymax = 6e-4

        nx = 300
        ny = 10
    []

    [top_left_node]
        type = ExtraNodesetGenerator
        new_boundary = 'top_left'
        coord = '5e-3 6e-4'
        input = gmg
    []

    [bottom_left_node]
        type = ExtraNodesetGenerator
        new_boundary = 'bottom_left'
        coord = '5e-3 0'
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
        mesh_file = interface_Q1D_newBC_2.e
        interface_velocity_function = '-3e-6'
        heal_always = true
        is_C4 = true 
        oxa_interface = true
    []
    #[velocity_ox_a]
    #    type = XFEMC4VelocityZrOxA
    #    value_at_interface_uo = value_uo_ox_a
    #[]
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
        function = 'if (x<0.00590, 0.0075 ,0.45)'
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
        use_automatic_differentiation = true
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
        type = ADComputeIsotropicElasticityTensor
        base_name = 'alpha'
        youngs_modulus = 1.01e11
        poissons_ratio = 0.33
    []
    [elasticity_tensor_oxide]
        type = ADComputeIsotropicElasticityTensor
        base_name = 'oxide'
        youngs_modulus = 1.75e11
        poissons_ratio = 0.27
    []
    [combined_elasticity_tensor]
        type = ADLevelSetBiMaterialRankFour
        level_set_var = ls_ox_a
        levelset_negative_base = 'alpha'
        levelset_positive_base = 'oxide'
        prop_name = elasticity_tensor
    []

    [radial_return_stress]
        type = ADComputeMultipleInelasticStress
        base_name = 'alpha'
        inelastic_models = 'power_law_creep_a'
    []
    [power_law_creep_a]
        type = ADPowerLawCreepStressUpdate
        coefficient = 1.2e-25
        n_exponent = 5.0
        activation_energy = 2.5e5
    []
    [stress_oxide]
        type = ADComputeFiniteStrainElasticStress
        base_name = 'oxide'
    []
    [combined_stress]
        type = ADLevelSetBiMaterialRankTwo
        levelset_negative_base = 'alpha'
        levelset_positive_base = 'oxide'
        level_set_var = ls_ox_a
        prop_name = stress
    []

    [strain_alpha]
        type = ADComputeFiniteStrain
        base_name = 'alpha'
    []
    [strain_oxide]
        type = ADComputeFiniteStrain
        base_name = 'oxide'
    []

[]

[BCs]
    [left_u]
        type = NeumannBC
        variable = u
        value = 0
        boundary = left
    []
    [right_u]
        type = DirichletBC
        variable = u
        boundary = right
        value = 0.45
    []

    [bottom_left_disp_x]
        type =ADDirichletBC
        variable = disp_x
        boundary = bottom_left
        value = 0.0
    []
    [bottom_disp_y]
        type = ADDirichletBC
        variable = disp_y
        boundary = bottom
        value = 0.0
    []

   
    [top_disp_y]
        type = ADDirichletBC
        variable = disp_y
        boundary = top
        value = 0.0
    []
    [top_left_disp_x]
        type = ADDirichletBC
        variable = disp_x
        boundary = top_left
        value = 0.0
    []
    
    [left_pressure]
        type = ADPressure
        variable = disp_x
        boundary = left
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
    
    automatic_scaling = true
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
    print_linear_residuals = true
    [console]
      type = Console
      output_linear = true
    []
    csv = true
    perf_graph = true
[]



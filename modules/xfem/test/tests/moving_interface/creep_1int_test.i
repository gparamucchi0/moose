#test for implementing regular creep model on 1 side of the interfac and not the other

[GlobalParams]
    order = FIRST
    family = LAGRANGE
    temperature = 1473.15
    displacements = 'disp_x disp_y'
[]

[Mesh]
    [cmg]
        type = CartesianMeshGenerator
        dim = 2
        dx = '300 300'
        dy = '4'
        ix = '30 151'
        iy = '2'
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
        mesh_file = flat_interface_1d.e
        interface_velocity_function = '2'
        heal_always = true
        is_C4 = true 
        oxa_interface = true
    []
[]

[Functions]
    [p]
        type = PiecewiseLinear
        x ='0 4'
        y='1 1'
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
        function = 'if (x<590.0, 0.0075,0.45)'
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
        use_automatic_differentiation = false #true
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
        prop_values = 10e6
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
        prop_name = Jacobian_mult #elastic_tensor 
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
        boundary = left
    []
    
    [right_u]
        type = DirichletBCRightC4Zr
        variable = u
        boundary = right
    []
    [bottom_disp_x]
        type = DirichletBC
        variable = disp_x
        boundary = bottom
        value = 0.0
    []
    [bottom_disp_y]
        type = DirichletBC
        variable = disp_y
        boundary = bottom
        value = 0.0
    []
    [top_disp_x]
        type = DirichletBC
        variable = disp_x
        boundary = top
        value = 0.0
    []
    [top_disp_y]
        type = DirichletBC
        variable = disp_y
        boundary = top
        value = 0.0
    []
    [left_pressure]
        type = Pressure
        variable = disp_x
        boundary = left
        factor = 10e6
        function = p   
    []
[]

[Postprocessors]
    [position_ox_a]
      type = PositionOfXFEMInterfacePostprocessor
      value_at_interface_uo = value_uo_ox_a
      execute_on ='timestep_end final'
    []
    [oxide_thickness]
        type = OxideThicknessZr
        oxide_alpha_pos = position_ox_a
        execute_on ='timestep_end final'
    []
    [weak_concentration_integral]
        type = ElementIntegralVariablePostprocessor
        variable = u
        execute_on = 'timestep_end final'
    []
[]

[VectorPostprocessors]
    [O_profile]
      type = LineValueSampler
      use_displaced_mesh = false
      start_point = '600 2 0'
      end_point = '0 2 0'
      sort_by = x
      num_points = 601
      outputs = csv
      variable = 'u'
    []
[]

[Executioner]
    type = Transient
    solve_type = 'PJFNK'
    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
    line_search = 'none'
  
    l_tol = 1e-3
    nl_max_its = 15
    nl_rel_tol = 1e-6
    nl_abs_tol = 1e-6
  
    start_time = 20
    dt = 1 
    num_steps = 50
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



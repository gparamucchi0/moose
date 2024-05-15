# Test for an oxide growing on top of a zirconium nuclear fuel cladding
# using the C4 model to compute the growth rate
# The variable is the reduced concentration [/um^3] over Czr
# The length unit is the micrometer
#T is homogeneous.
#Using the levelset advection equation to compute the velocity and update it.
#Need for an auxiliary file for the velocity and the level set equation
#phi is the levelset cut variable ls_vel is the velocity variable


[GlobalParams]
    order = FIRST
    family = LAGRANGE
    temperature = 1473.15
    temperature_neighbor = 1473.15
[]

[Mesh]
    [gen]
      type = GeneratedMeshGenerator
      dim = 2
      xmin = 0
      xmax = 600
      ymin = 0
      ymax = 4
      nx = 301 
      ny = 4 
      elem_type = QUAD4
    []
[]

[XFEM]
    qrule = volfrac
    output_cut_plane = true 
[]

[UserObjects]
    [moving_line_segments_ox_a]
        type = LevelSetCutUserObject
        level_set_var = phi
        heal_always = true
    []
    [value_uo_ox_a]
        type = QpPointValueAtXFEMInterface
        variable = 'u'
        interface_mesh_cut_userobject = 'moving_line_segments_ox_a'
        execute_on = timestep_end
        level_set_var = phi
    []
[]

[Functions]
    [phi_exact]
        type = ParsedFunction
        expression = '590' #probably wont work 
                               #might need another formulation
    []
[]

[Variables]
    [u]
    []
[]

[AuxVariables]
    [phi]
    []
    [ls_vel]
        order = FIRST
        family = LAGRANGE
    []
[]

[ICs]
    [phi_ic]
        type = FunctionIC
        function = phi_exact
        variable = phi
    []
    [ic_u]
        type = C4ZrIC4
        variable = u
    []
[]

[Constraints]
    [u_constraint]
        type = XFEMEqualValueAtInterfaceC4aox
        geometric_cut_userobject = 'moving_line_segments_ox_a'
        use_displaced_mesh = false
        variable = u
        alpha = 1e5
        level_set_var = phi
        use_penalty = false
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
        level_set_var = phi
        prop_name = diffusion_coefficient
    []
[]

[AuxKernels] 
    #ExtendVel 
    [ExtendVelocityLevelSetAux]
        type = ExtendVelocityLevelSetAuxZrOxA
        qp_point_value_user_object = value_uo_ox_a
        variable = ls_vel
        execute_on = 'timestep_end'
    []
[]

[BCs]
    # Define boundary conditions
    [right_u]
        type = DirichletBCRightC4Zr
        variable = u
        boundary = right
    []

    [left_u]
        type = NeumannBC
        variable = u
        value = 0
        boundary = left
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
[]

[MultiApps]
    [update]
        type = TransientMultiApp
        input_files ='moving_oxide_C4_adv_ls_update.i' #doesn't exist yet
        execute_on = 'timestep_end'
    []
[]

[Transfers]
    [from_sub]
        type = MultiAppNearestNodeTransfer
        source_variable = phi
        variable = phi
        direction = from_multiapp
        multi_app = update
        execute_on = 'timestep_end'
    []
    [to_sub]
        type = MultiAppNearestNodeTransfer
        source_variable = ls_vel
        variable = ls_vel
        direction = to_multiapp
        multi_app = update
        execute_on = 'timestep_end'
    []
[]

[Executioner]
    type = Transient
    #see later for solver type for PJFNK or NEWTON and for the petsc options
    solve_type = 'NEWTON'


    petsc_options_iname = '-pc_type  -pc_factor_shift_type -pc_factor_shift_amount'
    petsc_options_value = 'lu      NONZERO               1e-10'

    line_search = 'none'
    
    l_tol = 1e-3
    #l_max_its = 10
    nl_max_its = 15
    nl_rel_tol = 1e-6
    nl_abs_tol = 1e-6
  
    start_time = -0.5
    dt = 0.5
    num_steps = 51
    
    #might need some nl_forced_iteration
    #nl_forced_its = 3
[]

[Outputs]
    #execute_on = timestep_end
    exodus = true
    #[console]
    #  type = Console
    #  output_linear = true
    #[]
    csv = true
    interval = 1
[]
# Test for an oxide growing on top of a zirconium nuclear fuel cladding
# using the C4 model to compute the growth rate
# The variable is the reduced concentration [/um^3] over Czr
# The length unit is the micrometer
# there's 2 moving interfaces (alpha/oxide and alpha/beta)
# The ICs are set as constants in each phase through ICs, no steady state
# Temperature dependence is included. No heat equation yet. Homogeneous T.

# if change ix and iy (so dy), must change cut_data in MovingLineSegmentCutSetUO, ymax in weight_gain_space_integral and start/end_point in O_profile vector PP

[GlobalParams]
  order = FIRST
  family = LAGRANGE
  temperature = 1473.15
[]

[Mesh]
  [mesh]
    type = FileMeshGenerator
    file = 'cladd_30deg_2d_s10.e'
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
    interface_mesh_cut_userobject = 'moving_line_segments_ox_a'
    execute_on = 'nonlinear'
    level_set_var = ls_ox_a
    is_clad = true
  []
  [velocity_ox_a]
    type = XFEMC4VelocityZrOxA
    value_at_interface_uo = value_uo_ox_a
    is_clad = true
  []
  [moving_line_segments_ox_a]
    type = InterfaceMeshCut2DUserObjectZr
    mesh_file = interface_other_curvature_clad_2d.e
    interface_velocity_uo = velocity_ox_a
    #interface_velocity_function = 1
    #is_clad = true
    #clad_rad = 1400
    heal_always = true
    #is_C4 = true
    #oxa_interface = true

  []

  [level_set_cut]
    type = LevelSetCutUserObject
    level_set_var = ls_ox_a
    heal_always = true
  []

  #[value_uo_a_b]
  #  type = NodeValueAtXFEMInterface
  #  variable = 'u'
  #  interface_mesh_cut_userobject = 'moving_line_segments_a_b'
  #  execute_on = 'nonlinear'
  #  level_set_var = ls_a_b
  #  is_clad = true
  #[]
  #[velocity_a_b]
  #  type = XFEMC4VelocityZrAB
  #  value_at_interface_uo = value_uo_a_b
  #[]
  #[moving_line_segments_a_b]
  #  type = InterfaceMeshCut2DUserObjectZr
  #  mesh_file = interface_clad_30deg_2d.e
  #  interface_velocity_uo = velocity_a_b
  #  is_clad = true
  #  clad_rad = 1400
  #  heal_always = true
  #  is_C4 = true
  #  ab_interface = true
  #[]
[]

[Variables]
  [u]
  []
[]

[ICs]
  [ic_u]
    type = C4ZrICConst
    variable = u

    #is_clad = true
  []
[]

[AuxVariables]
  [ls_ox_a]
    order = FIRST
    family = LAGRANGE
  []
  #[ls_a_b]
  #  order = FIRST
  #  family = LAGRANGE
  #[]
[]

[Constraints]
  [u_constraint_ox_a]
    type = XFEMEqualValueAtInterfaceC4aox
    geometric_cut_userobject = 'moving_line_segments_ox_a'
    use_displaced_mesh = false
    variable = u
    alpha = 1e5
  []
  #[u_constraint_a_b]
  #  type = XFEMEqualValueAtInterfaceC4ab
  #  geometric_cut_userobject = 'moving_line_segments_a_b'
  #  use_displaced_mesh = false
  #  variable = u
  #  alpha = 1e5
  #[]
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

[AuxKernels]
  [ls_ox_a]
    type = MeshCutLevelSetAux
    mesh_cut_user_object = 'moving_line_segments_ox_a'
    variable = ls_ox_a
  []
  #[ls_a_b]
  #  type = MeshCutLevelSetAux
  #  mesh_cut_user_object = 'moving_line_segments_a_b'
  #  variable = ls_a_b
  #[]
[]

[Materials]
  #[diffusivity_beta]
  #  type = C4DiffusionCoefBeta
  #  prop_names = beta_diffusion_coefficient
  #[]
  [diffusivity_alpha]
    type = C4DiffusionCoefAlpha
    prop_names = alpha_diffusion_coefficient
  []
  [diffusivity_oxide]
    type = GenericConstantMaterial
    prop_names = oxide_diffusion_coefficient
    prop_values = 10e6
  []
  #[diff_combined]
  #  type = LevelSetTriMaterialReal
  #  levelset_neg_neg_base = 'beta'
  #  levelset_pos_neg_base = 'alpha'
  #  levelset_pos_pos_base = 'oxide'
  #  ls_var_1 = ls_a_b
  #  ls_var_2 = ls_ox_a
  #  prop_name = diffusion_coefficient
  #  outputs = exodus
  #[]
  [diff_combined]
    type = LevelSetBiMaterialReal
    level_set_var = ls_ox_a
    levelset_negative_base = 'alpha'
    levelset_positive_base = 'oxide'
    prop_name = diffusion_coefficient
    outputs = exodus
  []
[]

[BCs]
  # Define boundary conditions
  [left_u]
    type = NeumannBC
    variable = u
    value = 0
    boundary = '4'
  []

  [right_u]
    type = DirichletBCRightC4Zr
    variable = u
    boundary = '2'
  []
[]

[Postprocessors]
  [position_ox_a]
    type = PositionOfXFEMInterfacePostprocessor
    value_at_interface_uo = value_uo_ox_a
    execute_on = 'timestep_end final'
  []
  #[position_a_b]
  #  type = PositionOfXFEMInterfacePostprocessor
  #  value_at_interface_uo = value_uo_a_b
  #  execute_on ='timestep_end final'
  #[]
  [oxide_thickness]
    type = OxideThicknessZr
    oxide_alpha_pos = position_ox_a
    execute_on = 'timestep_end final'
    is_clad = true
  []
  #[alpha_thickness]
  #  type = AlphaThicknessZr
  #  oxide_alpha_pos = position_ox_a
  #  alpha_beta_pos = position_a_b
  #  execute_on ='timestep_end final'
  #[]
  #  [vacancy_flux]
  #    type = VacancyFluxZrPostprocessor
  #    velocity_uo = velocity_ox_a
  #    execute_on = 'timestep_end final'
  #  []
  #  [vacancy_flux_integral]
  #    type = TotalVariableValue
  #    value = vacancy_flux
  #    execute_on = 'timestep_end final'
  #  []
  #  [weight_gain]
  #    type = WeightGainZr
  #    flux_integral = vacancy_flux_integral
  #    execute_on = 'timestep_end final'
  #  []
  [weak_concentration_integral]
    type = ElementIntegralVariablePostprocessor
    variable = u
    execute_on = 'timestep_end final'
  []
  #[weight_gain_space_integral]
  #  type = WeightGainSpaceIntegralCladSlice
  #  concentration_integral = weak_concentration_integral
  #  angle = 30
  #  zmax = 4
  #  oxide_thickness = oxide_thickness
  #  alpha_thickness = alpha_thickness
  #  oxide_alpha_pos = position_ox_a
  #  alpha_beta_pos = position_a_b
  #  execute_on = 'timestep_end final'
  #[]
[]

#[VectorPostprocessors]
#  [O_profile]
#    type = LineValueSampler
#    use_displaced_mesh = false
#    start_point = '2000 0 0'
#    end_point = '1400 0 0'
#    sort_by = x
#    num_points = 601
#    outputs = csv
#    variable = 'u'
#  []
#[]

#[Controls]
#
#    [control]
#        type = TimePeriod
#        disable_objects = 'UserObjects::velocity_a_b UserObjects::velocity_ox_a'
#        start_time = '0'
#        end_time = '1'
#
#    []
#[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = 'none'

  l_tol = 1e-3
  #l_max_its = 10
  nl_max_its = 15
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-6

  start_time = 20
  dt = 1
  num_steps = 300
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


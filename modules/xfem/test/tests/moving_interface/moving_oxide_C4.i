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
  temperature_neighbor = 1473.15
[]

[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '300 300'
    dy = '4'#'800'
    ix = '30 101'
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
    interface_mesh_cut_userobject = 'moving_line_segments_ox_a'
    execute_on = 'nonlinear'
    level_set_var = ls_ox_a
  []
  [velocity_ox_a]
    type = XFEMC4VelocityZrOxA
    value_at_interface_uo = value_uo_ox_a
  []
  [moving_line_segments_ox_a]
    type = InterfaceMeshCut2DUserObjectZr
    mesh_file = flat_interface_1d.e 
    interface_velocity_uo = velocity_ox_a
    heal_always = true
    is_C4 = true
    oxa_interface = true
  []
[]

[Variables]
  [u]
  []
[]

[ICs]
  [ic_u]
    type = C4ZrIC4
    variable = u
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
    geometric_cut_userobject = 'moving_line_segments_ox_a'
    use_displaced_mesh = false
    variable = u
    alpha = 1e5
    level_set_var = ls_ox_a
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

[AuxKernels]
  [ls_ox_a]
    type = MeshCutLevelSetAux
    mesh_cut_user_object = 'moving_line_segments_ox_a'
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
  []
[]

[BCs]
# Define boundary conditions
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
[]

[Postprocessors]
  [position_ox_a]
    type = PositionOfXFEMInterfacePostprocessorNode
    value_at_interface_uo = value_uo_ox_a
    execute_on ='timestep_end final'
  []
  [oxide_thickness]
    type = OxideThicknessZr
    oxide_alpha_pos = position_ox_a
    execute_on ='timestep_end final'
  []
[]

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

  start_time = -0.5
  dt = 0.5
  num_steps = 301
  max_xfem_update = 1

[]


[Outputs]
  execute_on = timestep_end
  exodus = true
  #[console]
  #  type = Console
  #  output_linear = true
  #[]
  #csv = true
  perf_graph = true
[]

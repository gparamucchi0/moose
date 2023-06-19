[GlobalParams]
  order = FIRST
  family = LAGRANGE
  #radius = 1400
[]

[Mesh]
  [ring]
    type = AnnularMeshGenerator
    nt =50
    nr =10
    rmin = 1400
    rmax = 2000
  []
  [Clad]
    type = MeshExtruderGenerator
    input = ring 
    extrusion_vector = '0 0 2'
    bottom_sideset = 'bottom'
    top_sideset = 'top'
    num_layers = 2
  []
[]

[XFEM]
  qrule = volfrac
  output_cut_plane = true
[]

[UserObjects]
  [velocity]
   type = XFEMPhaseTransitionMovingInterfaceVelocity
    diffusivity_at_positive_level_set = 5
    diffusivity_at_negative_level_set = 1
    equilibrium_concentration_jump = 1
    value_at_interface_uo = value_uo
  []
  [velocity_a_b]
    type = XFEMC4VelocityZrAB
    value_at_interface_uo = value_uo
  []
  [value_uo]
    type = NodeValueAtXFEMInterface
    variable = 'u'
   interface_mesh_cut_userobject = 'cut_mesh'
   execute_on = TIMESTEP_END
    level_set_var = ls
  []
  [cut_mesh]
    type = InterfaceMeshCut3DUserObjectZr
    mesh_file = cylinder_zirc_coarse.e
    interface_velocity_uo = velocity_a_b
    heal_always = true
    is_C4 = true
    ab_interface = true
    clad_rad = 1400
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
    function = '1'
  []
[]

[AuxVariables]
  [ls]
    order = FIRST
    family = LAGRANGE
  []
[]

[Constraints]
  [u_constraint]
    type = XFEMEqualValueAtInterface
    geometric_cut_userobject = 'cut_mesh'
    use_displaced_mesh = false
    variable = u
    value = 2
    alpha = 1e6
  []
[]

[Kernels]
  [diff]
    type = MatDiffusion
    variable = u
    diffusivity = diffusion_coefficient
  []
  [time]
    type = TimeDerivative
    variable = u
  []
[]

[AuxKernels]
  [ls]
    type = MeshCutLevelSetAux
    mesh_cut_user_object = cut_mesh
    variable = ls
    execute_on = 'TIMESTEP_BEGIN'
  []
[]

[Materials]
  [diffusivity_A]
    type = GenericConstantMaterial
    prop_names = A_diffusion_coefficient
    prop_values = 5
  []
  [diffusivity_B]
    type = GenericConstantMaterial
    prop_names = B_diffusion_coefficient
    prop_values = 1
  []
  [diff_combined]
    type = LevelSetBiMaterialReal
    levelset_positive_base = 'A'
    levelset_negative_base = 'B'
    level_set_var = ls
    prop_name = diffusion_coefficient
  []
[]

[BCs]
  # Define boundary conditions
  [left_u]
    type = DirichletBC
    variable = u
    value = 2
    boundary = rmax
  []

  [right_u]
    type = NeumannBC
    variable = u
    boundary = rmin
    value = 0
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
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8

  start_time = 0.0
  dt = 1
  num_steps = 5
  max_xfem_update = 1
[]

[Outputs]
  execute_on = timestep_end
  exodus = true
  perf_graph = true
[]

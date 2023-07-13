[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y'
[]

[XFEM]
  geometric_cut_userobjects = 'cut_mesh'
  qrule = volfrac
  output_cut_plane = true
  use_crack_growth_increment = true
  crack_growth_increment = 0.2
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 41
    ny = 21
    xmin = 0.0
    xmax = 10
    ymin = 0.0
    ymax = 5.0
    elem_type = QUAD4
  []
  
[]

[UserObjects]
  #[cut_mesh]
  #  type = LevelSetCutUserObject
  #  level_set_var = ls
  #  heal_always = true
  #  execute_on = 'XFEM_MARK'
  #[]
  [line_seg_cut_uo]
    type = LineSegmentCutUserObject
    cut_data = '10  2.5  4.8  2.5'
    time_start_cut = 0.0
    time_end_cut = 2.0
    #heal_always = true

  []
  [cut_mesh]
    type = LineSegmentCutUserObject
    cut_data = '4 0 6 5'
    time_start_cut = 0
    time_end_cut = 0
  []
    
[]

[Functions]
  [ls_func]
    type = ParsedFunction
    expression = 'x-5'
  []
  [./pull]
    type = PiecewiseLinear
    x='0  5   10'
    y='0  0.02 0.1'
  []
  [vel]
    type = ParsedFunction
    expression = '0.0'
  []
[]

[Modules/TensorMechanics/Master]
  displacements = 'disp_x disp_y'
  [all]
    strain = FINITE
    add_variables = true
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 207000
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
  []
[]

[AuxVariables]
  [ls]
  []
[]

[AuxKernels]
  [ls]
    type = FunctionAux
    variable = ls
    function = ls_func
  []
[]

[Constraints]
  [./dispx_constraint]
    type = XFEMSingleVariableConstraint
    use_displaced_mesh = true
    variable = disp_x
    alpha = 1e8
    geometric_cut_userobject = 'cut_mesh'
  []
  [dispy_constraint]
    type = XFEMSingleVariableConstraint
    use_displaced_mesh = true
    variable = disp_y
    alpha = 1e8
    geometric_cut_userobject = 'cut_mesh'
  []
[]
[BCs]
  [bottom_x]
    type = DirichletBC
    variable = disp_x
    value = 0
    boundary = left
  []
  [bottom_y]
    type = DirichletBC
    variable = disp_y
    value = 0
    boundary = left
  []
  [top_y]
    type = FunctionDirichletBC
    variable = disp_y
    function = pull
    boundary = top
  []
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  l_max_its = 20
  l_tol = 1e-3
  nl_max_its = 15
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-8

  start_time = 0.0
  dt = 0.5
  end_time = 4

  max_xfem_update = 1
[]

[Outputs]
  exodus = true
[]

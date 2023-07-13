[GlobalParams]
  displacements = 'disp_x'
  volumetric_locking_correction = false
[]

[Mesh]
  [line1]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 3
    xmin = 0
    xmax = 0.5
  []
  [line2]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 3
    xmin = 0
    xmax = 0.5
  []
  # [line3]
  #   type = GeneratedMeshGenerator
  #   dim = 1
  #   nx = 3
  #   xmin = 0
  #   xmax = 0.5
  # []

  [translate_line1]
    type =TransformGenerator
    input = line1
    transform = TRANSLATE
    vector_value = '-0.6 -0.02 0'
  []
  [translate_line2]
    type =TransformGenerator
    input = line2
    transform = TRANSLATE
    vector_value = '0.1 0.02 0'
  []
  # [rotate_line3]
  #   type =TransformGenerator
  #   input = line3
  #   transform = ROTATE
  #   vector_value = '0 0 -30'
  # []

  [combine]
    type = CombinerGenerator
    inputs = 'translate_line1 translate_line2'
  []

[]
[Problem]
  solve=false
[]


[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    add_variables = true
    generate_output = 'stress_xx stress_yy vonmises_stress'
  [../]
[]

[BCs]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 207000
    poissons_ratio = 0.3
    block = 0
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
    block = 0
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      8'

  line_search = 'none'

  [./Predictor]
    type = SimplePredictor
    scale = 1.0
  [../]

# controls for linear iterations
  l_max_its = 100
  l_tol = 1e-2

# controls for nonlinear iterations
  nl_max_its = 15
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10

# time control
  start_time = 0.0
  dt = 1.0
  end_time = 3.0
[]

[Outputs]
  exodus = true
  [./console]
    type = Console
    output_linear = true
  [../]
[]

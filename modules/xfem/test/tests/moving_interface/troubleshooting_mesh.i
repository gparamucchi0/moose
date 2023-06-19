
[Mesh]
  [ring]
    type = AnnularMeshGenerator
    nt =100
    nr =10
    rmin = 1400
    rmax = 2000
  []
  [Clad]
    type = MeshExtruderGenerator
    input = ring 
    extrusion_vector = '0 0 10'
    bottom_sideset = 'bottom'
    top_sideset = 'top'
    num_layers = 4
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left_u]
    type = DirichletBC
    variable = u
    value = 2
    boundary = top
  []

  [right_u]
    type = DirichletBC
    variable = u
    boundary = bottom
    value = 0
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  execute_on = timestep_end
  exodus = true
[]

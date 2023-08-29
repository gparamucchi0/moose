#Dogbone Q1D 08/29 Oxide only w/o interface and elastic mechanics under tensile 1 test 
# right = 1; left = 3; bottom = 2; top = 4;

[GlobalParams]
    order = FIRST
    family = LAGRANGE
    temperature = 1473.15
    displacements = 'disp_x disp_y'
[]

[Mesh]
                                  #                           (-0.0003,0.0001)  ___________ (0.0003,0.0001)
                                  #                                            |__|__|__|__|
    [gmg]                         #                                            |__|__|__|__|
        type = FileMeshGenerator  #                                            |__|__|__|__|
        file = dogbone_finished_08_24.e #imported mesh        (-0.0003,-0.0001)|__|__|__|__|(0.0003,-0.0001)  (middle section only)
    []
[]

[Functions]
    [ic_u]
        type = ParsedFunction
        expression = 'if (x<0.00029, 0.0075,0.45)'
    []
[]

[Variables]
    [u]  # reduced weak oxygen concentration 
    []
[]

[ICs]
    [ic_u]
        type = FunctionIC
        variable = u
        function =  ic_u   #Step function for u(t=0) changing at 590 um of the cladding (mesh is centered)
    []
[]


[Kernels]
    [diff]
        type = MatDiffusion  #Kernel solving Fick's 2nd law 
        variable = u
        diffusivity = 'diffusion_coefficient' #Diffusion coefficient is defined in the Materials block
    []
    [time]
        type = TimeDerivative
        variable = u
    []
[]

[Modules/TensorMechanics/Master]
    [all]
        strain = FINITE  #Finite strain formlation necessary for the ComputeMultipleInelasticStress
        #use_automatic_differentiation = true   #can be enabled or not but need to change all the 
        incremental = true                      #the Materials objects with AD....
        add_variables = true
        generate_output = 'stress_xx stress_yy stress_xy strain_yy strain_xx'
    []
[]


[Materials]
    [diffusivity_oxide]     #generic diffusion coefficient for the oxide 
        type = GenericConstantMaterial
        prop_names = diffusion_coefficient
        prop_values = 10e-6 #in m^2/s
    []

    [elasticity_tensor_oxide]
        type = ComputeIsotropicElasticityTensor  #generic Elasticity tensor for the oxide phase 
        #base_name = 'oxide'  #name of the base will serve to have a combined LevelSet material property
        youngs_modulus = 1.75e11
        poissons_ratio = 0.27
        #prop_name = Jacobian_mult
    []
  
    [stress_oxide]
        type = ComputeFiniteStrainElasticStress  #Computation of elastic stress with the creep model 
        #base_name = 'oxide'  
        #prop_name = stress                    #name of the base will serve to have a combined LevelSet material property
    []
  
    #[strain_oxide]
    #    type = ComputeFiniteStrain
    #    #base_name = 'oxide'
    #[]
[]

[BCs]
    [left_u]
        type = NeumannBC     #  [du/dx]x=-0.0003 = 0
        variable = u
        value = 0
        boundary = 3
    []

    [right_u]
        type = DirichletBCRightC4Zr   # custom BC taking into account the weak formulatio of the problem 
        variable = u
        boundary = 1
    []

    [bottom_disp_y]
        type = DirichletBC
        variable = disp_y
        boundary = 2
        value = 0.0
    []
    [bottom_disp_x]
        type = DirichletBC
        variable = disp_x
        boundary = 2
        value = 0.0
    []
    [top_disp_x]
        type = DirichletBC
        variable = disp_x
        boundary = 4
        value = 0.0
    []
    [top_disp_y]
        type = FunctionDirichletBC
        variable = disp_y
        boundary = 4
        function = '0.0006*0.45*(t-20)*0.001*0.5'
    []

[]
[Debug]
    show_var_residual_norms = true
[]

[Executioner]
    type = Transient
    solve_type = 'PJFNK'
    
    line_search ='none'
    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
    
    automatic_scaling = true
    #scaling_group_variables = 'disp_x disp_y; u' 

    l_tol = 1e-7
    nl_max_its = 15
    nl_rel_tol = 1e-5
    nl_abs_tol = 1e-6
  
    start_time = 20
    dt = 0.25 
    num_steps = 320
    max_xfem_update = 1

[]
  
  
[Outputs]
    execute_on = 'timestep_end failed'
    exodus = true

    [console]
      type = Console
      output_linear = true
    []
    csv = true
    perf_graph = true
[]



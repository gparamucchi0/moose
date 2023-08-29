#Dogbone Q1D 08/29 Oxide only w interface and elastic mechanics under tensile 1 test 
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
  [gmg]                       #                                            |__|__|__|__|
    type = FileMeshGenerator  #                                            |__|__|__|__|
    file = dogbone_finished_08_24.e #imported mesh        (-0.0003,-0.0001)|__|__|__|__|(0.0003,-0.0001)  (middle section only)
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
        interface_mesh_cut_userobject = 'moving_line_segment_ox_ox'
        execute_on = 'nonlinear'
        level_set_var = ls_ox_ox 
    []
    [moving_line_segment_ox_ox]
        type = InterfaceMeshCut2DUserObjectZr  #Class taking a mesh cutter flat and that position the mesh cutter 
        mesh_file = interface_uniform_08_29.e        #at the right position depending on the number of phases. 
        interface_velocity_function = '0'  #Here we have 1 interface ox/ox and the velocity is equal to 0
        heal_always = true
        is_C4 = true
        oxa_interface = true   #true but just for positionning the interface 
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
        function =  ic_u   #Step function for u(t=0) changing at 590 um of the cladding
    []
[]

[AuxVariables]
    [ls_ox_ox]
        order = FIRST
        family = LAGRANGE
    []
[]

[Constraints]              #The Constraints block will maintain the value of the coupled variables disp_x and disp_y 
    [u_constraint_ox_a]    #as well as the reduced weak oxygen concentration
        type = XFEMEqualValueAtInterfaceC4aox
        geometric_cut_userobject = 'moving_line_segment_ox_ox'
        use_displaced_mesh = false
        variable = u
        alpha = 1e5
    []
    [disp_x_constraint]
        type = XFEMSingleVariableConstraint
        geometric_cut_userobject = 'moving_line_segment_ox_ox'
        use_displaced_mesh = false
        variable = disp_x
        alpha = 1e5
    []
    [disp_y_constraint]
        type = XFEMSingleVariableConstraint
        geometric_cut_userobject = 'moving_line_segment_ox_ox'
        use_displaced_mesh = false
        variable = disp_y
        alpha = 1e5
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
        incremental = true  #                   #the Materials objects with AD....
        add_variables = true
        generate_output = 'stress_xx stress_yy stress_xy strain_yy strain_xx'
    []
[]

[AuxKernels]
    [ls_ox_ox]
        type = MeshCutLevelSetAux
        mesh_cut_user_object = 'moving_line_segment_ox_ox'
        variable = ls_ox_ox
    []
[]

[Materials]
    [diffusivity_alpha]     #same diffusion coefficient as oxide but different prop name to make it work 
        type = GenericConstantMaterial
        prop_values = 10e-6 #in m^2/s
        prop_names = alpha_diffusion_coefficient  #name is important for the LevelSet material properties
    []
    [diffusivity_oxide]     #generic diffusion coefficient for the oxide 
        type = GenericConstantMaterial
        prop_names = oxide_diffusion_coefficient
        prop_values = 10e-6 #in m^2/s
    []
    [diff_combined]
        type = LevelSetBiMaterialReal    #LevelSet Bimaterial object that will give the alpha diffusion coefficent REAL value 
        levelset_negative_base = 'alpha' #when the values of the levelset Auxvariable is >0 and the oxide diffusion coefficient 
        levelset_positive_base = 'oxide' #REAL value in the other case. Then the material property is named 'diffusion coefficient' 
        level_set_var = ls_ox_ox          #and used by the MatDiffusion kernel
        prop_name = diffusion_coefficient
        outputs = exodus
    []


    [elasticity_tensor_alpha]
        type = ComputeIsotropicElasticityTensor  #same elasticity rank four tensor as the oxide phase
        base_name = 'alpha'  #name of the base will serve to have a combined LevelSet material property
        youngs_modulus = 1.75e11
        poissons_ratio = 0.27
    []
    [elasticity_tensor_oxide]
        type = ComputeIsotropicElasticityTensor  #generic Elasticity tensor for the oxide phase 
        base_name = 'oxide'  #name of the base will serve to have a combined LevelSet material property
        youngs_modulus = 1.75e11
        poissons_ratio = 0.27
    []
    [combined_elasticity_tensor]
        type = LevelSetBiMaterialRankFour #LevelSet Bimaterial object that will give the alpha elasticity RANK FOUR tensor
        level_set_var = ls_ox_ox           #when the values of the levelset Auxvariable is >0 and the oxide elasticity RANK FOUR 
        levelset_negative_base = 'alpha'  #tensor in the other case.
        levelset_positive_base = 'oxide'  #Depending if is_AD or not the name has to be Jacobian_mult or elasticity_tensor respectively
                                          #to be understood by the Tensor Mechanics Action
        prop_name = Jacobian_mult #elasticity_tensor
    []

    [stress_oxide]
        type = ComputeFiniteStrainElasticStress  #Computation of elastic stress without the creep model for oxide
        base_name = 'oxide'                      #name of the base will serve to have a combined LevelSet material property
    []
    [stress_alpha]
        type = ComputeFiniteStrainElasticStress  #Computation of elastic stress without the creep model for oxide again
        base_name = 'alpha'                      #name of the base will serve to have a combined LevelSet material property
    []
    [combined_stress]
        type = LevelSetBiMaterialRankTwo   #LevelSet Bimaterial object that will give the alpha stress RANK TWO tensor
        levelset_negative_base = 'alpha'   #when the values of the levelset Auxvariable is >0 and the oxide stress RANK TWO
        levelset_positive_base = 'oxide'   #tensor in the other case.
        level_set_var = ls_ox_ox            #The name of the Material property as to be stress even if is_AD
        prop_name = stress
    []

    [strain_alpha]
        type = ComputeFiniteStrain        #Computation of finite strain needed to compute the stresses in both phases 
        base_name = 'alpha'
    []
    [strain_oxide]
        type = ComputeFiniteStrain
        base_name = 'oxide'
    []

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



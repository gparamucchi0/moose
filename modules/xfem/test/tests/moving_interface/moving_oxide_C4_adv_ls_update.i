# Test for an oxide growing on top of a zirconium nuclear fuel cladding
# using the C4 model to compute the growth rate
# The variable is the reduced concentration [/um^3] over Czr
# The length unit is the micrometer
#T is homogeneous.
#Using the levelset advection equation to compute the velocity and update it.
#phi is the levelset cut variable ls_vel is the velocity variable

#Auxiliary file for ls update for moving oxide C4 adv


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

[Variables]
    [phi]
    []
[]

[AuxVariables]
    [velocity]
        family = LAGRANGE_VEC
    []
    [ls_vel]
        initial_condition = 1
    []
[]

[Functions]
    [phi_exact]
        type = ParsedFunction
        expression = '590' #probably wont work 
                               #might need another formulation
    []
[]

[ICs]
    [phi_ic]
        type = FunctionIC
        function = phi_exact
        variable = phi
    []
[]

[Kernels]
    [time]
        type = TimeDerivative
        variable = phi
    []

    [advection]
        type = LevelSetNormalAdvection
        velocity = ls_vel
        variable = phi
    []

    [advection_supg]
        type = LevelSetNormalAdvectionSUPG
        velocity = ls_vel 
        variable = phi
    []

    [time_supg]
        type = LevelSetNormalTimeDerivativeSUPG
        velocity = ls_vel
        variable = phi
    []
[]

#[AuxKernels] 
#    #ExtendVel 
#    [ExtendVelocityLevelSetAux]
#        type = ExtendVelocityLevelSetAuxZrOxA
#        qp_point_value_user_object = value_uo_ox_a
#        variable = ls_vel
#        execute_on = 'timestep_end'
#    []
#[]

[Postprocessors]
    [cfl]
        type = LevelSetCFLCondition
        velocity = velocity
        execute_on = 'initial'
    []
[]

#[MultiApps]
#    [update]
#        type = TransientMultiApp
#        input_files ='moving-oxide_C4_adv_ls_update.i' #doesn't exist yet
#        execute_on = 'timestep_end'
#    []
#[]
#
#[Transfers]
#    [from_sub]
#       type = MultiAppNearestNodeTransfer
#        source_variable = phi
#        variable = phi
#        direction = from_multiapp
#        multi_app = update
#        execute_on = 'timestep_end'
#    []
#    [to_sub]
#        type = MultiAppNearestNodeTransfer
#        source_variable = ls_vel
#        variable = ls_vel
#        direction = to_multiapp
#        multi_app = update
#        execute_on = 'timestep_end'
#    []
#[]

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
    max_xfem_update = 1
    
    #might need some nl_forced_iteration
    #nl_forced_its = 3
[]

[Outputs]
    execute_on = timestep_end
    exodus = true
    #[console]
    #  type = Console
    #  output_linear = true
    #[]
    #csv = true
    perf_graph = false
[]
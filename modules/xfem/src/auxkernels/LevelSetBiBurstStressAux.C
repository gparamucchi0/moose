//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LevelSetBiBurstStressAux.h"
#include "AuxiliarySystem.h"
#include "MooseVariable.h"
#include "XFEM.h"
#include "Function.h"

//#include "metaphysicl/raw_type.h"
registerMooseObject("MooseApp", LevelSetBiBurstStressAux);

InputParameters
LevelSetBiBurstStressAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Compute burst stress for bi-materials (consisting of two "
                             "different materials) defined by a level set function.");
  params.addRequiredParam<VariableName>(
      "level_set_var", "The name of level set variable used to represent the interface");
  params.addParam<Real>("temperature", 1473.15, "Temperature of the cladding (K)");
  params.addRequiredCoupledVar("u","oxygen weak reduced concentration");
  params.addRequiredParam<FunctionName>("u_IC", "The function that defines the initial distribution"
                                        "of oxygen in the cladding");
  return params;
}

LevelSetBiBurstStressAux::LevelSetBiBurstStressAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _level_set_var_number(_subproblem
                              .getVariable(_tid,
                                           parameters.get<VariableName>("level_set_var"),
                                           Moose::VarKindType::VAR_ANY,
                                           Moose::VarFieldType::VAR_FIELD_STANDARD)
                              .number()),
    _system(_subproblem.getSystem(getParam<VariableName>("level_set_var"))),
    _solution(*_system.current_local_solution.get()),
    _use_positive_value(false),
    _T(getParam<Real>("temperature")), 
    _u(coupledValue("u")),
    _u_0(getFunction("u_IC"))
{
  FEProblemBase * fe_problem = dynamic_cast<FEProblemBase *>(&_subproblem);

  if (fe_problem == NULL)
    mooseError("Problem casting _subproblem to FEProblemBase in XFEMMaterialStateMarkerBase");

  _xfem = MooseSharedNamespace::dynamic_pointer_cast<XFEM>(fe_problem->getXFEM());
}

Real
LevelSetBiBurstStressAux::computeValue()
{
  //parameter for brst stress in alpha phase 
  const Real a = 830e6; //Pa
  const Real b = 1e-3;  //K-1
  const Real c = 9.5e-4*10;  //1 modified

  //conversion from oxygen reduced concentration to weight fraction in alpha phase
  const Real K = 15.9994/90.8271; // from weight fraction calculation

  //yield strength of ZrO2 
  const Real sig_y = 700e6; //Pa from 
  Real u_0 ;

  const Node * node = _current_elem->node_ptr(0);

  dof_id_type ls_dof_id = node->dof_number(_system.number(), _level_set_var_number, 0);
  Number ls_node_value = _solution(ls_dof_id);

  _use_positive_value = false;

  if (_xfem->isPointInsidePhysicalDomain(_current_elem, *node))
  {
    if (ls_node_value > 0.0)
      _use_positive_value = true;
  }
  else
  {
    if (ls_node_value < 0.0)
      _use_positive_value = true;
  }
  
  if(_use_positive_value)
  {
    return sig_y ;
  }
  else
  {
    if (isNodal())
    {  //expression modified to see physical results
      u_0 = _u_0.value(_t, *_current_node);
      return  a * std::exp(-b * _T) * std::exp(- std::pow(K*(0.1*(_u[_qp]/(1+_u[_qp]) - u_0/(1+u_0))) / c,2));
    }
    else
    { //expression modified to see physical results
      u_0 = _u_0.value(_t, _q_point[_qp]);
      return a * std::exp(-b * _T) * std::exp(- std::pow(K*(0.1*(_u[_qp]/(1+_u[_qp]) - u_0/(1+u_0))) / c,2));
    }
  }
}

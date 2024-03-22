//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XFEMEqualValueAtInterfaceC4aox.h"
#include "FEProblem.h"
#include "GeometricCutUserObject.h"
#include "XFEM.h"
#include "AuxiliarySystem.h"

registerMooseObject("XFEMApp", XFEMEqualValueAtInterfaceC4aox);

InputParameters
XFEMEqualValueAtInterfaceC4aox::validParams()
{
  InputParameters params = ElemElemConstraint::validParams();
  params.addRequiredParam<Real>("alpha", "Penalty parameter in penalty formulation.");
  params.addRequiredParam<Real>("temperature", "Prescribed value at the interface.");
  params.addRequiredParam<Real>("temperature_neighbor", "Prescribed value at the interface.");
  //offset parameter for discontinuity
  params.addParam<bool>("offset",true, "Specifies if an offset is needed or not. Due to the use "
                                "of a weak discontinuity variable, an offset on the oxide/alpha interface"
                                "value is needed when an alpha/beta interface is also used.");
  params.addRequiredParam<bool>("use_penalty", "Use penalty approach.");
  params.addParam<Real>(
      "diff", 1., "The diffusion (or thermal conductivity or viscosity) coefficient.");
  params.addRequiredParam<VariableName>(
      "level_set_var", "The name of level set variable used to represent the interface");
  params.addParam<UserObjectName>(
      "geometric_cut_userobject",
      "Name of GeometricCutUserObject associated with this constraint.");
  params.addClassDescription("Enforce that the solution have the same value on opposing sides of "
                             "an XFEM interface. Here this is the oxide/alpha interface value is given"
                             "as modelled in the C4 model. The value is given by Zr phase diagram.");
  params.addRelationshipManager("ElementSideNeighborLayers",
                                Moose::RelationshipManagerType::ALGEBRAIC,
                                [](const InputParameters &, InputParameters & rm_params)
                                { rm_params.set<unsigned short>("layers") = 2; });
  return params;
}

XFEMEqualValueAtInterfaceC4aox::XFEMEqualValueAtInterfaceC4aox(const InputParameters & parameters)
  : ElemElemConstraint(parameters),
    _alpha(getParam<Real>("alpha")),
    _temperature(getParam<Real>("temperature")),
    _temperature_neighbor(getParam<Real>("temperature_neighbor")),
    _offset(getParam<bool>("offset")),
    _level_set_var_number(_subproblem
                              .getVariable(_tid,
                                           parameters.get<VariableName>("level_set_var"),
                                           Moose::VarKindType::VAR_ANY,
                                           Moose::VarFieldType::VAR_FIELD_STANDARD)
                              .number()),
    _system(_subproblem.getSystem(getParam<VariableName>("level_set_var"))),
    _solution(*_system.current_local_solution.get()),
    _use_penalty(getParam<bool>("use_penalty")),
    _diff(getParam<Real>("diff"))
{
  _xfem = std::dynamic_pointer_cast<XFEM>(_fe_problem.getXFEM());
  if (_xfem == nullptr)
    mooseError("Problem casting to XFEM in XFEMEqualValueAtInterfaceC4aox");

  const UserObject * uo =
      &(_fe_problem.getUserObjectBase(getParam<UserObjectName>("geometric_cut_userobject")));

  if (dynamic_cast<const GeometricCutUserObject *>(uo) == nullptr)
    mooseError("UserObject casting to GeometricCutUserObject in XFEMEqualValueAtInterfaceC4aox");

  _interface_id = _xfem->getGeometricCutID(dynamic_cast<const GeometricCutUserObject *>(uo));

  //transformtion of _temperature into _value

  Real x_o_a_ox = 0.2978;
  if (_temperature > 473.15 && _temperature < 1478.15)
  {
    x_o_a_ox = (28.6 + exp(-6748/_temperature + 4.748)) * 1e-2;
  }
  else if (_temperature > 1478.15 && _temperature < 1798.15)
  {
    x_o_a_ox = (28.6 + exp(-6301/_temperature + 4.460)) * 1e-2;
  }
  else if (_temperature > 1798.15 && _temperature < 2338.15)
  {
    x_o_a_ox = (28.6 + exp(-7012/_temperature + 8.434 - 3.521e-3 * _temperature )) * 1e-2;
  }
  else
  {
    x_o_a_ox = 28.6 *1e-2;
  }

  _value = x_o_a_ox / (1 - x_o_a_ox);

  //same transformation for _value_neighbor

  Real x_o_a_ox_neigh = 0.2978;
  if (_temperature_neighbor > 473.15 && _temperature_neighbor < 1478.15)
  {
    x_o_a_ox_neigh = (28.6 + exp(-6748/_temperature_neighbor + 4.748)) * 1e-2;
  }
  else if (_temperature_neighbor > 1478.15 && _temperature_neighbor < 1798.15)
  {
    x_o_a_ox_neigh = (28.6 + exp(-6301/_temperature_neighbor + 4.460)) * 1e-2;
  }
  else if (_temperature_neighbor > 1798.15 && _temperature_neighbor < 2338.15)
  {
    x_o_a_ox_neigh = (28.6 + exp(-7012/_temperature_neighbor + 8.434 - 3.521e-3 * _temperature_neighbor )) * 1e-2;
  }
  else
  {
    x_o_a_ox_neigh = 28.6 *1e-2;
  }

  _value_neighbor = x_o_a_ox_neigh / (1 - x_o_a_ox_neigh);

  //treatment of offset for _value and _value_neighbor in case of the discontinuity

  if (_offset)
  {
    //modification relative to _value 

    const Real x_o_b_a = (9.59e-3 * (_temperature - 1136) + 4.72e-6 * pow(_temperature - 1136,2) - 4.35e-9 * pow(_temperature - 1136,3)) * 1e-2;
    const Real x_o_a_b = (45.86e-3 * (_temperature - 1136) - 44.77e-6 * pow(_temperature - 1136,2) + 17.40e-9 * pow(_temperature - 1136,3)) * 1e-2; //the original one, not the weak equivalent
    const Real c_o_a_b = x_o_a_b / (1 - x_o_a_b);
    const Real c_o_b_a = x_o_b_a / (1 - x_o_b_a);
    _value = _value; //- (c_o_a_b - c_o_b_a); 1 interface only 

    //modification relative to _value_neighbor

    const Real x_o_b_a_neigh = (9.59e-3 * (_temperature_neighbor - 1136) + 4.72e-6 * pow(_temperature_neighbor - 1136,2) - 4.35e-9 * pow(_temperature_neighbor - 1136,3)) * 1e-2;
    const Real x_o_a_b_neigh = (45.86e-3 * (_temperature_neighbor - 1136) - 44.77e-6 * pow(_temperature_neighbor - 1136,2) + 17.40e-9 * pow(_temperature_neighbor - 1136,3)) * 1e-2; //the original one, not the weak equivalent
    const Real c_o_a_b_neigh = x_o_a_b_neigh / (1 - x_o_a_b_neigh);
    const Real c_o_b_a_neigh = x_o_b_a_neigh / (1 - x_o_b_a_neigh);
    _value_neighbor = _value_neighbor; //- (c_o_a_b_neigh - c_o_b_a_neigh); 1 interface only 

  }

}

XFEMEqualValueAtInterfaceC4aox::~XFEMEqualValueAtInterfaceC4aox() {}

void
XFEMEqualValueAtInterfaceC4aox::reinitConstraintQuadrature(const ElementPairInfo & element_pair_info)
{
  _interface_normal = element_pair_info._elem1_normal;
  ElemElemConstraint::reinitConstraintQuadrature(element_pair_info);
}

Real
XFEMEqualValueAtInterfaceC4aox::computeQpResidual(Moose::DGResidualType type)
{
  Real area = _xfem->getCutPlaneArea(_current_elem);
  Real elem_vol = _xfem->getPhysicalVolumeFraction(_current_elem) * _current_elem->volume();
  Real neighbor_vol = _xfem->getPhysicalVolumeFraction(_neighbor_elem) * _neighbor_elem->volume();

  unsigned int count_pos = 0;
  unsigned int count_neg = 0;

  const std::set<unsigned int> new_nodes = _xfem->getNewNodes();

  std::set<unsigned int>::const_iterator it;

  for (auto neighbor : _current_elem->neighbor_ptr_range())
  {
    if (neighbor != nullptr)
      for (unsigned int i = 0; i < neighbor->n_nodes(); i++)
      {
        const Node * node = neighbor->node_ptr(i);
        if (new_nodes.find(node->id()) == new_nodes.end())
        {
          // std::cout << "node id = " << node->id() << std::endl;
          // std::cout << "node = " << *node << std::endl;
          // std::cout << "_level_set_var_number = " << _level_set_var_number << std::endl;
          // std::cout << "_system number = " << _system.number() << std::endl;
          // neighbor->print_info();
          // node->print_dof_info();
          dof_id_type ls_dof_id = node->dof_number(_system.number(), _level_set_var_number, 0);
          Number ls_node_value = _solution(ls_dof_id);
          if (ls_node_value >= 0.5)
            count_pos += 1;
          else
            count_neg += 1;
        }
      }
  }

  // for (unsigned int i = 0; i < _current_elem->n_nodes(); i++)
  // {
  //   const Node * node = _current_elem->node_ptr(i);
  //
  //   dof_id_type ls_dof_id = node->dof_number(_system.number(), _level_set_var_number, 0);
  //   Number ls_node_value = _solution(ls_dof_id);
  //
  //   if (_xfem->isPointInsidePhysicalDomain(_current_elem, *node))
  //   {
  //     if (ls_node_value >= 0.5)
  //       count_pos += 1;
  //   }
  //   else
  //   {
  //     if (ls_node_value < 0.5)
  //       count_pos += 1;
  //   }
  // }

  Real use_positive_property = false;

  // std::cout << "area = " << area << ", elem_vol = " << elem_vol
  //           << ", neighbor_vol =  " << neighbor_vol << std::endl;

  // std::cout << "count_pos = " << count_pos << std::endl;

  // if (count_pos / _current_elem->n_nodes() > 0.8)
  //   use_positive_property = true;

  if (count_pos > count_neg)
    use_positive_property = true;

  // const Node * node = _current_elem->node_ptr(0);
  // dof_id_type ls_dof_id = node->dof_number(_system.number(), _level_set_var_number, 0);
  // Number ls_node_value = _solution(ls_dof_id);

  // if (_xfem->isPointInsidePhysicalDomain(_current_elem, *node))
  // {
  //   if (ls_node_value >= 0.5)
  //     use_positive_property = true;
  // }
  // else
  // {
  //   if (ls_node_value < 0.5)
  //     use_positive_property = true;
  // }

  Real r = 0;

  Real C_elem = std::sqrt(std::abs(_diff * area / elem_vol));
  Real C_neigh = std::sqrt(std::abs(_diff * area / neighbor_vol));

  // C_elem = std::min(C_elem, 1e5);
  // C_neigh = std::min(C_neigh, 1e5);

  // C_elem = 1.0;
  // C_neigh = 1.0;

  // std::cout << "_u[_qp] = " << _u[_qp] << ", _u_neighbor[_qp] = " << _u_neighbor[_qp]
  //           << ", area = " << area << std::endl;

  switch (type)
  {
    case Moose::Element:

      if (!_use_penalty)
      {
        r += -_test[_i][_qp] * (_grad_u[_qp] * _diff * _interface_normal) +
             (_u[_qp] - (use_positive_property ? _value : _value_neighbor)) *
                 (_grad_test[_i][_qp] * _diff * _interface_normal);
        r += _alpha * (_u[_qp] - (use_positive_property ? _value : _value_neighbor)) *
             _test[_i][_qp] * C_elem;
      }
      else
        r += _alpha * (_u[_qp] - (use_positive_property ? _value : _value_neighbor)) *
             _test[_i][_qp] * C_elem;
      break;

    case Moose::Neighbor:
      if (!_use_penalty)
      {
        r += -_test_neighbor[_i][_qp] * (_grad_u_neighbor[_qp] * _diff * -_interface_normal) +
             (_u_neighbor[_qp] - (use_positive_property ? _value_neighbor : _value)) *
                 (_grad_test_neighbor[_i][_qp] * _diff * -_interface_normal);
        r += _alpha * (_u_neighbor[_qp] - (use_positive_property ? _value_neighbor : _value)) *
             _test_neighbor[_i][_qp] * C_neigh;
      }
      else
        r += _alpha * (_u_neighbor[_qp] - (use_positive_property ? _value_neighbor : _value)) *
             _test_neighbor[_i][_qp] * C_neigh;
      break;
  }
  return r;
}

Real
XFEMEqualValueAtInterfaceC4aox::computeQpJacobian(Moose::DGJacobianType type)
{
  Real area = _xfem->getCutPlaneArea(_current_elem);
  Real elem_vol = _xfem->getPhysicalVolumeFraction(_current_elem) * _current_elem->volume();
  Real neighbor_vol = _xfem->getPhysicalVolumeFraction(_neighbor_elem) * _neighbor_elem->volume();

  Real r = 0;

  Real C_elem = std::sqrt(std::abs(_diff * area / elem_vol));
  Real C_neigh = std::sqrt(std::abs(_diff * area / neighbor_vol));
  // C_elem = std::min(C_elem, 1e5);
  // C_neigh = std::min(C_neigh, 1e5);
  // C_elem = 1.0;
  // C_neigh = 1.0;

  switch (type)
  {
    case Moose::ElementElement:
      if (!_use_penalty)
      {
        r += -_test[_i][_qp] * (_grad_phi[_j][_qp] * _diff * _interface_normal) +
             _phi[_j][_qp] * (_grad_test[_i][_qp] * _diff * _interface_normal);
        r += _alpha * _phi[_j][_qp] * _test[_i][_qp] * C_elem;
      }
      else
        r += _alpha * _phi[_j][_qp] * _test[_i][_qp] * C_elem;
      break;

    case Moose::NeighborNeighbor:
      if (!_use_penalty)
      {
        r += -_test_neighbor[_i][_qp] * (_grad_phi_neighbor[_j][_qp] * _diff * -_interface_normal) +
             _phi_neighbor[_j][_qp] * (_grad_test_neighbor[_i][_qp] * _diff * -_interface_normal);
        r += _alpha * _phi_neighbor[_j][_qp] * _test_neighbor[_i][_qp] * C_neigh;
      }
      else
        r += _alpha * _phi_neighbor[_j][_qp] * _test_neighbor[_i][_qp] * C_neigh;
      break;

    default:
      break;
  }

  return r;
}

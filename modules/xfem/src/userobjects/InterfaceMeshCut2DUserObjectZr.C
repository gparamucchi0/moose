//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceMeshCut2DUserObjectZr.h"
#include "XFEMMovingInterfaceVelocityBase.h"

registerMooseObject("XFEMApp", InterfaceMeshCut2DUserObjectZr);

InputParameters
InterfaceMeshCut2DUserObjectZr::validParams()
{
  InputParameters params = InterfaceMeshCut2DUserObject::validParams();
  //required parameters
  //ONLY PUT A LINE OR SPLINE 1D MESH TALLER THAN THE MESH OTHERWISE IT WOULD BE A CRACK
  //optionnal parameters from the C4 model
  params.addParam<bool>("is_C4",false,"Boolean specifying if the object is used for the C4 model");
  params.addParam<bool>("is_expcomp",false,"Boolean specifying if the object is used for comparing model with UW-MIT experiment");
  params.addParam<bool>("ab_interface",false,"Boolean specifying if the object is used for alpha/beta interface.");
  params.addParam<bool>("oxa_interface",false,"Boolean specifying if the object is used for oxide/alpha interface.");
  params.addParam<Real>("temperature",1473.15,"Temperature of the cladding [K]. Homogeneous temperature only.");
  //class description
  params.addClassDescription("A userobject to cut a 2D mesh using a 1D cutter mesh.");
  //return params
  return params;
 

}

InterfaceMeshCut2DUserObjectZr::InterfaceMeshCut2DUserObjectZr(const InputParameters & parameters)
  : InterfaceMeshCut2DUserObject(parameters), _is_C4(getParam<bool>("is_C4")),
    _is_expcomp(getParam<bool>("is_expcomp")),
    _ab_interface(getParam<bool>("ab_interface")),
    _oxa_interface(getParam<bool>("oxa_interface")),
    _temperature(getParam<Real>("temperature"))
{
  for (const auto & elem : _cutter_mesh->element_ptr_range())
    if (elem->type() != EDGE2)
      mooseError(
          "InterfaceMeshCut2DUserObjectZr currently only supports EDGE2 element in the cut mesh.");
}

void
InterfaceMeshCut2DUserObjectZr::initialSetup()
{
  if (_func == nullptr)
  {
    const UserObject * uo =
        &(_fe_problem.getUserObjectBase(getParam<UserObjectName>("interface_velocity_uo")));

    if (dynamic_cast<const XFEMMovingInterfaceVelocityBase *>(uo) == nullptr)
      mooseError("UserObject casting to XFEMMovingInterfaceVelocityBase in "
                 "MovingLineSegmentCutSetUserObject");

    _interface_velocity = dynamic_cast<const XFEMMovingInterfaceVelocityBase *>(uo);
    const_cast<XFEMMovingInterfaceVelocityBase *>(_interface_velocity)->initialize();
  }
   
  //Check if C4 model 
  if (_is_C4)
  {
    if (_ab_interface)
    {
      Real x_a_b;
      if (_is_expcomp)
      {
        x_a_b =1244.0;
      }
      else
      {
        x_a_b = 594;//594
      }

        //Real x_a_b = 541.7;
/**      if (MooseUtils::absoluteFuzzyEqual(_temperature,1273.15,1))
      {
        x_a_b = 591.4;
      }
      else if (MooseUtils::absoluteFuzzyEqual(_temperature,1373.15,1))
      {
        x_a_b = 583.6;
      }
      else if (MooseUtils::absoluteFuzzyEqual(_temperature,1473.15,1))
      {
        x_a_b = 572.2;
        }
      else if (MooseUtils::absoluteFuzzyEqual(_temperature,1573.15,1))
      {
        x_a_b = 560.7;
      }
      else if (MooseUtils::absoluteFuzzyEqual(_temperature,1673.15,1))
      {
        x_a_b = 539.5;
      }
      else if (MooseUtils::absoluteFuzzyEqual(_temperature,1773.15,1))
      {
        x_a_b = 516.3;
      }
      else
      {
        x_a_b = -1.9259*1e-7*pow(_temperature,3) + 6.7254*1e-4*pow(_temperature,2) - 0.84697*_temperature + 977.01  ;
      }
      
     for (auto & node : _cutter_mesh->node_ptr_range())
     {
      //std::cout << node[0] << std:endl
      for (auto & Node : node)
      {
       Node->assign(const TypeVector<double> p =(x_a_b, Node[1], Node[2]));
      }
      //std::cout << x_a_b << std::endl
     }*/
     for (auto & node : _cutter_mesh->node_ptr_range())
     {
      node->operator()(0) += - node[0](0) + x_a_b;
     }
    }
    if (_oxa_interface)
    {
      Real x_ox_a;
      if (_is_expcomp)
      {
        x_ox_a = 1248.0;
      }
      else
      {
        x_ox_a =598.0;
      }
      //Real x_ox_a = 577.9;
/**      if (MooseUtils::absoluteFuzzyEqual(_temperature,1273.15,1))
      {
        x_ox_a = 595.6;
      }
      else if (MooseUtils::absoluteFuzzyEqual(_temperature,1373.15,1))
      {
        x_ox_a = 593.2;
      }
      else if (MooseUtils::absoluteFuzzyEqual(_temperature,1473.15,1))
      {
        x_ox_a = 590.0;
      }
      else if (MooseUtils::absoluteFuzzyEqual(_temperature,1573.15,1))
      {
        x_ox_a = 586.0;
      }
      else if (MooseUtils::absoluteFuzzyEqual(_temperature,1673.15,1))
      {
        x_ox_a = 582.0;
      }
      else if (MooseUtils::absoluteFuzzyEqual(_temperature,1773.15,1))
      {
        x_ox_a = 576.2;
      }
      else
      {
        x_ox_a = -3.6071*1e-5*pow(_temperature,2) + 7.1427*1e-2*_temperature + 563.11  ;
      }
      
     for (auto & node : _cutter_mesh->node_ptr_range())
     {
      for (auto & Node : node)
      {
       Node->assign(const TypeVector<double> p =(x_ox_a, Node[1], Node[2]));
      }
     }*/
     for (auto & node : _cutter_mesh->node_ptr_range())
     {
      node->operator()(0) += - node[0](0) + x_ox_a;
     }
    }

  }
  for (const auto & node : _cutter_mesh->node_ptr_range())
    {_initial_nodes_location[node->id()] = *node;  
    //std::string info = node->get_info();
    //std::cout << info;
    }                                             
  for (const auto & elem : _cutter_mesh->element_ptr_range())
    for (unsigned int n = 0; n < elem->n_nodes(); n++)
      _node_to_elem_map[elem->node_id(n)].push_back(elem->id());

  _cutter_mesh->prepare_for_use();
  _cutter_mesh->set_mesh_dimension(_mesh.dimension() - 1);

  if (_output_exodus)
  {
    _equation_systems = std::make_unique<EquationSystems>(*_cutter_mesh);
    _explicit_system = &(_equation_systems->add_system<ExplicitSystem>("InterfaceMeshSystem"));

    _explicit_system->add_variable("disp_x");
    _explicit_system->add_variable("disp_y");

    if (_mesh.dimension() == 3)
      _explicit_system->add_variable("disp_z");

    _equation_systems->init();
    _exodus_io->write_equation_systems(_app.getOutputFileBase() + "_" + name() + ".e",
                                       *_equation_systems);

    _var_num_disp_x = _explicit_system->variable_number("disp_x");
    _var_num_disp_y = _explicit_system->variable_number("disp_y");
    if (_mesh.dimension() == 3)
      _var_num_disp_z = _explicit_system->variable_number("disp_z");
  }
}


void
InterfaceMeshCut2DUserObjectZr::calculateNormals()
{
  _element_normals.clear();

  for (const auto & elem : _cutter_mesh->element_ptr_range())
  {
    Point a = elem->node_ref(1);
    Point b = elem->node_ref(0);

    Point normal_ab = Point(-(b - a)(1), (b - a)(0), 0);
    normal_ab /= normal_ab.norm();

    _element_normals.insert(std::make_pair(elem->id(), normal_ab));
  }
}

Point
InterfaceMeshCut2DUserObjectZr::nodeNormal(const unsigned int & node_id)
{
  Point normal(0.0);

  for (const auto & node_neigh_elem_id : _node_to_elem_map[node_id])
  {
    const auto & elem = _cutter_mesh->elem_ref(node_neigh_elem_id);

    Point a = elem.node_ref(1);
    Point b = elem.node_ref(0);

    Point normal_ab = Point(-(b - a)(1), (b - a)(0), 0);
    normal_ab /= normal_ab.norm();

    normal += normal_ab;
  }

  unsigned int num = _node_to_elem_map[node_id].size();

  if (num == 0)
    mooseError("InterfaceMeshCut2DUserObjectZr, the node is not found in node_to_elem_map in "
               "calculting its normal.");

  return normal / num;
}

bool
InterfaceMeshCut2DUserObjectZr::cutElementByGeometry(const Elem * elem,
                                                   std::vector<Xfem::CutEdge> & cut_edges,
                                                   std::vector<Xfem::CutNode> & cut_nodes) const
{
  mooseAssert(elem->dim() == 2, "Dimension of element to be cut must be 2");

  bool elem_cut = false;

  for (const auto & cut_elem : _cutter_mesh->element_ptr_range())
  {
    unsigned int n_sides = elem->n_sides();

    for (unsigned int i = 0; i < n_sides; ++i)
    {
      std::unique_ptr<const Elem> curr_side = elem->side_ptr(i);

      mooseAssert(curr_side->type() == EDGE2, "Element side type must be EDGE2.");

      const Node * node1 = curr_side->node_ptr(0);
      const Node * node2 = curr_side->node_ptr(1);
      Real seg_int_frac = 0.0;

      const std::pair<Point, Point> elem_endpoints(cut_elem->node_ref(0), cut_elem->node_ref(1));

      if (Xfem::intersectSegmentWithCutLine(*node1, *node2, elem_endpoints, 1.0, seg_int_frac))
      {
        if (seg_int_frac > Xfem::tol && seg_int_frac < 1.0 - Xfem::tol)
        {
          elem_cut = true;
          Xfem::CutEdge mycut;
          mycut._id1 = node1->id();
          mycut._id2 = node2->id();
          mycut._distance = seg_int_frac;
          mycut._host_side_id = i;
          cut_edges.push_back(mycut);
        }
        else if (seg_int_frac < Xfem::tol)
        {
          elem_cut = true;
          Xfem::CutNode mycut;
          mycut._id = node1->id();
          mycut._host_id = i;
          cut_nodes.push_back(mycut);
        }
      }
    }
  }
  return elem_cut;
}

bool
InterfaceMeshCut2DUserObjectZr::cutElementByGeometry(const Elem * /* elem*/,
                                                   std::vector<Xfem::CutFace> & /*cut_faces*/) const
{
  mooseError("invalid method for InterfaceMeshCut2DUserObjectZr");
  return false;
}

bool
InterfaceMeshCut2DUserObjectZr::cutFragmentByGeometry(
    std::vector<std::vector<Point>> & /*frag_edges*/,
    std::vector<Xfem::CutEdge> & /*cut_edges*/) const
{
  mooseError("cutFragmentByGeometry not yet implemented for InterfaceMeshCut2DUserObjectZr");
  return false;
}

bool
InterfaceMeshCut2DUserObjectZr::cutFragmentByGeometry(
    std::vector<std::vector<Point>> & /*frag_faces*/,
    std::vector<Xfem::CutFace> & /*cut_faces*/) const
{
  mooseError("invalid method for InterfaceMeshCut2DUserObjectZr");
  return false;
}

Real
InterfaceMeshCut2DUserObjectZr::calculateSignedDistance(Point p) const
{
  Real min_dist = std::numeric_limits<Real>::max();

  for (const auto & cut_elem : _cutter_mesh->element_ptr_range())
  {
    Point a = cut_elem->node_ref(0);
    Point b = cut_elem->node_ref(1);

    Point c = p - a;
    Point v = (b - a) / (b - a).norm();
    Real d = (b - a).norm();
    Real t = v * c;

    Real dist;
    Point nearest_point;

    if (t < 0)
    {
      dist = (p - a).norm();
      nearest_point = a;
    }
    else if (t > d)
    {
      dist = (p - b).norm();
      nearest_point = b;
    }
    else
    {
      v *= t;
      dist = (p - a - v).norm();
      nearest_point = (a + v);
    }

    Point p_nearest_point = nearest_point - p;

    Point normal_ab = Point(-(b - a)(1), (b - a)(0), 0);

    if (normal_ab * p_nearest_point < 0)
      dist = -dist;

    if (std::abs(dist) < std::abs(min_dist))
      min_dist = dist;
  }

  return min_dist;
}

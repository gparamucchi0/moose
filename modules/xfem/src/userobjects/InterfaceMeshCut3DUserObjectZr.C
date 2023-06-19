//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceMeshCut3DUserObjectZr.h"
#include "XFEMMovingInterfaceVelocityBase.h"
#include "libmesh/enum_point_locator_type.h"

registerMooseObject("XFEMApp", InterfaceMeshCut3DUserObjectZr);

InputParameters
InterfaceMeshCut3DUserObjectZr::validParams()
{
  InputParameters params = InterfaceMeshCut3DUserObject::validParams();
  //required parameters
  //ONLY PUT A 2D CYLINDER MESH TALLER THAN THE MESH OTHERWISE IT WOULD BE A CRACK
  //optionnal parameters from the C4 model
  params.addParam<bool>("is_C4",false,"Boolean specifying if the object is used for the C4 model");
  params.addParam<bool>("is_expcomp",false,"Boolean specifying if the object is used for comparing model with UW-MIT experiment");
  params.addParam<bool>("ab_interface",false,"Boolean specifying if the object is used for alpha/beta interface.");
  params.addParam<bool>("oxa_interface",false,"Boolean specifying if the object is used for oxide/alpha interface.");
  params.addParam<Real>("temperature",1473.15,"Temperature of the cladding [K]. Homogeneous temperature only.");
  params.addParam<Real>("clad_rad",49700,"Inner radius of the cladding [um]. Needed for intial setup of the mesh");
  //class description
  params.addClassDescription("A userobject to cut a 3D mesh using a 2D cutter mesh.");
  //return params 
  return params;
}

InterfaceMeshCut3DUserObjectZr::InterfaceMeshCut3DUserObjectZr(const InputParameters & parameters)
  : InterfaceMeshCut3DUserObject(parameters), _is_C4(getParam<bool>("is_C4")),
    _is_expcomp(getParam<bool>("is_expcomp")),
    _ab_interface(getParam<bool>("ab_interface")),
    _oxa_interface(getParam<bool>("oxa_interface")),
    _temperature(getParam<Real>("temperature")),
    _R_clad(getParam<Real>("clad_rad"))
{
  for (const auto & elem : _cutter_mesh->element_ptr_range())
    if (elem->type() != TRI3)
      mooseError("InterfaceMeshCut3DUserObjectZr currently only supports TRI3 elements in the "
                 "cutting mesh.");
}

void
InterfaceMeshCut3DUserObjectZr::initialSetup()
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
        x_a_b = 572;//594
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
      //std::cout << "Node [0](0):" << node[0](0) << std::endl;
      //std::cout << "Node [0](1):" << node[0](1) << std::endl;
      if (!((node[0](0) == 0) && (node[0](1) == 0)))
      {
        node->operator()(0) /= (std::sqrt(pow(node[0](0),2) + pow(node[0](1),2)) / (_R_clad + x_a_b));
        node->operator()(1) /= (std::sqrt(pow(node[0](0),2) + pow(node[0](1),2)) / (_R_clad + x_a_b));
      }
      else
      {
        mooseError("Cylinder interface mesh not set up properly"
                   "A mesh nodes is at (0,0)");
      }
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
        x_ox_a =590.0;
      }
      //Real x_ox_a = 577.9;
      /** if (MooseUtils::absoluteFuzzyEqual(_temperature,1273.15,1))
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
      if (!((node[0](0) == 0) && (node[0](1) == 0)))
      {
        node->operator()(0) /= (std::sqrt(pow(node[0](0),2) + pow(node[0](1),2)) / (_R_clad + x_ox_a));
        node->operator()(1) /= (std::sqrt(pow(node[0](0),2) + pow(node[0](1),2)) / (_R_clad + x_ox_a)); 
      }
      else
      {
        mooseError("Cylinder interface mesh not set up properly"
                   "A mesh nodes is at (0,0)");
      }
      
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
InterfaceMeshCut3DUserObjectZr::calculateNormals()
{
  _pseudo_normal.clear();

  for (const auto & elem : _cutter_mesh->element_ptr_range())
  {
    std::vector<Point> vertices{elem->node_ref(0), elem->node_ref(1), elem->node_ref(2)};
    std::array<Point, 7> normal;
    Plane elem_plane(vertices[0], vertices[1], vertices[2]);
    normal[0] = 2.0 * libMesh::pi * elem_plane.unit_normal(vertices[0]);

    for (unsigned int i = 0; i < elem->n_nodes(); i++)
    {
      Point normal_at_node(0.0);
      const Node & node = elem->node_ref(i);

      Real angle_sum = 0.0;

      for (const auto & node_neigh_elem_id : _node_to_elem_map[node.id()])
      {
        const Elem & node_neigh_elem = _cutter_mesh->elem_ref(node_neigh_elem_id);
        std::vector<Point> vertices{
            node_neigh_elem.node_ref(0), node_neigh_elem.node_ref(1), node_neigh_elem.node_ref(2)};
        Plane elem_plane(vertices[0], vertices[1], vertices[2]);
        unsigned int j = node_neigh_elem.local_node(node.id());
        Point normal_at_node_j = elem_plane.unit_normal(vertices[0]);
        unsigned int m = j + 1 < 3 ? j + 1 : j + 1 - 3;
        unsigned int n = j + 2 < 3 ? j + 2 : j + 2 - 3;
        Point line_1 = node_neigh_elem.node_ref(j) - node_neigh_elem.node_ref(m);
        Point line_2 = node_neigh_elem.node_ref(j) - node_neigh_elem.node_ref(n);
        Real dot = line_1 * line_2;
        Real lenSq1 = line_1 * line_1;
        Real lenSq2 = line_2 * line_2;
        Real angle = std::acos(dot / std::sqrt(lenSq1 * lenSq2));
        normal_at_node += normal_at_node_j * angle;
        angle_sum += angle;
      }
      normal[1 + i] = normal_at_node;
    }

    for (unsigned int i = 0; i < elem->n_sides(); i++)
    {
      std::vector<Point> vertices{elem->node_ref(0), elem->node_ref(1), elem->node_ref(2)};

      Plane elem_plane(vertices[0], vertices[1], vertices[2]);
      Point normal_at_edge = libMesh::pi * elem_plane.unit_normal(vertices[0]);

      const Elem * neighbor = elem->neighbor_ptr(i);

      if (neighbor != nullptr)
      {
        std::vector<Point> vertices{
            neighbor->node_ref(0), neighbor->node_ref(1), neighbor->node_ref(2)};

        Plane elem_plane(vertices[0], vertices[1], vertices[2]);
        normal_at_edge += libMesh::pi * elem_plane.unit_normal(vertices[0]);
      }
      normal[4 + i] = normal_at_edge;
    }
    _pseudo_normal.insert(std::make_pair(elem->id(), normal));
  }
}

Point
InterfaceMeshCut3DUserObjectZr::nodeNormal(const unsigned int & node_id)
{
  Point normal(0.0);

  for (const auto & node_neigh_elem_id : _node_to_elem_map[node_id])
  {
    const auto & elem = _cutter_mesh->elem_ref(node_neigh_elem_id);
    Plane elem_plane(elem.node_ref(0), elem.node_ref(1), elem.node_ref(2));
    normal += elem_plane.unit_normal(elem.node_ref(0));
  }

  unsigned int num = _node_to_elem_map[node_id].size();
  return normal / num;
}

bool
InterfaceMeshCut3DUserObjectZr::cutElementByGeometry(const Elem * /*elem*/,
                                                   std::vector<Xfem::CutEdge> & /*cut_edges*/,
                                                   std::vector<Xfem::CutNode> & /*cut_nodes*/) const
{
  mooseError("invalid method for 3D mesh cutting");
  return false;
}

bool
InterfaceMeshCut3DUserObjectZr::cutElementByGeometry(const Elem * elem,
                                                   std::vector<Xfem::CutFace> & cut_faces) const
{
  mooseAssert(elem->dim() == 3, "Dimension of element to be cut must be 3");

  bool elem_cut = false;

  for (unsigned int i = 0; i < elem->n_sides(); ++i)
  {
    // This returns the lowest-order type of side.
    std::unique_ptr<const Elem> curr_side = elem->side_ptr(i);

    mooseAssert(curr_side->dim() == 2, "Side dimension must be 2");

    unsigned int n_edges = curr_side->n_sides();

    std::vector<unsigned int> cut_edges;
    std::vector<Real> cut_pos;

    for (unsigned int j = 0; j < n_edges; j++)
    {
      // This returns the lowest-order type of side.
      std::unique_ptr<const Elem> curr_edge = curr_side->side_ptr(j);
      if (curr_edge->type() != EDGE2)
        mooseError("In cutElementByGeometry face edge must be EDGE2, but type is: ",
                   libMesh::Utility::enum_to_string(curr_edge->type()),
                   " base element type is: ",
                   libMesh::Utility::enum_to_string(elem->type()));
      const Node * node1 = curr_edge->node_ptr(0);
      const Node * node2 = curr_edge->node_ptr(1);

      for (const auto & cut_elem : _cutter_mesh->element_ptr_range())
      {
        std::vector<Point> vertices;

        for (auto & node : cut_elem->node_ref_range())
        {
          Point & this_point = node;
          vertices.push_back(this_point);
        }

        Point intersection;
        if (Xfem::intersectWithEdge(*node1, *node2, vertices, intersection) &&
            std::find(cut_edges.begin(), cut_edges.end(), j) == cut_edges.end())
        {
          cut_edges.push_back(j);
          cut_pos.push_back(Xfem::getRelativePosition(*node1, *node2, intersection));
        }
      }
    }

    // if two edges of an element are cut, it is considered as an element being cut
    if (cut_edges.size() == 2)
    {
      elem_cut = true;
      Xfem::CutFace mycut;
      mycut._face_id = i;
      mycut._face_edge.push_back(cut_edges[0]);
      mycut._face_edge.push_back(cut_edges[1]);
      mycut._position.push_back(cut_pos[0]);
      mycut._position.push_back(cut_pos[1]);
      cut_faces.push_back(mycut);
    }
  }

  return elem_cut;
}

bool
InterfaceMeshCut3DUserObjectZr::cutFragmentByGeometry(
    std::vector<std::vector<Point>> & /*frag_edges*/,
    std::vector<Xfem::CutEdge> & /*cut_edges*/) const
{
  mooseError("invalid method for 3D mesh cutting");
  return false;
}

bool
InterfaceMeshCut3DUserObjectZr::cutFragmentByGeometry(
    std::vector<std::vector<Point>> & /*frag_faces*/,
    std::vector<Xfem::CutFace> & /*cut_faces*/) const
{
  mooseError("cutFragmentByGeometry not yet implemented for 3D mesh cutting");
  return false;
}

Real
InterfaceMeshCut3DUserObjectZr::calculateSignedDistance(Point p) const
{
  std::vector<Real> distance;
  Real min_dist = std::numeric_limits<Real>::max();
  for (const auto & cut_elem : _cutter_mesh->element_ptr_range())
  {
    std::vector<Point> vertices{
        cut_elem->node_ref(0), cut_elem->node_ref(1), cut_elem->node_ref(2)};
    unsigned int region;
    Point xp;
    Real dist = Xfem::pointTriangleDistance(
        p, cut_elem->node_ref(0), cut_elem->node_ref(1), cut_elem->node_ref(2), xp, region);

    distance.push_back(std::abs(dist));

    if (dist < std::abs(min_dist))
    {
      min_dist = dist;
      Point normal = (_pseudo_normal.find(cut_elem->id())->second)[region];
      if (normal * (p - xp) < 0.0)
        min_dist *= -1.0;
    }
  }
  std::sort(distance.begin(), distance.end());
  Real sum_dist = 0.0;
  for (std::vector<Real>::iterator it = distance.begin(); it != distance.begin() + 1; ++it)
    sum_dist += *it;

  if (min_dist < 0.0)
    return -sum_dist / 1.0;
  else
    return sum_dist / 1.0;
}

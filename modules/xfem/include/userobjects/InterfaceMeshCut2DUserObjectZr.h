//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceMeshCut2DUserObject.h"
/**
 * Mesh cutter for 2D material interface problems 
 * Version for the zircalloy iterfaces.
 * 
* Added booleans to specify if the object is representing an interface in the
* weak discontinuity equivalent of the C4 model for the high-temperature
* corrosion of Zircaloy-4 (1000C to 1500C), and if so which interfaces and what
* the temperature is.
* This is used to specify the "initial" (time = 20s) positions of the _two_interfaces
* as retrieved from the finite difference Matlab implementation of the model.
*
* By defaults, all the booleans are set to false so that the class can keep
* being used to model interfaces in other applications without any change.
 */

class XFEMMovingInterfaceVelocityBase;

class InterfaceMeshCut2DUserObjectZr : public InterfaceMeshCut2DUserObject
{
public:
  static InputParameters validParams();

  InterfaceMeshCut2DUserObjectZr(const InputParameters & parameters);
  //*overridig the initial setup to move the nodes according to the case in the 2d mesh /1d cut
  virtual void initialSetup() override;

  virtual bool cutElementByGeometry(const Elem * elem,
                                    std::vector<Xfem::CutEdge> & cut_edges,
                                    std::vector<Xfem::CutNode> & cut_nodes) const override;
  virtual bool cutElementByGeometry(const Elem * elem,
                                    std::vector<Xfem::CutFace> & cut_faces) const override;
  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_edges,
                                     std::vector<Xfem::CutEdge> & cut_edges) const override;
  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_faces,
                                     std::vector<Xfem::CutFace> & cut_faces) const override;

  virtual Real calculateSignedDistance(Point p) const override;

  virtual Point nodeNormal(const unsigned int & node_id) override;

  virtual void calculateNormals() override;

protected:
  /// Map of element normal
  std::unordered_map<unsigned int, Point> _element_normals;

  bool _is_C4;
  bool _is_expcomp;
  bool _ab_interface;
  bool _oxa_interface;
  
  Real _temperature;

};

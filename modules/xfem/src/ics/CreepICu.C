//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CreepICu.h"
#include "MooseUtils.h"

registerMooseObject("MooseApp", CreepICu);

InputParameters
CreepICu::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addClassDescription(
      "Initial condition for the creep input file 1 interface but with oxygen coupling. "
      "Trying to smooth x= 295 on a 300 um long slab.");
  return params;
}

CreepICu::CreepICu(const InputParameters & parameters)
  : InitialCondition(parameters)
{}

Real
CreepICu::value(const Point & p)
{
  if (p(0) < 292.5)
    {
      return _x_1;
    }
  else if (294.5 < p(0) < 295.5)
  {
    return _x_1 + (_x_2 - _x_1)*(p(0)-294.5);
  }
  else
  {
    return _x_2;
  }
  
}

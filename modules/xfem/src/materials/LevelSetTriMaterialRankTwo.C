//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LevelSetTriMaterialRankTwo.h"

InputParameters
LevelSetTriMaterialRankTwo::validParams()
{
  InputParameters params = LevelSetTriMaterialBase::validParams();
  params.addClassDescription(
      "Compute a RankTwoTensor material property for tri-materials problem (consisting of three "
      "different materials) defined by 2 level set function.");
  return params;
}

LevelSetTriMaterialRankTwo::LevelSetTriMaterialRankTwo(
    const InputParameters & parameters)
  : LevelSetTriMaterialBase(parameters),
    _trimaterial_material_prop(3),
    _material_prop(declareProperty<RankTwoTensor>(_base_name + _prop_name))
{
  _trimaterial_material_prop[0] = &getMaterialProperty<RankTwoTensor>(
      getParam<std::string>("levelset_neg_neg_base") + "_" + _prop_name);
  _trimaterial_material_prop[1] = &getMaterialProperty<RankTwoTensor>(
      getParam<std::string>("levelset_pos_neg_base") + "_" + _prop_name);
  _trimaterial_material_prop[2] = &getMaterialProperty<RankTwoTensor>(
      getParam<std::string>("levelset_pos_pos_base") + "_" + _prop_name);
}



void
LevelSetTriMaterialRankTwo::assignQpPropertiesForLevelSetNegNeg()
{
  _material_prop[_qp] = (*_trimaterial_material_prop[0])[_qp];
}

void
LevelSetTriMaterialRankTwo::assignQpPropertiesForLevelSetPosNeg()
{
  _material_prop[_qp] = (*_trimaterial_material_prop[1])[_qp];
}

void
LevelSetTriMaterialRankTwo::assignQpPropertiesForLevelSetPosPos()
{
  _material_prop[_qp] = (*_trimaterial_material_prop[2])[_qp];
}


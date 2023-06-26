//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LevelSetTriMaterialRankFour.h"

InputParameters
LevelSetTriMaterialRankFour::validParams()
{
  InputParameters params = LevelSetTriMaterialBase::validParams();
  params.addClassDescription(
      "Compute a RankFourTensor material property for tri-materials problem (consisting of three "
      "different materials) defined by 2 level set function.");
  return params;
}

LevelSetTriMaterialRankFour::LevelSetTriMaterialRankFour(
    const InputParameters & parameters)
  : LevelSetTriMaterialBase(parameters),
    _trimaterial_material_prop(3),
    _material_prop(declareProperty<RankFourTensor>(_base_name + _prop_name))
{
  _trimaterial_material_prop[0] = &getMaterialProperty<RankFourTensor>(
      getParam<std::string>("levelset_neg_neg_base") + "_" + _prop_name);
  _trimaterial_material_prop[1] = &getMaterialProperty<RankFourTensor>(
      getParam<std::string>("levelset_pos_neg_base") + "_" + _prop_name);
  _trimaterial_material_prop[2] = &getMaterialProperty<RankFourTensor>(
      getParam<std::string>("levelset_pos_pos_base") + "_" + _prop_name);
}



void
LevelSetTriMaterialRankFour::assignQpPropertiesForLevelSetNegNeg()
{
  _material_prop[_qp] = (*_trimaterial_material_prop[0])[_qp];
}

void
LevelSetTriMaterialRankFour::assignQpPropertiesForLevelSetPosNeg()
{
  _material_prop[_qp] = (*_trimaterial_material_prop[1])[_qp];
}

void
LevelSetTriMaterialRankFour::assignQpPropertiesForLevelSetPosPos()
{
  _material_prop[_qp] = (*_trimaterial_material_prop[2])[_qp];
}


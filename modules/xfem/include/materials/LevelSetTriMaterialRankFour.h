//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LevelSetTriMaterialBase.h"
#include "RankFourTensor.h"

/**
 * Compute a RankFourTensor material property for tri-materials problem
 * (consisting of three different materials) defined by a 2 level set function
 */
class LevelSetTriMaterialRankFour : public LevelSetTriMaterialBase
{
public:
  static InputParameters validParams();

  LevelSetTriMaterialRankFour(const InputParameters & parameters);

protected:
  virtual void assignQpPropertiesForLevelSetNegNeg() override;
  virtual void assignQpPropertiesForLevelSetPosNeg() override;
  virtual void assignQpPropertiesForLevelSetPosPos() override;

  /// RankTwoTensor Material properties for the two separate materials in the bi-material system
  std::vector<const MaterialProperty<RankFourTensor> *> _trimaterial_material_prop;

  /// Global RankFourTensor material property (switch tri-material tensor based on level set values)
  MaterialProperty<RankFourTensor> & _material_prop;
};


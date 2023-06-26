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
#include "RankTwoTensor.h"

/**
 * Compute a RankTwoTensor material property for rii-materials problem
 * (consisting of thre different materials) defined by 2 level set function
 */
class LevelSetTriMaterialRankTwo : public LevelSetTriMaterialBase
{
public:
  static InputParameters validParams();

  LevelSetTriMaterialRankTwo(const InputParameters & parameters);

protected:
  virtual void assignQpPropertiesForLevelSetNegNeg() override;
  virtual void assignQpPropertiesForLevelSetPosNeg() override;
  virtual void assignQpPropertiesForLevelSetPosPos() override;

  /// RankTwoTensor Material properties for the two separate materials in the bi-material system
  std::vector<const MaterialProperty<RankTwoTensor> *> _trimaterial_material_prop;

  /// Global RankTwoTensor material property (switch bi-material diffusion coefficient based on level set values)
  MaterialProperty<RankTwoTensor> & _material_prop;
};


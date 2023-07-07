//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

// Forward Declarations
class XFEM;
class Function;

/**
 * Base class for switching between materials in a bi-material system where the
 * interface is defined by a level set function.
 */
class LevelSetBiBurstStressAux : public AuxKernel
{
public:
  static InputParameters validParams();

  LevelSetBiBurstStressAux(const InputParameters & parameters);

protected:
  //From the burst cladding class
  virtual Real computeValue() override;

  /// Property name
  //std::string _prop_name;

  /// shared pointer to XFEM
  std::shared_ptr<XFEM> _xfem;

  /// The variable number of the level set variable we are operating on
  const unsigned int _level_set_var_number;

  /// system reference
  const System & _system;

  /// the subproblem solution vector
  const NumericVector<Number> & _solution;

  /// use the positive level set region's value or not
  bool _use_positive_value;
  
  //temperature value
  const Real _T;

  //reduced weak oxygen concentration
  const VariableValue & _u;
  /// Function used to define initial distribution of the oxygen in the metal
  const Function & _u_0;
};

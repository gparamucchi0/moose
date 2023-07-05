//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CreepChowOldValue.h"



registerMooseObject("TensorMechanicsApp", CreepChowOldValue);
registerMooseObject("TensorMechanicsApp", ADCreepChowOldValue);

template <bool is_ad>
InputParameters
CreepChowOldValueTempl<is_ad>::validParams()
{
  InputParameters params = RadialReturnCreepStressUpdateBaseTempl<is_ad>::validParams();
  params.addClassDescription(
      "This class uses the stress update material in a radial return isotropic power law creep "
      "model. This class can be used in conjunction with other creep and plasticity materials "
      "for more complex simulations. This specific version of the templated class also adds "
      "coupling with the oxygen concentration distribution with Chow and al. model (1982)");

  // Linear strain hardening parameters. Default values are the one of the model 
  params.addCoupledVar("temperature", "Coupled temperature");
  params.addRequiredCoupledVar("u", "coupled reduced oxygen concentration");  //couplde variable added to compute the strain rate 
  params.addParam<Real>("coefficient", 5.3395e-28, "Leading coefficient in power-law equation IN Pa^-n");
  params.addParam<Real>("n_exponent", 5.43, "Exponent on effective stress in power-law equation");
  params.addParam<Real>("m_exponent", 0.0, "Exponent on time in power-law equation");
  params.addParam<Real>("activation_energy", 320, "Activation energy");
  params.addParam<Real>("gas_constant", 8.3143, "Universal gas constant");
  params.addParam<Real>("Lambda", 0.4932, "coefficient in oxygen exp with u --> wt%(O)"); //parameter added for the reduced weak oxygen concentration
  params.addParam<Real>("start_time", 0.0, "Start time (if not zero)");
  return params;
}

template <bool is_ad>
CreepChowOldValueTempl<is_ad>::CreepChowOldValueTempl(
    const InputParameters & parameters)
  : RadialReturnCreepStressUpdateBaseTempl<is_ad>(parameters),
    _temperature(this->isParamValid("temperature")
                     ? &this->template coupledGenericValue<is_ad>("temperature")
                     : nullptr),
    _u(this->isParamValid("u")
                     ? &this->template coupledGenericValue<is_ad>("u")
                     : nullptr),
    _u_old(this->coupledValueOld("u")),  //declaration of the old value (previous timestep) form the u coupled variable 
    _coefficient(this->template getParam<Real>("coefficient")),
    _n_exponent(this->template getParam<Real>("n_exponent")),
    _m_exponent(this->template getParam<Real>("m_exponent")),
    _activation_energy(this->template getParam<Real>("activation_energy")),
    _gas_constant(this->template getParam<Real>("gas_constant")),
    _Lambda(this->template getParam<Real>("Lambda")),
    _start_time(this->template getParam<Real>("start_time")),
    _exponential(1.0),
    _exp_ox(1.0)
{
  if (_start_time < this->_app.getStartTime() && (std::trunc(_m_exponent) != _m_exponent))
    this->paramError("start_time",
                     "Start time must be equal to or greater than the Executioner start_time if a "
                     "non-integer m_exponent is used");
}

template <bool is_ad>
void
CreepChowOldValueTempl<is_ad>::computeStressInitialize(
    const GenericReal<is_ad> & /*effective_trial_stress*/,
    const GenericRankFourTensor<is_ad> & /*elasticity_tensor*/)
{
  if (_temperature)
    _exponential = std::exp(-_activation_energy / (_gas_constant * (*_temperature)[_qp]));  
  
  if (_u)
    _exp_ox = std::exp(-_Lambda * (_u_old[_qp])/(1 + _u_old[_qp])); //computation of the exponential part with the oxygen dependence at quadrature points
                                                                    //during the initialization
  _exp_time = std::pow(_t - _start_time, _m_exponent);
}

template <bool is_ad>
template <typename ScalarType>
ScalarType
CreepChowOldValueTempl<is_ad>::computeResidualInternal(
    const GenericReal<is_ad> & effective_trial_stress, const ScalarType & scalar)
{
  const ScalarType stress_delta = effective_trial_stress - _three_shear_modulus * scalar;
  const ScalarType creep_rate =
      _coefficient * std::pow(stress_delta, _n_exponent) * _exponential * _exp_time * _exp_ox;   //adding _exp_ox to the residual inernals 
  return creep_rate * _dt - scalar;
}

template <bool is_ad>
GenericReal<is_ad>
CreepChowOldValueTempl<is_ad>::computeDerivative(
    const GenericReal<is_ad> & effective_trial_stress, const GenericReal<is_ad> & scalar)
{
  const GenericReal<is_ad> stress_delta = effective_trial_stress - _three_shear_modulus * scalar;
  const GenericReal<is_ad> creep_rate_derivative =
      -_coefficient * _three_shear_modulus * _n_exponent *
      std::pow(stress_delta, _n_exponent - 1.0) * _exponential * _exp_time * _exp_ox;  //adding _exp_ox to the derivative
  return creep_rate_derivative * _dt - 1.0;
}

template <bool is_ad>
Real
CreepChowOldValueTempl<is_ad>::computeStrainEnergyRateDensity(
    const GenericMaterialProperty<RankTwoTensor, is_ad> & stress,
    const GenericMaterialProperty<RankTwoTensor, is_ad> & strain_rate)
{
  if (_n_exponent <= 1)
    return 0.0;

  Real creep_factor = _n_exponent / (_n_exponent + 1);

  return MetaPhysicL::raw_value(creep_factor * stress[_qp].doubleContraction((strain_rate)[_qp]));
}

template <bool is_ad>
void
CreepChowOldValueTempl<is_ad>::computeStressFinalize(
    const GenericRankTwoTensor<is_ad> & plastic_strain_increment)
{
  _creep_strain[_qp] += plastic_strain_increment;
}

template <bool is_ad>
void
CreepChowOldValueTempl<is_ad>::resetIncrementalMaterialProperties()
{
  _creep_strain[_qp] = _creep_strain_old[_qp];
}

template <bool is_ad>
bool
CreepChowOldValueTempl<is_ad>::substeppingCapabilityEnabled()
{
  return this->_use_substepping != RadialReturnStressUpdateTempl<is_ad>::SubsteppingType::NONE;
}

template class CreepChowOldValueTempl<false>;
template class CreepChowOldValueTempl<true>;
template Real CreepChowOldValueTempl<false>::computeResidualInternal<Real>(const Real &,
                                                                                   const Real &);
template ADReal
CreepChowOldValueTempl<true>::computeResidualInternal<ADReal>(const ADReal &,
                                                                      const ADReal &);
template ChainedReal
CreepChowOldValueTempl<false>::computeResidualInternal<ChainedReal>(const Real &,
                                                                            const ChainedReal &);
template ChainedADReal
CreepChowOldValueTempl<true>::computeResidualInternal<ChainedADReal>(const ADReal &,
                                                                             const ChainedADReal &);

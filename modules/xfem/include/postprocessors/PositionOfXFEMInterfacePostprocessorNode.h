#pragma once

#include "NodeValueAtXFEMInterface.h"
#include "GeneralPostprocessor.h"

/**
*
* Retrieves the position of a specified interface
*
*/


class PositionOfXFEMInterfacePostprocessorNode : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  PositionOfXFEMInterfacePostprocessorNode(const InputParameters & parameters);

  virtual void initialize() override;

  virtual void execute() override {}

  virtual Real getValue() override;

protected:
  /// Pointer to NodeValueAtXFEMInterface object
  const NodeValueAtXFEMInterface * _value_at_interface_uo;

};

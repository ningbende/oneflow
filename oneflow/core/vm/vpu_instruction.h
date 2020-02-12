#ifndef ONEFLOW_CORE_VM_VPU_INSTRUCTION_H_
#define ONEFLOW_CORE_VM_VPU_INSTRUCTION_H_

#include "oneflow/core/vm/vpu_type_desc.msg.h"

namespace oneflow {

class VpuInstruction {
 public:
  virtual ~VpuInstruction() = default;

  virtual void Execute() const = 0;

 protected:
  VpuInstruction() = default;
};

class VpuInstructionFactory {
 public:
  virtual ~VpuInstructionFactory() = default;

  virtual const VpuInstruction* GetVpuInstruction(VpuInstructionOpcode vpu_instr_opcode) const = 0;

 protected:
  VpuInstructionFactory() = default;
};

}  // namespace oneflow

#endif  // ONEFLOW_CORE_VM_VPU_INSTRUCTION_H_

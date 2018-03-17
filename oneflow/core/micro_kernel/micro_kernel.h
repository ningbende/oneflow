#ifndef ONEFLOW_CORE_MICRO_KERNEL_MICRO_KERNEL_H_
#define ONEFLOW_CORE_MICRO_KERNEL_MICRO_KERNEL_H_

#include "oneflow/core/micro_kernel/micro_kernel_graph.h"

namespace oneflow {

template<DeviceType device_type, typename T>
class MicroKernel : public MicroKernelNode {
 public:
  OF_DISALLOW_COPY_AND_MOVE(MicroKernel);
  virtual ~MicroKernel() = default;

  virtual void Forward(
      const KernelCtx& device_ctx,
      const std::function<Blob*(const std::string&)>& Blob4BnInOp) const = 0;

  virtual void Backward(
      const KernelCtx& device_ctx,
      const std::function<Blob*(const std::string&)>& Blob4BnInOp) const = 0;

  const BlobSymbol& out_blob_symbol() const { return *out_blob_symbol_; };
  BlobSymbol* mu_out_blob_symbol() { return out_blob_symbol_; }

 protected:
  MicroKernel(const std::vector<BlobSymbol*>& input_blob_symbols,
              BlobSymbol* out_blob_symbol)
      : MicroKernelNode(input_blob_symbols), out_blob_symbol_(out_blob_symbol) {
    CHECK(!input_blob_symbols_.empty());
    MicroKernelGraph* last_mk_graph = nullptr;
    for (BlobSymbol* input_blob_symbol : input_blob_symbols) {
      MicroKernelGraph* input_mk_graph = input_blob_symbol->mut_mk_graph();
      CHECK(last_mk_graph == nullptr || last_mk_graph == input_mk_graph);
      last_mk_graph = input_mk_graph;
      input_mk_graph->AddAllocatedNode(this);
      MicroKernelNode* input_node = input_blob_symbol->mut_producer_mk_node();
      if (input_node) { Connect(input_node, input_mk_graph->NewEdge(), this); }
    }
    out_blob_symbol->set_producer_mk_node(this);
  }
  MicroKernelGraph* mut_mk_graph() { return out_blob_symbol_->mut_mk_graph(); }

 private:
  BlobSymbol* out_blob_symbol_;
};

template<template<DeviceType, typename> class DerivedT, DeviceType device_type,
         typename T>
struct MicroKernelUtil final {
  static void Forward(
      const DerivedT<device_type, T>* micro_kernel, const KernelCtx& device_ctx,
      const std::function<Blob*(const std::string&)>& Blob4BnInOp);

  static void Backward(
      const DerivedT<device_type, T>* micro_kernel, const KernelCtx& device_ctx,
      const std::function<Blob*(const std::string&)>& Blob4BnInOp);
};

template<template<DeviceType, typename> class DerivedT, DeviceType device_type,
         typename T>
class MicroKernelIf : public MicroKernel<device_type, T> {
 public:
  OF_DISALLOW_COPY_AND_MOVE(MicroKernelIf);
  virtual ~MicroKernelIf() = default;

  void Forward(const KernelCtx& device_ctx,
               const std::function<Blob*(const std::string&)>& Blob4BnInOp)
      const override {
    auto* derived_this = dynamic_cast<DerivedT<device_type, T>*>(this);
    CHECK(derived_this);
    MicroKernelUtil<DerivedT, device_type, T>::Forward(derived_this, device_ctx,
                                                       Blob4BnInOp);
  }

  void Backward(const KernelCtx& device_ctx,
                const std::function<Blob*(const std::string&)>& Blob4BnInOp)
      const override {
    auto* derived_this = dynamic_cast<DerivedT<device_type, T>*>(this);
    CHECK(derived_this);
    MicroKernelUtil<DerivedT, device_type, T>::Backward(
        derived_this, device_ctx, Blob4BnInOp);
  }

  template<typename... Args>
  static BlobSymbol* Trainable(Args&&... args) {
    return Build(std::forward<Args>(args)...);
  }

  template<typename... Args>
  static BlobSymbol* Untrainable(Args&&... args) {
    return Build(std::forward<Args>(args)..., "");
  }

 protected:
  MicroKernelIf(const std::vector<BlobSymbol*>& input_blob_symbols,
                const std::string& out_blob_name,
                const std::string& out_diff_blob_name)
      : MicroKernel<device_type, T>(input_blob_symbols, out_blob_name,
                                    out_diff_blob_name) {}

 private:
  template<typename... Args>
  static BlobSymbol* Build(Args&&... args) {
    auto* mk = new DerivedT<device_type, T>(std::forward<Args>(args)...);
    return mk->mut_out_blob_symbol();
  }
};

}  // namespace oneflow

#endif  // ONEFLOW_CORE_MICRO_KERNEL_MICRO_KERNEL_H_

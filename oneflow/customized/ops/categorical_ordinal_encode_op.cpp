#include "oneflow/core/framework/framework.h"

namespace oneflow {

REGISTER_USER_OP("CategoricalOrdinalEncode")
    .Input("table")
    .Input("size")
    .Input("in")
    .Output("out")
    .Attr("hash_precomputed", UserOpAttrType::kAtBool)
    .SetTensorDescInferFn([](user_op::InferContext* ctx) -> Maybe<void> {
      const DataType data_type = *ctx->Dtype4ArgNameAndIndex("in", 0);
      CHECK_OR_RETURN(IsIndexDataType(data_type));
      CHECK_EQ_OR_RETURN(*ctx->Dtype4ArgNameAndIndex("table", 0), data_type);
      CHECK_EQ_OR_RETURN(*ctx->Dtype4ArgNameAndIndex("size", 0), data_type);
      *ctx->Dtype4ArgNameAndIndex("out", 0) = data_type;
      CHECK_EQ_OR_RETURN(ctx->parallel_ctx().parallel_num(), 1);
      const Shape* table_shape = ctx->Shape4ArgNameAndIndex("table", 0);
      CHECK_EQ_OR_RETURN(table_shape->NumAxes(), 1);
      CHECK_EQ_OR_RETURN(table_shape->elem_cnt() % 2, 0);
      const Shape* size_shape = ctx->Shape4ArgNameAndIndex("size", 0);
      CHECK_EQ_OR_RETURN(size_shape->NumAxes(), 1);
      CHECK_EQ_OR_RETURN(size_shape->elem_cnt(), 1);
      *ctx->Shape4ArgNameAndIndex("out", 0) = *ctx->Shape4ArgNameAndIndex("in", 0);
      return Maybe<void>::Ok();
    })
    .SetInputArgModifyFn([](user_op::GetInputArgModifier GetInputArgModifierFn) {
      user_op::InputArgModifier* table = GetInputArgModifierFn("table", 0);
      table->set_is_mutable(true);
      table->set_requires_grad(false);
      user_op::InputArgModifier* size = GetInputArgModifierFn("size", 0);
      size->set_is_mutable(true);
      size->set_requires_grad(false);
      user_op::InputArgModifier* in = GetInputArgModifierFn("in", 0);
      in->set_requires_grad(false);
    })
    .SetBatchAxisInferFn([](user_op::BatchAxisContext* ctx) -> Maybe<void> {
      CHECK_OR_RETURN(!ctx->BatchAxis4ArgNameAndIndex("table", 0)->has_value());
      CHECK_OR_RETURN(!ctx->BatchAxis4ArgNameAndIndex("size", 0)->has_value());
      *ctx->BatchAxis4ArgNameAndIndex("out", 0) = *ctx->BatchAxis4ArgNameAndIndex("in", 0);
      return Maybe<void>::Ok();
    })
    .SetGetSbpFn([](user_op::SbpContext* ctx) -> Maybe<void> {
      CHECK_EQ_OR_RETURN(ctx->parallel_num(), 1);
      return Maybe<void>::Ok();
    })
    .SetCheckAttrFn([](const user_op::UserOpDefWrapper& op_def,
                       const user_op::UserOpConfWrapper& op_conf) -> Maybe<void> {
      CHECK_OR_RETURN(op_conf.attr<bool>("hash_precomputed"));
      return Maybe<void>::Ok();
    });

}  // namespace oneflow
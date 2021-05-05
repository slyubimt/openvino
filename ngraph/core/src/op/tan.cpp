// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "itt.hpp"

#include "ngraph/op/cos.hpp"
#include "ngraph/op/divide.hpp"
#include "ngraph/op/multiply.hpp"
#include "ngraph/op/tan.hpp"

#include "ngraph/runtime/host_tensor.hpp"
#include "ngraph/runtime/reference/tan.hpp"

using namespace std;
using namespace ngraph;

constexpr NodeTypeInfo op::Tan::type_info;

op::Tan::Tan(const Output<Node>& arg)
    : UnaryElementwiseArithmetic(arg)
{
    constructor_validate_and_infer_types();
}

bool ngraph::op::v0::Tan::visit_attributes(AttributeVisitor& visitor)
{
    NGRAPH_OP_SCOPE(v0_Tan_visit_attributes);
    return true;
}

shared_ptr<Node> op::Tan::clone_with_new_inputs(const OutputVector& new_args) const
{
    NGRAPH_OP_SCOPE(v0_Tan_clone_with_new_inputs);
    check_new_args_count(this, new_args);
    return make_shared<Tan>(new_args.at(0));
}

namespace tanop
{
    template <element::Type_t ET>
    inline bool evaluate(const HostTensorPtr& arg0, const HostTensorPtr& out, const size_t count)
    {
        using T = typename element_type_traits<ET>::value_type;
        runtime::reference::tan<T>(arg0->get_data_ptr<ET>(), out->get_data_ptr<ET>(), count);
        return true;
    }

    bool evaluate_tan(const HostTensorPtr& arg0, const HostTensorPtr& out, const size_t count)
    {
        bool rc = true;
        out->set_unary(arg0);

        switch (arg0->get_element_type())
        {
            NGRAPH_TYPE_CASE(evaluate_tan, boolean, arg0, out, count);
            NGRAPH_TYPE_CASE(evaluate_tan, i32, arg0, out, count);
            NGRAPH_TYPE_CASE(evaluate_tan, i64, arg0, out, count);
            NGRAPH_TYPE_CASE(evaluate_tan, u32, arg0, out, count);
            NGRAPH_TYPE_CASE(evaluate_tan, u64, arg0, out, count);
            NGRAPH_TYPE_CASE(evaluate_tan, f16, arg0, out, count);
            NGRAPH_TYPE_CASE(evaluate_tan, f32, arg0, out, count);
        default: rc = false; break;
        }
        return rc;
    }
} // namespace tanop

bool op::Tan::evaluate(const HostTensorVector& outputs, const HostTensorVector& inputs) const
{
    NGRAPH_OP_SCOPE(v0_Tan_evaluate);
    return tanop::evaluate_tan(inputs[0], outputs[0], shape_size(get_output_shape(0)));
}

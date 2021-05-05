// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <ngraph/validation_util.hpp>
#include "itt.hpp"

#include "ngraph/op/multiply.hpp"
#include "ngraph/op/relu.hpp"

#include "ngraph/runtime/host_tensor.hpp"
#include "ngraph/runtime/reference/relu.hpp"

using namespace std;
using namespace ngraph;

NGRAPH_RTTI_DEFINITION(op::Relu, "Relu", 0, util::UnaryElementwiseArithmetic);

op::Relu::Relu(const Output<Node>& arg)
    : UnaryElementwiseArithmetic(arg)
{
    constructor_validate_and_infer_types();
}

shared_ptr<Node> op::Relu::clone_with_new_inputs(const OutputVector& new_args) const
{
    NGRAPH_OP_SCOPE(v0_Relu_clone_with_new_inputs);
    check_new_args_count(this, new_args);
    return make_shared<Relu>(new_args.at(0));
}

namespace relu
{
    template <element::Type_t ET>
    inline bool evaluate(const HostTensorPtr& arg0, const HostTensorPtr& out, const size_t count)
    {
        using T = typename element_type_traits<ET>::value_type;
        runtime::reference::relu<T>(arg0->get_data_ptr<ET>(), out->get_data_ptr<ET>(), count);
        return true;
    }

    bool evaluate_relu(const HostTensorPtr& arg0, const HostTensorPtr& out)
    {
        bool rc = true;
        size_t count = shape_size(arg0->get_shape());
        out->set_unary(arg0);

        switch (arg0->get_element_type())
        {
            NGRAPH_TYPE_CASE(evaluate_relu, boolean, arg0, out, count);
            NGRAPH_TYPE_CASE(evaluate_relu, i32, arg0, out, count);
            NGRAPH_TYPE_CASE(evaluate_relu, i64, arg0, out, count);
            NGRAPH_TYPE_CASE(evaluate_relu, u32, arg0, out, count);
            NGRAPH_TYPE_CASE(evaluate_relu, u64, arg0, out, count);
            NGRAPH_TYPE_CASE(evaluate_relu, f16, arg0, out, count);
            NGRAPH_TYPE_CASE(evaluate_relu, f32, arg0, out, count);
        default: rc = false; break;
        }
        return rc;
    }
} // namespace relu

bool op::Relu::evaluate(const HostTensorVector& outputs, const HostTensorVector& inputs) const
{
    NGRAPH_OP_SCOPE(v0_Relu_evaluate);
    NGRAPH_CHECK(validate_host_tensor_vector(outputs, 1) && validate_host_tensor_vector(inputs, 1));
    return relu::evaluate_relu(inputs[0], outputs[0]);
}

bool op::Relu::visit_attributes(AttributeVisitor& visitor)
{
    NGRAPH_OP_SCOPE(v0_Relu_visit_attributes);
    return true;
}

// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <string>
#include <vector>

#include "itt.hpp"
#include "ngraph/op/atanh.hpp"
#include "ngraph/runtime/host_tensor.hpp"
#include "ngraph/runtime/reference/atanh.hpp"
#include "ngraph/type/element_type.hpp"

using namespace std;
using namespace ngraph;

constexpr NodeTypeInfo op::v3::Atanh::type_info;

op::v3::Atanh::Atanh(const Output<Node>& arg)
    : UnaryElementwiseArithmetic(arg)
{
    constructor_validate_and_infer_types();
}

shared_ptr<Node> op::v3::Atanh::clone_with_new_inputs(const OutputVector& new_args) const
{
    NGRAPH_OP_SCOPE(v3_Atanh_clone_with_new_inputs);
    check_new_args_count(this, new_args);
    return make_shared<Atanh>(new_args.at(0));
}

namespace atanhop
{
    template <element::Type_t ET>
    bool evaluate(const HostTensorPtr& arg0, const HostTensorPtr& out)
    {
        runtime::reference::atanh(
            arg0->get_data_ptr<ET>(), out->get_data_ptr<ET>(), shape_size(arg0->get_shape()));
        return true;
    }

    bool evaluate_atanh(const HostTensorPtr& arg0, const HostTensorPtr& out)
    {
        bool rc = true;
        out->set_unary(arg0);
        switch (arg0->get_element_type())
        {
            NGRAPH_TYPE_CASE(evaluate_atanh, i32, arg0, out);
            NGRAPH_TYPE_CASE(evaluate_atanh, i64, arg0, out);
            NGRAPH_TYPE_CASE(evaluate_atanh, u32, arg0, out);
            NGRAPH_TYPE_CASE(evaluate_atanh, u64, arg0, out);
            NGRAPH_TYPE_CASE(evaluate_atanh, f16, arg0, out);
            NGRAPH_TYPE_CASE(evaluate_atanh, f32, arg0, out);
        default: rc = false; break;
        }
        return rc;
    }
} // namespace atanhop

bool op::v3::Atanh::evaluate(const HostTensorVector& outputs, const HostTensorVector& inputs) const
{
    NGRAPH_OP_SCOPE(v3_Atanh_evaluate);
    return atanhop::evaluate_atanh(inputs[0], outputs[0]);
}

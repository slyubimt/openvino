// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "ngraph/op/erf.hpp"
#include "itt.hpp"
#include "ngraph/log.hpp"
#include "ngraph/util.hpp"

#include "ngraph/runtime/host_tensor.hpp"
#include "ngraph/runtime/reference/erf.hpp"

using namespace std;
using namespace ngraph;

constexpr NodeTypeInfo op::Erf::type_info;

bool ngraph::op::v0::Erf::visit_attributes(AttributeVisitor& visitor)
{
    NGRAPH_OP_SCOPE(v0_Erf_visit_attributes);
    return true;
}

shared_ptr<Node> op::Erf::clone_with_new_inputs(const OutputVector& new_args) const
{
    NGRAPH_OP_SCOPE(v0_Erf_clone_with_new_inputs);
    check_new_args_count(this, new_args);
    return make_shared<Erf>(new_args.at(0));
}

op::Erf::Erf(const Output<Node>& arg)
    : UnaryElementwiseArithmetic(arg)
{
    constructor_validate_and_infer_types();
}

namespace erfop
{
    template <element::Type_t ET>
    inline bool evaluate(const HostTensorPtr& arg0, const HostTensorPtr& out, const size_t count)
    {
        using T = typename element_type_traits<ET>::value_type;
        runtime::reference::erf<T>(arg0->get_data_ptr<ET>(), out->get_data_ptr<ET>(), count);
        return true;
    }

    bool evaluate_erf(const HostTensorPtr& arg0, const HostTensorPtr& out, const size_t count)
    {
        bool rc = true;
        out->set_unary(arg0);

        switch (arg0->get_element_type())
        {
            NGRAPH_TYPE_CASE(evaluate_erf, boolean, arg0, out, count);
            NGRAPH_TYPE_CASE(evaluate_erf, i32, arg0, out, count);
            NGRAPH_TYPE_CASE(evaluate_erf, i64, arg0, out, count);
            NGRAPH_TYPE_CASE(evaluate_erf, u32, arg0, out, count);
            NGRAPH_TYPE_CASE(evaluate_erf, u64, arg0, out, count);
            NGRAPH_TYPE_CASE(evaluate_erf, f16, arg0, out, count);
            NGRAPH_TYPE_CASE(evaluate_erf, f32, arg0, out, count);
        default: rc = false; break;
        }
        return rc;
    }
} // namespace erfop

bool op::Erf::evaluate(const HostTensorVector& outputs, const HostTensorVector& inputs) const
{
    NGRAPH_OP_SCOPE(v0_Erf_evaluate);
    return erfop::evaluate_erf(inputs[0], outputs[0], shape_size(get_output_shape(0)));
}

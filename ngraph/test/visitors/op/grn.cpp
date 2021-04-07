// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "gtest/gtest.h"

#include "ngraph/ngraph.hpp"
#include "ngraph/op/util/attr_types.hpp"
#include "ngraph/opsets/opset1.hpp"
#include "ngraph/opsets/opset3.hpp"
#include "ngraph/opsets/opset4.hpp"
#include "ngraph/opsets/opset5.hpp"

#include "util/visitor.hpp"

using namespace std;
using namespace ngraph;
using ngraph::test::NodeBuilder;
using ngraph::test::ValueMap;

TEST(attributes, grn_op)
{
    NodeBuilder::get_ops().register_factory<opset1::GRN>();
    auto data = make_shared<op::Parameter>(element::f32, Shape{2, 3, 4, 5});

    float bias = 1.25f;

    auto grn = make_shared<opset1::GRN>(data, bias);
    NodeBuilder builder(grn);
    auto g_grn = as_type_ptr<opset1::GRN>(builder.create());

    EXPECT_EQ(g_grn->get_bias(), grn->get_bias());
}

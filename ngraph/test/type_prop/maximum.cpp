// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "arithmetic_ops.hpp"

using Type = ::testing::Types<ngraph::op::v1::Maximum>;

INSTANTIATE_TYPED_TEST_CASE_P(type_prop_maximum, ArithmeticOperator, Type);

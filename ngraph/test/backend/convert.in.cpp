// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "gtest/gtest.h"
#include "ngraph/ngraph.hpp"
#include "ngraph/runtime/reference/convert.hpp"
#include "ngraph/runtime/tensor.hpp"
#include "runtime/backend.hpp"
#include "util/all_close.hpp"
#include "util/all_close_f.hpp"
#include "util/engine/test_engines.hpp"
#include "util/ndarray.hpp"
#include "util/test_case.hpp"
#include "util/test_control.hpp"
#include "util/test_tools.hpp"

using namespace std;
using namespace ngraph;

static string s_manifest = "${MANIFEST}";

using TestEngine = test::ENGINE_CLASS_NAME(${BACKEND_NAME});

NGRAPH_TEST(${BACKEND_NAME}, convert_int32_float32)
{
    Shape shape{2, 2};
    auto A = make_shared<op::Parameter>(element::i32, shape);
    auto f = make_shared<Function>(make_shared<op::Convert>(A, element::f32), ParameterVector{A});

    auto backend = runtime::Backend::create("${BACKEND_NAME}");

    // Create some tensors for input/output
    auto a = backend->create_tensor(element::i32, shape);
    copy_data(a, vector<int32_t>{281, 2, 3, 4});
    auto result = backend->create_tensor(element::f32, shape);

    auto handle = backend->compile(f);
    handle->call_with_validate({result}, {a});
    EXPECT_TRUE(test::all_close_f((vector<float>{281, 2, 3, 4}), read_vector<float>(result)));
}

NGRAPH_TEST(${BACKEND_NAME}, convert_uint16_float32)
{
    Shape shape{2, 2};
    auto A = make_shared<op::Parameter>(element::u16, shape);
    auto f = make_shared<Function>(make_shared<op::Convert>(A, element::f32), ParameterVector{A});

    auto backend = runtime::Backend::create("${BACKEND_NAME}");

    // Create some tensors for input/output
    auto a = backend->create_tensor(element::u16, shape);
    copy_data(a, vector<uint16_t>{1, 2, 3, 4});
    auto result = backend->create_tensor(element::f32, shape);

    auto handle = backend->compile(f);
    handle->call_with_validate({result}, {a});
    EXPECT_TRUE(test::all_close_f(
        (vector<float>{1, 2, 3, 4}), read_vector<float>(result), MIN_FLOAT_TOLERANCE_BITS));
}

NGRAPH_TEST(${BACKEND_NAME}, convert_int32_bool)
{
    Shape shape{2, 3};
    auto A = make_shared<op::Parameter>(element::i32, shape);
    auto f =
        make_shared<Function>(make_shared<op::Convert>(A, element::boolean), ParameterVector{A});

    auto backend = runtime::Backend::create("${BACKEND_NAME}");

    int32_t lowest = std::numeric_limits<int32_t>::lowest();
    int32_t max = std::numeric_limits<int32_t>::max();

    // Create some tensors for input/output
    auto a = backend->create_tensor(element::i32, shape);
    copy_data(a, vector<int32_t>{0, 12, 23, 0, lowest, max});
    auto result = backend->create_tensor(element::boolean, shape);

    auto handle = backend->compile(f);
    handle->call_with_validate({result}, {a});
    EXPECT_EQ((vector<char>{0, 1, 1, 0, 1, 1}), read_vector<char>(result));
}

NGRAPH_TEST(${BACKEND_NAME}, convert_float32_bool)
{
    Shape shape{3, 3};
    auto A = make_shared<op::Parameter>(element::f32, shape);
    auto f =
        make_shared<Function>(make_shared<op::Convert>(A, element::boolean), ParameterVector{A});

    auto backend = runtime::Backend::create("${BACKEND_NAME}");

    float lowest = std::numeric_limits<float>::lowest();
    float max = std::numeric_limits<float>::max();
    float min = std::numeric_limits<float>::min();
    float pos_inf = std::numeric_limits<float>::infinity();
    float neg_inf = -std::numeric_limits<float>::infinity();

    // Create some tensors for input/output
    auto a = backend->create_tensor(element::f32, shape);
    copy_data(a, vector<float>{0.f, 1.5745f, 0.12352f, 0.f, lowest, max, min, pos_inf, neg_inf});
    auto result = backend->create_tensor(element::boolean, shape);

    auto handle = backend->compile(f);
    handle->call_with_validate({result}, {a});
    EXPECT_EQ((vector<char>{0, 1, 1, 0, 1, 1, 1, 1, 1}), read_vector<char>(result));
}

NGRAPH_TEST(${BACKEND_NAME}, convert_float32_bf16)
{
    const vector<float> a_data = {
        0.5f, 1.5f, 0.5f, 2.5f, 1.5f, 0.5f, 3.5f, 2.5f, 0.5f, 0.5f, 2.5f, 0.5f, 0.5f, 0.5f, 1.5f};

    const auto A = make_shared<op::Parameter>(element::f32, Shape{1, 1, 3, 5});
    const auto convert = make_shared<op::Convert>(A, element::bf16);
    const auto f = make_shared<Function>(NodeVector{convert}, ParameterVector{A});

    auto test_case = test::TestCase<TestEngine>(f);
    test_case.add_input<float>(a_data);
    test_case.add_expected_output<bfloat16>(
        std::vector<bfloat16>(std::begin(a_data), std::end(a_data)));

    test_case.run();
}

NGRAPH_TEST(${BACKEND_NAME}, convert_bf16_float32)
{
    const vector<bfloat16> a_data = {
        0.5, 1.5, 0.5, 2.5, 1.5, 0.5, 3.5, 2.5, 0.5, 0.5, 2.5, 0.5, 0.5, 0.5, 1.5};

    const auto A = make_shared<op::Parameter>(element::bf16, Shape{1, 1, 3, 5});
    const auto convert = make_shared<op::Convert>(A, element::f32);
    const auto f = make_shared<Function>(NodeVector{convert}, ParameterVector{A});

    auto test_case = test::TestCase<TestEngine>(f);
    test_case.add_input<bfloat16>(a_data);
    test_case.add_expected_output<float>(std::vector<float>(std::begin(a_data), std::end(a_data)));

    test_case.run();
}

NGRAPH_TEST(${BACKEND_NAME}, convert_fp16_float32)
{
    std::vector<float> f32vec = {-20.5, -15, -10.5, -0.5, 0, 0.5, 10.5, 15, 20.5};
    std::vector<float16> f16vec(std::begin(f32vec), std::end(f32vec));
    std::vector<float> result(f32vec.size());
    runtime::reference::convert(f16vec.data(), result.data(), f32vec.size());
    EXPECT_EQ(result, f32vec);
}

NGRAPH_TEST(${BACKEND_NAME}, convert_uint8_fp16)
{
    std::vector<uint8_t> u8vec = {0, 10, 15, 20, 43, 56, 78, 99, 102, 130, 142};
    std::vector<float16> f16vec(std::begin(u8vec), std::end(u8vec));
    std::vector<float16> result(u8vec.size());
    runtime::reference::convert(u8vec.data(), result.data(), u8vec.size());
    EXPECT_EQ(result, f16vec);
}

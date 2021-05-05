// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "gtest/gtest.h"
#include "ngraph/ngraph.hpp"
#include "ngraph/runtime/tensor.hpp"
#include "runtime/backend.hpp"
#include "util/all_close.hpp"
#include "util/all_close_f.hpp"
#include "util/ndarray.hpp"
#include "util/test_control.hpp"
#include "util/test_tools.hpp"

NGRAPH_SUPPRESS_DEPRECATED_START

using namespace std;
using namespace ngraph;

static string s_manifest = "${MANIFEST}";

NGRAPH_TEST(${BACKEND_NAME}, tensor_constant)
{
    Shape shape{2, 2, 2};
    auto A = op::Constant::create(element::f32, shape, {1, 2, 3, 4, 5, 6, 7, 8});
    auto f = make_shared<Function>(A, ParameterVector{});

    auto backend = runtime::Backend::create("${BACKEND_NAME}");

    // Create some tensors for input/output
    auto result = backend->create_tensor(element::f32, shape);

    auto handle = backend->compile(f);
    handle->call_with_validate({result}, {});
    EXPECT_TRUE(test::all_close_f((vector<float>{1, 2, 3, 4, 5, 6, 7, 8}),
                                  read_vector<float>(result),
                                  MIN_FLOAT_TOLERANCE_BITS));
}

NGRAPH_TEST(${BACKEND_NAME}, tensor_2constant)
{
    Shape shape{2, 2, 2};
    auto A = op::Constant::create(element::f32, shape, {1, 2, 3, 4, 5, 6, 7, 8});
    auto f = make_shared<Function>(NodeVector{A, A}, ParameterVector{});

    auto backend = runtime::Backend::create("${BACKEND_NAME}");

    // Create some tensors for input/output
    auto result0 = backend->create_tensor(element::f32, shape);
    auto result1 = backend->create_tensor(element::f32, shape);

    auto handle = backend->compile(f);
    handle->call_with_validate({result0, result1}, {});
    EXPECT_TRUE(test::all_close_f((vector<float>{1, 2, 3, 4, 5, 6, 7, 8}),
                                  read_vector<float>(result0),
                                  MIN_FLOAT_TOLERANCE_BITS));
    EXPECT_TRUE(test::all_close_f((vector<float>{1, 2, 3, 4, 5, 6, 7, 8}),
                                  read_vector<float>(result1),
                                  MIN_FLOAT_TOLERANCE_BITS));
}

NGRAPH_TEST(${BACKEND_NAME}, tensor_constant_with_op)
{
    Shape shape{2, 2, 2};
    auto A = op::Constant::create(element::f32, shape, {-1, 2, 3, -4, 5, -6, -7, 8});
    auto f = make_shared<Function>(make_shared<op::Abs>(A), ParameterVector{});

    auto backend = runtime::Backend::create("${BACKEND_NAME}");

    // Create some tensors for input/output
    auto result = backend->create_tensor(element::f32, shape);

    auto handle = backend->compile(f);
    handle->call_with_validate({result}, {});
    EXPECT_TRUE(test::all_close_f((vector<float>{1, 2, 3, 4, 5, 6, 7, 8}),
                                  read_vector<float>(result),
                                  MIN_FLOAT_TOLERANCE_BITS));
}

NGRAPH_TEST(${BACKEND_NAME}, constant_multi_use)
{
    auto A = make_shared<op::Constant>(element::i32, Shape{}, std::vector<std::string>{"388"});
    auto f = make_shared<Function>(A, ParameterVector{});
    auto backend = runtime::Backend::create("${BACKEND_NAME}");

    std::shared_ptr<runtime::Tensor> r1 = backend->create_tensor(element::i32, Shape{});
    auto handle = backend->compile(f);
    handle->call_with_validate({r1}, std::vector<std::shared_ptr<runtime::Tensor>>{});
    EXPECT_EQ(read_vector<int>(r1), std::vector<int>{388});

    std::shared_ptr<runtime::Tensor> r2 = backend->create_tensor(element::i32, Shape{});
    handle->call_with_validate({r2}, std::vector<std::shared_ptr<runtime::Tensor>>{});
    EXPECT_EQ(read_vector<int>(r2), std::vector<int>{388});
}

NGRAPH_TEST(${BACKEND_NAME}, scalar_constant_float32)
{
    auto r = op::Constant::create(element::f32, Shape{}, {4.75});
    auto f = make_shared<Function>(r, ParameterVector{});

    auto backend = runtime::Backend::create("${BACKEND_NAME}");

    // Create some tensors for input/output
    auto result = backend->create_tensor(element::f32, Shape{});

    auto handle = backend->compile(f);
    handle->call_with_validate({result}, {});
    EXPECT_TRUE(test::all_close_f(
        vector<float>{4.75f}, read_vector<float>(result), MIN_FLOAT_TOLERANCE_BITS));
}

NGRAPH_TEST(${BACKEND_NAME}, scalar_constant_int64)
{
    auto r = op::Constant::create(element::i64, Shape{}, {0x4000000000000001});
    auto f = make_shared<Function>(r, ParameterVector{});

    auto backend = runtime::Backend::create("${BACKEND_NAME}");

    // Create some tensors for input/output
    auto result = backend->create_tensor(element::i64, Shape{});

    auto handle = backend->compile(f);
    handle->call_with_validate({result}, {});
    EXPECT_EQ(vector<int64_t>{0x4000000000000001}, read_vector<int64_t>(result));
}

NGRAPH_TEST(${BACKEND_NAME}, tensor_constant_float32)
{
    Shape shape{2, 2};
    auto r = op::Constant::create(element::f32, shape, {4.75, 4.5, -5.25, 0.0});
    auto f = make_shared<Function>(r, ParameterVector{});

    auto backend = runtime::Backend::create("${BACKEND_NAME}");

    // Create some tensors for input/output
    auto result = backend->create_tensor(element::f32, shape);

    auto handle = backend->compile(f);
    handle->call_with_validate({result}, {});
    EXPECT_TRUE(test::all_close_f((vector<float>{4.75f, 4.5f, -5.25f, 0.0f}),
                                  read_vector<float>(result),
                                  MIN_FLOAT_TOLERANCE_BITS));
}

NGRAPH_TEST(${BACKEND_NAME}, tensor_constant_int64)
{
    Shape shape{2};
    auto r = op::Constant::create(element::i64, shape, {0x4000000000000001, 0x4000000000000002});
    auto f = make_shared<Function>(r, ParameterVector{});
    auto backend = runtime::Backend::create("${BACKEND_NAME}");
    // Create some tensors for input/output
    auto result = backend->create_tensor(element::i64, shape);
    auto handle = backend->compile(f);
    handle->call_with_validate({result}, {});
    EXPECT_EQ((vector<int64_t>{0x4000000000000001, 0x4000000000000002}),
              read_vector<int64_t>(result));
}

NGRAPH_TEST(${BACKEND_NAME}, constant_equality_bool)
{
    Shape shape{4};
    // auto A = make_shared<op::Parameter>(element::boolean, shape);
    // auto B = make_shared<op::Parameter>(element::boolean, shape);
    // auto f = make_shared<Function>(make_shared<op::v1::Equal>(A, B), ParameterVector{A, B});

    auto A = op::Constant::create(element::boolean, shape, {true, false, true, false});
    auto B = op::Constant::create(element::boolean, shape, {true, true, true, true});
    auto f = make_shared<Function>(make_shared<op::v1::Equal>(A, B), ParameterVector{});

    auto backend = runtime::Backend::create("${BACKEND_NAME}");

    // Create some tensors for input/output
    auto result = backend->create_tensor(element::boolean, shape);

    auto handle = backend->compile(f);
    handle->call_with_validate({result}, {});
    EXPECT_EQ((vector<char>{true, false, true, false}), read_vector<char>(result));
}

namespace
{
    std::vector<uint8_t> read_raw_data(std::shared_ptr<ngraph::runtime::Tensor> tv)
    {
        const size_t mem_size = tv->get_size_in_bytes();
        std::vector<uint8_t> rc(mem_size);
        tv->read(rc.data(), mem_size);
        return rc;
    }

    void run_constant_equality_for_low_precision(const Shape& shape,
                                                 const std::vector<uint8_t>& data,
                                                 element::Type element_type)
    {
        const void* raw_data = data.data();
        auto A = op::Constant::create(element_type, shape, raw_data);

        const auto constant_raw_data = static_cast<const uint8_t*>(A->get_data_ptr());
        EXPECT_EQ(std::memcmp(raw_data, constant_raw_data, data.size()), 0)
            << "wrong data hold in Constant";

        auto f = make_shared<Function>(A, ParameterVector{});

        auto backend = runtime::Backend::create("${BACKEND_NAME}");

        // Create some tensors for input/output
        auto result = backend->create_tensor(element_type, shape);

        auto handle = backend->compile(f);
        handle->call_with_validate({result}, {});
        EXPECT_EQ(data, read_raw_data(result));
    }
}

NGRAPH_TEST(${BACKEND_NAME}, constant_equality_u4_2x2x3)
{
    const Shape shape{2, 2, 3};
    const std::vector<uint8_t> data{0x12, 0x34, 0x56, 0x78, 0x9a, 0xFF};
    constexpr auto element_type = element::u4;

    run_constant_equality_for_low_precision(shape, data, element_type);
}

NGRAPH_TEST(${BACKEND_NAME}, constant_equality_u4_1x3)
{
    const Shape shape{1, 3};
    const std::vector<uint8_t> data{0x12, 0x34}; // last 8 bits constains rubbish
    constexpr auto element_type = element::u4;

    run_constant_equality_for_low_precision(shape, data, element_type);
}

NGRAPH_TEST(${BACKEND_NAME}, constant_equality_u4_1x10)
{
    const Shape shape{1, 10};
    const std::vector<uint8_t> data{0x12, 0x34}; // last 6 bits constains rubbish
    constexpr auto element_type = element::u1;

    run_constant_equality_for_low_precision(shape, data, element_type);
}

NGRAPH_TEST(${BACKEND_NAME}, constant_equality_i4_2x2x3)
{
    const Shape shape{2, 2, 3};
    const std::vector<uint8_t> data{0x12, 0x34, 0x56, 0x78, 0x9a, 0xFF};
    constexpr auto element_type = element::i4;

    run_constant_equality_for_low_precision(shape, data, element_type);
}

NGRAPH_TEST(${BACKEND_NAME}, constant_equality_i4_1x3)
{
    const Shape shape{1, 3};
    const std::vector<uint8_t> data{0x12, 0x34}; // last 8 bits constains rubbish
    constexpr auto element_type = element::i4;

    run_constant_equality_for_low_precision(shape, data, element_type);
}

// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "op/constant.hpp"
#include "core/attribute.hpp"
#include "core/tensor.hpp"
#include "default_opset.hpp"
#include "ngraph/log.hpp"
#include "ngraph/op/constant.hpp"
#include "ngraph/validation_util.hpp"

namespace ngraph
{
    namespace onnx_import
    {
        namespace op
        {
            namespace
            {
                template <typename T>
                inline std::shared_ptr<default_opset::Constant>
                    __make_ng_constant(const element::Type& type, const Tensor& tensor)
                {
                    std::shared_ptr<default_opset::Constant> constant{nullptr};
                    try
                    {
                        constant = std::make_shared<default_opset::Constant>(
                            type, tensor.get_shape(), tensor.get_data<T>());
                    }
                    catch (const ngraph::ngraph_error& exc)
                    {
                        NGRAPH_WARN
                            << "\nCould not create an nGraph Constant for an ONNX Constant "
                               "node. "
                            << "Constant with a 0 value was created instead.\n"
                            << "Verify if the ONNX Constant node contains a correct number of "
                               "elements matching the node's shape. \n"
                            << "Detailed error:\n"
                            << exc.what();
                        constant = std::make_shared<default_opset::Constant>(type, Shape{}, 0);
                    }

                    return constant;
                }

                template <Tensor::Type>
                inline std::shared_ptr<default_opset::Constant>
                    make_ng_constant(const Tensor& tensor)
                {
                    throw error::tensor::unsupported_data_type{tensor};
                }

                template <>
                inline std::shared_ptr<default_opset::Constant>
                    make_ng_constant<Tensor::Type::float16>(const Tensor& tensor)
                {
                    return __make_ng_constant<ngraph::float16>(element::f16, tensor);
                }

                template <>
                inline std::shared_ptr<default_opset::Constant>
                    make_ng_constant<Tensor::Type::float32>(const Tensor& tensor)
                {
                    return __make_ng_constant<float>(element::f32, tensor);
                }

                template <>
                inline std::shared_ptr<default_opset::Constant>
                    make_ng_constant<Tensor::Type::float64>(const Tensor& tensor)
                {
                    return __make_ng_constant<double>(element::f64, tensor);
                }

                template <>
                inline std::shared_ptr<default_opset::Constant>
                    make_ng_constant<Tensor::Type::int8>(const Tensor& tensor)
                {
                    return __make_ng_constant<int8_t>(element::i8, tensor);
                }

                template <>
                inline std::shared_ptr<default_opset::Constant>
                    make_ng_constant<Tensor::Type::int16>(const Tensor& tensor)
                {
                    return __make_ng_constant<int16_t>(element::i16, tensor);
                }

                template <>
                inline std::shared_ptr<default_opset::Constant>
                    make_ng_constant<Tensor::Type::int32>(const Tensor& tensor)
                {
                    return __make_ng_constant<int32_t>(element::i32, tensor);
                }

                template <>
                inline std::shared_ptr<default_opset::Constant>
                    make_ng_constant<Tensor::Type::int64>(const Tensor& tensor)
                {
                    return __make_ng_constant<int64_t>(element::i64, tensor);
                }

                template <>
                inline std::shared_ptr<default_opset::Constant>
                    make_ng_constant<Tensor::Type::uint8>(const Tensor& tensor)
                {
                    return __make_ng_constant<uint8_t>(element::u8, tensor);
                }

                template <>
                inline std::shared_ptr<default_opset::Constant>
                    make_ng_constant<Tensor::Type::uint16>(const Tensor& tensor)
                {
                    return __make_ng_constant<uint16_t>(element::u16, tensor);
                }

                template <>
                inline std::shared_ptr<default_opset::Constant>
                    make_ng_constant<Tensor::Type::uint32>(const Tensor& tensor)
                {
                    return __make_ng_constant<uint32_t>(element::u32, tensor);
                }

                template <>
                inline std::shared_ptr<default_opset::Constant>
                    make_ng_constant<Tensor::Type::uint64>(const Tensor& tensor)
                {
                    return __make_ng_constant<uint64_t>(element::u64, tensor);
                }

                template <>
                inline std::shared_ptr<default_opset::Constant>
                    make_ng_constant<Tensor::Type::boolean>(const Tensor& tensor)
                {
                    return __make_ng_constant<char>(element::boolean, tensor);
                }

                template <>
                inline std::shared_ptr<default_opset::Constant>
                    make_ng_constant<Tensor::Type::bfloat16>(const Tensor& tensor)
                {
                    return __make_ng_constant<ngraph::bfloat16>(element::bf16, tensor);
                }

                inline std::shared_ptr<default_opset::Constant> make_constant(const Tensor& tensor)
                {
#define MAKE_NG_CONSTANT(data_type_)                                                               \
    case data_type_: return make_ng_constant<data_type_>(tensor)

                    switch (tensor.get_type())
                    {
                        MAKE_NG_CONSTANT(Tensor::Type::float16);
                        MAKE_NG_CONSTANT(Tensor::Type::float32);
                        MAKE_NG_CONSTANT(Tensor::Type::float64);
                        MAKE_NG_CONSTANT(Tensor::Type::int8);
                        MAKE_NG_CONSTANT(Tensor::Type::int16);
                        MAKE_NG_CONSTANT(Tensor::Type::int32);
                        MAKE_NG_CONSTANT(Tensor::Type::int64);
                        MAKE_NG_CONSTANT(Tensor::Type::uint8);
                        MAKE_NG_CONSTANT(Tensor::Type::uint16);
                        MAKE_NG_CONSTANT(Tensor::Type::uint32);
                        MAKE_NG_CONSTANT(Tensor::Type::uint64);
                        MAKE_NG_CONSTANT(Tensor::Type::boolean);
                        MAKE_NG_CONSTANT(Tensor::Type::bfloat16);
                    default: throw error::tensor::invalid_data_type{tensor};
                    }
                }
            } // namespace

            namespace set_1
            {
                OutputVector constant(const onnx_import::Node& node)
                {
                    return {make_constant(node.get_attribute_value<Tensor>("value"))};
                }

            } // namespace set_1

            namespace set_13
            {
                OutputVector constant(const onnx_import::Node& node)
                {
                    auto attributes_names = node.get_attribute_names();
                    NGRAPH_CHECK(attributes_names.size() == 1,
                                 "The Constant op expects exactly one attribute."
                                 "Got: ",
                                 attributes_names.size());

                    auto& attribute = node.get_attribute(attributes_names[0]);

                    if (attribute.is_float())
                    {
                        return {default_opset::Constant::create(
                            element::f32, ngraph::Shape{}, {attribute.get_float()})};
                    }
                    else if (attribute.is_float_array())
                    {
                        auto values = attribute.get_float_array();
                        return {default_opset::Constant::create(
                            element::f32, ngraph::Shape{values.size()}, values)};
                    }
                    else if (attribute.is_integer())
                    {
                        return {default_opset::Constant::create(
                            element::i64, ngraph::Shape{}, {attribute.get_integer()})};
                    }
                    else if (attribute.is_integer_array())
                    {
                        auto values = attribute.get_integer_array();
                        return {default_opset::Constant::create(
                            element::i64, ngraph::Shape{values.size()}, values)};
                    }
                    return {make_constant(node.get_attribute_value<Tensor>(attributes_names[0]))};
                }

            } // namespace set_13

        } // namespace op

    } // namespace onnx_import

} // namespace ngraph

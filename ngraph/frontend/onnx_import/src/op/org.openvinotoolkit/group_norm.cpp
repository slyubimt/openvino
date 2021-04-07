// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "op/org.openvinotoolkit/group_norm.hpp"
#include "default_opset.hpp"
#include "ngraph/builder/reduce_ops.hpp"
#include "ngraph/builder/split.hpp"
#include "ngraph/node.hpp"
#include "ngraph/opsets/opset5.hpp"
#include "onnx_import/core/node.hpp"
#include "utils/common.hpp"
#include "utils/reshape.hpp"

namespace ngraph
{
    namespace onnx_import
    {
        namespace op
        {
            namespace detail
            {
                namespace
                {
                    // This function creates a shape to which we need to reshape the input
                    // before normalization.
                    // If data shape is [N,C,H,W], the function returns
                    // [N, num_groups, C // num_groups, H, W]
                    std::shared_ptr<ngraph::Node>
                        create_group_norm_shape(const Output<ngraph::Node>& data, size_t num_groups)
                    {
                        const auto& pshape = data.get_partial_shape();
                        NGRAPH_CHECK(pshape.rank().is_static());
                        size_t rank_size = pshape.rank().get_length();
                        NGRAPH_CHECK(rank_size >= 3, "3-D and above tensors supported only");

                        auto shape = std::make_shared<default_opset::ShapeOf>(data);
                        auto splits = builder::opset1::split(shape, rank_size);
                        auto num_groups_const =
                            default_opset::Constant::create(element::i64, Shape{1}, {num_groups});
                        ngraph::OutputVector new_shape{
                            splits[0],
                            num_groups_const,
                            std::make_shared<default_opset::Divide>(splits[1], num_groups_const)};

                        for (size_t i = 2; i < rank_size; i++)
                        {
                            new_shape.push_back(splits[i]);
                        }
                        return std::make_shared<default_opset::Concat>(new_shape, 0);
                    }
                }
            } // detail

            namespace set_1
            {
                OutputVector group_norm(const Node& node)
                {
                    auto inputs = node.get_ng_inputs();
                    NGRAPH_CHECK(inputs.size() == 3,
                                 "Invalid number of inputs. Expected 3, actual " +
                                     std::to_string(inputs.size()));

                    auto data = inputs[0];
                    auto scale = inputs[1];
                    auto bias = inputs[2];

                    size_t num_groups =
                        static_cast<size_t>(node.get_attribute_value<int64_t>("num_groups"));
                    float eps = node.get_attribute_value<float>("eps", 1e-6);

                    auto data_shape_node = std::make_shared<default_opset::ShapeOf>(data);
                    auto data_reshaped = std::make_shared<default_opset::Reshape>(
                        data, detail::create_group_norm_shape(data, num_groups), true);
                    const auto reduction_axes =
                        common::get_monotonic_range_along_node_rank(data_reshaped, 2);

                    auto mvn =
                        std::make_shared<default_opset::MVN>(data_reshaped,
                                                             reduction_axes,
                                                             true,
                                                             eps,
                                                             ngraph::op::MVNEpsMode::INSIDE_SQRT);
                    std::shared_ptr<ngraph::Node> result =
                        std::make_shared<default_opset::Reshape>(mvn, data_shape_node, true);

                    const auto& rank = data.get_partial_shape().rank();
                    NGRAPH_CHECK(rank.is_static());
                    auto data_rank_size = rank.get_length();

                    result = std::make_shared<default_opset::Multiply>(
                        result,
                        reshape::reshape_channel_shaped_node_to_nchw(scale, data_rank_size));
                    result = std::make_shared<default_opset::Add>(
                        result, reshape::reshape_channel_shaped_node_to_nchw(bias, data_rank_size));

                    return {result};
                }

            } // namespace set_1

        } // namespace op

    } // namespace onnx_import

} // namespace ngraph

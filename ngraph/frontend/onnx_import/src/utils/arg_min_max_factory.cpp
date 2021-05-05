// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "utils/arg_min_max_factory.hpp"
#include "default_opset.hpp"
#include "ngraph/validation_util.hpp"

namespace ngraph
{
    namespace onnx_import
    {
        namespace utils
        {
            ArgMinMaxFactory::ArgMinMaxFactory(const Node& node)
                : m_keep_dims{node.get_attribute_value<std::int64_t>("keepdims", 1)}
                , m_axis{node.get_attribute_value<std::int64_t>("axis", 0)}
            {
                m_input_node = node.get_ng_inputs().at(0);
            }

            std::shared_ptr<ngraph::Node> ArgMinMaxFactory::make_arg_max() const
            {
                return make_topk_subgraph(default_opset::TopK::Mode::MAX);
            }

            std::shared_ptr<ngraph::Node> ArgMinMaxFactory::make_arg_min() const
            {
                return make_topk_subgraph(default_opset::TopK::Mode::MIN);
            }

            std::shared_ptr<ngraph::Node>
                ArgMinMaxFactory::make_topk_subgraph(default_opset::TopK::Mode mode) const
            {
                const auto k_node =
                    default_opset::Constant::create(ngraph::element::i64, Shape{}, {1});
                const auto topk = std::make_shared<default_opset::TopK>(
                    m_input_node, k_node, m_axis, mode, default_opset::TopK::SortType::NONE);

                if (m_keep_dims == 0)
                {
                    const auto axis_to_remove =
                        default_opset::Constant::create(element::u64, Shape{}, {topk->get_axis()});
                    const auto reshaped_indices =
                        std::make_shared<default_opset::Squeeze>(topk->output(1), axis_to_remove);

                    return std::make_shared<default_opset::Convert>(reshaped_indices, element::i64);
                }
                return std::make_shared<default_opset::Convert>(topk->output(1), element::i64);
            }
        } // namespace utils
    }     // namespace onnx_import
} // namespace ngraph

// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <ngraph/pass/pass.hpp>

#include "ie_input_info.hpp"

namespace ngraph {
namespace pass {

class AddPreprocessing;

}  // namespace pass
}  // namespace ngraph

/**
 * @brief Converts the following preprocessing information to ngraph operations:
 *  - InferenceEngine::PreProcessInfo->PreProcessChannel::meanData -> Subtract
 *  - InferenceEngine::PreProcessInfo->PreProcessChannel::meanValue -> Subtract
 *  - InferenceEngine::PreProcessInfo->PreProcessChannel::stdScale -> Multiply
 *
 * The order of operations is the following:
 *      (x - mean) * stdScale
 */
class ngraph::pass::AddPreprocessing : public ngraph::pass::FunctionPass {
    const InferenceEngine::InputsDataMap& m_inputInfoMap;

public:
    NGRAPH_RTTI_DECLARATION;
    explicit AddPreprocessing(const InferenceEngine::InputsDataMap& inputInfoMap);

    bool run_on_function(std::shared_ptr<ngraph::Function> f) override;
};

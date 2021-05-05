// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <ie_common.h>
#include <mkldnn_node.h>
#include <string>
#include "common/permute_kernel.h"

namespace MKLDNNPlugin {

class MKLDNNSpaceToDepthNode : public MKLDNNNode {
public:
    MKLDNNSpaceToDepthNode(const InferenceEngine::CNNLayerPtr& layer, const mkldnn::engine& eng, MKLDNNWeightsSharing::Ptr &cache);
    ~MKLDNNSpaceToDepthNode() override = default;

    void getSupportedDescriptors() override;
    void initSupportedPrimitiveDescriptors() override;
    void createPrimitive() override;
    void execute(mkldnn::stream strm) override;
    bool created() const override;

private:
    enum Mode {
        BLOCKS_FIRST = 0,
        DEPTH_FIRST = 1
    };

    Mode mode;
    size_t blockSize;
    size_t blockStep;

    std::unique_ptr<PermuteKernel> permuteKernel;
};

}  // namespace MKLDNNPlugin

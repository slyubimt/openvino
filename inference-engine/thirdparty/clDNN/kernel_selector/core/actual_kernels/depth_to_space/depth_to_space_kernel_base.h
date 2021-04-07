// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "kernel_base_opencl.h"
#include "kernel_selector_params.h"

namespace kernel_selector {
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// depth_to_space_params
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct depth_to_space_params : public base_params {
    depth_to_space_params()
    : base_params(KernelType::DEPTH_TO_SPACE)
    , block_size(0)
    , mode(DepthToSpaceMode::DEPTH_FIRST) {}
    size_t block_size;
    DepthToSpaceMode mode;

    virtual ParamsKey GetParamsKey() const { return base_params::GetParamsKey(); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// depth_to_space_optional_params
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct depth_to_space_optional_params : optional_params {
    depth_to_space_optional_params() : optional_params(KernelType::DEPTH_TO_SPACE) {}
};

struct depth_to_space_fuse_params : fuse_params {
    depth_to_space_fuse_params() : fuse_params(KernelType::DEPTH_TO_SPACE) {}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DepthToSpaceKernelBase
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class DepthToSpaceKernelBase : public KernelBaseOpenCL {
public:
    using KernelBaseOpenCL::KernelBaseOpenCL;
    virtual ~DepthToSpaceKernelBase() {}

    struct DispatchData : public CommonDispatchData {
    };

protected:
    virtual bool Validate(const Params&, const optional_params&) const;
    virtual JitConstants GetJitConstants(const depth_to_space_params& params) const;
    virtual CommonDispatchData SetDefault(const depth_to_space_params& params) const;
    KernelsData GetCommonKernelsData(const Params& params, const optional_params&) const;
};
}  // namespace kernel_selector

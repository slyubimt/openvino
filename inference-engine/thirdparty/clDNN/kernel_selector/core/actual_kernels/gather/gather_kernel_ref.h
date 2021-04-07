// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "kernel_base_opencl.h"

namespace kernel_selector {
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// gather_params
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct gather_params : public base_params {
    gather_params() : base_params(KernelType::GATHER), axis(GatherAxis::BATCH) {}

    GatherAxis axis;

    virtual ParamsKey GetParamsKey() const { return base_params::GetParamsKey(); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// gather_optional_params
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct gather_optional_params : optional_params {
    gather_optional_params() : optional_params(KernelType::GATHER) {}
};

class GatherKernelRef : public KernelBaseOpenCL {
public:
    GatherKernelRef() : KernelBaseOpenCL("gather_ref") {}
    virtual ~GatherKernelRef() {}
    virtual JitConstants GetJitConstants(const gather_params& params) const;
    virtual CommonDispatchData SetDefault(const gather_params& params, const optional_params&) const;
    KernelsData GetKernelsData(const Params& params, const optional_params& options) const override;
    KernelsPriority GetKernelsPriority(const Params& params, const optional_params& options) const override;
    ParamsKey GetSupportedKey() const override;
    std::vector<FusedOpType> GetSupportedFusedOps() const override {
        return { FusedOpType::QUANTIZE,
                 FusedOpType::SCALE,
                 FusedOpType::ACTIVATION };
    }

protected:
    bool Validate(const Params& p, const optional_params& o) const override;
};
}  // namespace kernel_selector

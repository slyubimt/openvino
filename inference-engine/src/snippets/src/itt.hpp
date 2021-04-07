// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

/**
 * @brief Defines openvino domains for tracing
 * @file itt.hpp
 */

#pragma once

#include <openvino/cc/selective_build.h>
#include <openvino/itt.hpp>

namespace ngraph {
namespace pass {
namespace itt {
namespace domains {
    OV_ITT_DOMAIN(IETransform);
}   // namespace domains
}   // namespace itt
}   // namespace pass
}   // namespace ngraph

OV_CC_DOMAINS(ngraph_pass);
OV_CC_DOMAINS(internal_op);

/*
 * RUN_ON_FUNCTION_SCOPE macro allows to disable the run_on_function pass
 * MATCHER_SCOPE macro allows to disable the MatcherPass if matcher isn't applied
 * INTERNAL_OP_SCOPE macro allows to disable parts of internal nGraph operations if they are not used
 */
#if defined(SELECTIVE_BUILD_ANALYZER)
#define RUN_ON_FUNCTION_SCOPE(region) OV_SCOPE(ngraph_pass, OV_PP_CAT(region, _run_on_function))
#define MATCHER_SCOPE(region)                                                                   \
    const std::string matcher_name(OV_PP_TOSTRING(region))

#define INTERNAL_OP_SCOPE(region) OV_SCOPE(internal_op, region)

#elif defined(SELECTIVE_BUILD)

#define MATCHER_SCOPE_(scope, region)                                                           \
    if (OV_CC_SCOPE_IS_ENABLED(OV_PP_CAT3(scope, _, region)) == 0)                              \
    throw ngraph::ngraph_error(std::string(OV_PP_TOSTRING(OV_PP_CAT3(scope, _, region))) +      \
                               " is disabled!")

#define MATCHER_SCOPE(region)                                                                   \
    const std::string matcher_name(OV_PP_TOSTRING(region));                                           \
    if (OV_CC_SCOPE_IS_ENABLED(OV_PP_CAT3(ngraph_pass, _, region)) == 0)                        \
        return
#define INTERNAL_OP_SCOPE(region) MATCHER_SCOPE_(internal_op, region)
#define RUN_ON_FUNCTION_SCOPE(region) MATCHER_SCOPE_(ngraph_pass, OV_PP_CAT(region, _run_on_function))

#else
#define MATCHER_SCOPE(region)                                                                   \
    const std::string matcher_name(OV_PP_TOSTRING(region))
#define INTERNAL_OP_SCOPE(region)
#define RUN_ON_FUNCTION_SCOPE(region)
#endif

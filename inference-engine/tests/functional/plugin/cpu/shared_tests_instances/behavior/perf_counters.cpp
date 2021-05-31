// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "auto_plugin/auto_config.hpp"
#include "multi-device/multi_device_config.hpp"
#include "behavior/perf_counters.hpp"

using namespace BehaviorTestsDefinitions;
namespace {
    const std::vector<std::map<std::string, std::string>> configs = {
            {}
    };

    const std::vector<std::map<std::string, std::string>> Multiconfigs = {
            {{ MULTI_CONFIG_KEY(DEVICE_PRIORITIES) , CommonTestUtils::DEVICE_CPU}}
    };

    const std::vector<std::map<std::string, std::string>> Autoconfigs = {
            {{ AUTO_CONFIG_KEY(DEVICE_LIST) , CommonTestUtils::DEVICE_CPU}}
    };

    INSTANTIATE_TEST_CASE_P(smoke_BehaviorTests, PerfCountersTest,
                            ::testing::Combine(
                                    ::testing::Values(InferenceEngine::Precision::FP32),
                                    ::testing::Values(CommonTestUtils::DEVICE_CPU),
                                    ::testing::ValuesIn(configs)),
                            PerfCountersTest::getTestCaseName);

    INSTANTIATE_TEST_CASE_P(smoke_Multi_BehaviorTests, PerfCountersTest,
                            ::testing::Combine(
                                    ::testing::Values(InferenceEngine::Precision::FP32),
                                    ::testing::Values(CommonTestUtils::DEVICE_MULTI),
                                    ::testing::ValuesIn(Multiconfigs)),
                            PerfCountersTest::getTestCaseName);

    INSTANTIATE_TEST_CASE_P(smoke_Auto_BehaviorTests, PerfCountersTest,
                            ::testing::Combine(
                                    ::testing::Values(InferenceEngine::Precision::FP32),
                                    ::testing::Values(CommonTestUtils::DEVICE_AUTO),
                                    ::testing::ValuesIn(Autoconfigs)),
                            PerfCountersTest::getTestCaseName);

}  // namespace

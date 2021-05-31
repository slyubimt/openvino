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
            {{ MULTI_CONFIG_KEY(DEVICE_PRIORITIES) , CommonTestUtils::DEVICE_GPU}}
    };

    const std::vector<std::map<std::string, std::string>> Autoconfigs = {
            {{ AUTO_CONFIG_KEY(DEVICE_LIST) , CommonTestUtils::DEVICE_GPU}}
    };

    const std::vector<std::map<std::string, std::string>> auto_cpu_gpu_conf = {
        {{InferenceEngine::AutoConfigParams::KEY_AUTO_DEVICE_LIST , std::string(CommonTestUtils::DEVICE_CPU) + "," + CommonTestUtils::DEVICE_GPU}}
    };

    INSTANTIATE_TEST_CASE_P(smoke_BehaviorTests, PerfCountersTest,
                            ::testing::Combine(
                                    ::testing::Values(InferenceEngine::Precision::FP32),
                                    ::testing::Values(CommonTestUtils::DEVICE_GPU),
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

    INSTANTIATE_TEST_CASE_P(smoke_AutoCG_BehaviorTests, PerfCountersTest,
                            ::testing::Combine(
                                ::testing::Values(InferenceEngine::Precision::FP32),
                                ::testing::Values(CommonTestUtils::DEVICE_AUTO),
                                ::testing::ValuesIn(auto_cpu_gpu_conf)),
                            PerfCountersTest::getTestCaseName);

}  // namespace

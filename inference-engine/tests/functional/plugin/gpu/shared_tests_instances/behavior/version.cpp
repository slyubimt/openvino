// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "auto_plugin/auto_config.hpp"
#include "multi-device/multi_device_config.hpp"
#include "hetero/hetero_plugin_config.hpp"
#include "behavior/version.hpp"

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

    const std::vector<std::map<std::string, std::string>> Heteroconfigs = {
            {{ HETERO_CONFIG_KEY(DUMP_GRAPH_DOT) , CommonTestUtils::DEVICE_GPU}}
    };

    INSTANTIATE_TEST_CASE_P(smoke_BehaviorTests, VersionTest,
                            ::testing::Combine(
                                    ::testing::Values(InferenceEngine::Precision::FP32),
                                    ::testing::Values(CommonTestUtils::DEVICE_GPU),
                                    ::testing::ValuesIn(configs)),
                            VersionTest::getTestCaseName);

    INSTANTIATE_TEST_CASE_P(smoke_Multi_BehaviorTests, VersionTest,
                            ::testing::Combine(
                                    ::testing::Values(InferenceEngine::Precision::FP32),
                                    ::testing::Values(CommonTestUtils::DEVICE_MULTI),
                                    ::testing::ValuesIn(Multiconfigs)),
                            VersionTest::getTestCaseName);

    INSTANTIATE_TEST_CASE_P(smoke_Auto_BehaviorTests, VersionTest,
                            ::testing::Combine(
                                    ::testing::Values(InferenceEngine::Precision::FP32),
                                    ::testing::Values(CommonTestUtils::DEVICE_AUTO),
                                    ::testing::ValuesIn(Autoconfigs)),
                            VersionTest::getTestCaseName);

    INSTANTIATE_TEST_CASE_P(smoke_AutoCG_BehaviorTests, VersionTest,
                            ::testing::Combine(
                                ::testing::Values(InferenceEngine::Precision::FP32),
                                ::testing::Values(CommonTestUtils::DEVICE_AUTO),
                                ::testing::ValuesIn(auto_cpu_gpu_conf)),
                            VersionTest::getTestCaseName);

    INSTANTIATE_TEST_CASE_P(smoke_Hetero_BehaviorTests, VersionTest,
                            ::testing::Combine(
                                    ::testing::Values(InferenceEngine::Precision::FP32),
                                    ::testing::Values(CommonTestUtils::DEVICE_HETERO),
                                    ::testing::ValuesIn(Heteroconfigs)),
                            VersionTest::getTestCaseName);


}  // namespace

// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "auto_plugin/auto_config.hpp"
#include "multi-device/multi_device_config.hpp"

#include "behavior/config.hpp"

using namespace BehaviorTestsDefinitions;
namespace {
    const std::vector<InferenceEngine::Precision> netPrecisions = {
            InferenceEngine::Precision::FP32,
            InferenceEngine::Precision::FP16
    };

    const std::vector<std::map<std::string, std::string>> inconfigs = {
            {{InferenceEngine::PluginConfigParams::KEY_GPU_THROUGHPUT_STREAMS, "OFF"}},
            {{InferenceEngine::PluginConfigParams::KEY_PERF_COUNT, "ON"}},
            {{InferenceEngine::PluginConfigParams::KEY_CONFIG_FILE, "unknown_file"}},
            {{InferenceEngine::PluginConfigParams::KEY_DUMP_KERNELS, "ON"}},
            {{InferenceEngine::PluginConfigParams::KEY_TUNING_MODE, "TUNING_UNKNOWN_MODE"}},
            {{InferenceEngine::PluginConfigParams::KEY_DEVICE_ID, "DEVICE_UNKNOWN"}}
    };

    const std::vector<std::map<std::string, std::string>> multiinconfigs = {
            {{InferenceEngine::MultiDeviceConfigParams::KEY_MULTI_DEVICE_PRIORITIES , CommonTestUtils::DEVICE_GPU},
                    {InferenceEngine::PluginConfigParams::KEY_PERF_COUNT, "ON"}},
            {{InferenceEngine::MultiDeviceConfigParams::KEY_MULTI_DEVICE_PRIORITIES , CommonTestUtils::DEVICE_GPU},
                    {InferenceEngine::PluginConfigParams::KEY_CONFIG_FILE, "unknown_file"}},
            {{InferenceEngine::MultiDeviceConfigParams::KEY_MULTI_DEVICE_PRIORITIES , CommonTestUtils::DEVICE_GPU},
                    {InferenceEngine::PluginConfigParams::KEY_DUMP_KERNELS, "ON"}},
            {{InferenceEngine::MultiDeviceConfigParams::KEY_MULTI_DEVICE_PRIORITIES , CommonTestUtils::DEVICE_GPU},
                    {InferenceEngine::PluginConfigParams::KEY_TUNING_MODE, "TUNING_UNKNOWN_MODE"}},
            {{InferenceEngine::MultiDeviceConfigParams::KEY_MULTI_DEVICE_PRIORITIES , CommonTestUtils::DEVICE_GPU},
                    {InferenceEngine::PluginConfigParams::KEY_DEVICE_ID, "DEVICE_UNKNOWN"}}
    };

    const std::vector<std::map<std::string, std::string>> autoinconfigs = {
            {{InferenceEngine::AutoConfigParams::KEY_AUTO_DEVICE_LIST , CommonTestUtils::DEVICE_GPU},
                    {InferenceEngine::PluginConfigParams::KEY_PERF_COUNT, "ON"}},
            {{InferenceEngine::AutoConfigParams::KEY_AUTO_DEVICE_LIST , CommonTestUtils::DEVICE_GPU},
                    {InferenceEngine::PluginConfigParams::KEY_CONFIG_FILE, "unknown_file"}},
            {{InferenceEngine::AutoConfigParams::KEY_AUTO_DEVICE_LIST , CommonTestUtils::DEVICE_GPU},
                    {InferenceEngine::PluginConfigParams::KEY_DUMP_KERNELS, "ON"}},
            {{InferenceEngine::AutoConfigParams::KEY_AUTO_DEVICE_LIST , CommonTestUtils::DEVICE_GPU},
                    {InferenceEngine::PluginConfigParams::KEY_TUNING_MODE, "TUNING_UNKNOWN_MODE"}},
            {{InferenceEngine::AutoConfigParams::KEY_AUTO_DEVICE_LIST , CommonTestUtils::DEVICE_GPU},
                    {InferenceEngine::PluginConfigParams::KEY_DEVICE_ID, "DEVICE_UNKNOWN"}}
    };

    INSTANTIATE_TEST_CASE_P(smoke_BehaviorTests, IncorrectConfigTests,
            ::testing::Combine(
                ::testing::ValuesIn(netPrecisions),
                ::testing::Values(CommonTestUtils::DEVICE_GPU),
                ::testing::ValuesIn(inconfigs)),
            IncorrectConfigTests::getTestCaseName);

    INSTANTIATE_TEST_CASE_P(smoke_Multi_BehaviorTests, IncorrectConfigTests,
            ::testing::Combine(
                ::testing::ValuesIn(netPrecisions),
                ::testing::Values(CommonTestUtils::DEVICE_MULTI),
                ::testing::ValuesIn(multiinconfigs)),
            IncorrectConfigTests::getTestCaseName);

    INSTANTIATE_TEST_CASE_P(smoke_Auto_BehaviorTests, IncorrectConfigTests,
            ::testing::Combine(
                ::testing::ValuesIn(netPrecisions),
                ::testing::Values(CommonTestUtils::DEVICE_AUTO),
                ::testing::ValuesIn(autoinconfigs)),
            IncorrectConfigTests::getTestCaseName);


    const std::vector<std::map<std::string, std::string>> conf = {
            {}
    };

    const std::vector<std::map<std::string, std::string>> multiconf = {
            {{InferenceEngine::MultiDeviceConfigParams::KEY_MULTI_DEVICE_PRIORITIES , CommonTestUtils::DEVICE_GPU}}
    };

    const std::vector<std::map<std::string, std::string>> autoconf = {
            {{InferenceEngine::AutoConfigParams::KEY_AUTO_DEVICE_LIST , CommonTestUtils::DEVICE_GPU}}
    };

    const std::vector<std::map<std::string, std::string>> auto_cpu_gpu_conf = {
        {{InferenceEngine::AutoConfigParams::KEY_AUTO_DEVICE_LIST , std::string(CommonTestUtils::DEVICE_CPU) + "," + CommonTestUtils::DEVICE_GPU}}
    };

    INSTANTIATE_TEST_CASE_P(smoke_BehaviorTests, CorrectConfigAPITests,
            ::testing::Combine(
                ::testing::ValuesIn(netPrecisions),
                ::testing::Values(CommonTestUtils::DEVICE_GPU),
                ::testing::ValuesIn(conf)),
            CorrectConfigAPITests::getTestCaseName);

    INSTANTIATE_TEST_CASE_P(smoke_Multi_BehaviorTests, CorrectConfigAPITests,
            ::testing::Combine(
                    ::testing::ValuesIn(netPrecisions),
                    ::testing::Values(CommonTestUtils::DEVICE_MULTI),
                    ::testing::ValuesIn(multiconf)),
            CorrectConfigAPITests::getTestCaseName);

    INSTANTIATE_TEST_CASE_P(smoke_Auto_BehaviorTests, CorrectConfigAPITests,
            ::testing::Combine(
                    ::testing::ValuesIn(netPrecisions),
                    ::testing::Values(CommonTestUtils::DEVICE_AUTO),
                    ::testing::ValuesIn(autoconf)),
            CorrectConfigAPITests::getTestCaseName);

    INSTANTIATE_TEST_CASE_P(smoke_BehaviorTests, IncorrectConfigAPITests,
            ::testing::Combine(
                    ::testing::ValuesIn(netPrecisions),
                    ::testing::Values(CommonTestUtils::DEVICE_GPU),
                    ::testing::ValuesIn(conf)),
             IncorrectConfigAPITests::getTestCaseName);

    INSTANTIATE_TEST_CASE_P(smoke_Multi_BehaviorTests, IncorrectConfigAPITests,
            ::testing::Combine(
                    ::testing::ValuesIn(netPrecisions),
                    ::testing::Values(CommonTestUtils::DEVICE_MULTI),
                    ::testing::ValuesIn(multiconf)),
            IncorrectConfigAPITests::getTestCaseName);

    INSTANTIATE_TEST_CASE_P(smoke_Auto_BehaviorTests, IncorrectConfigAPITests,
            ::testing::Combine(
                    ::testing::ValuesIn(netPrecisions),
                    ::testing::Values(CommonTestUtils::DEVICE_AUTO),
                    ::testing::ValuesIn(autoconf)),
            IncorrectConfigAPITests::getTestCaseName);

    INSTANTIATE_TEST_CASE_P(smoke_AutoCG_BehaviorTests, IncorrectConfigAPITests,
                            ::testing::Combine(
                                ::testing::ValuesIn(netPrecisions),
                                ::testing::Values(CommonTestUtils::DEVICE_AUTO),
                                ::testing::ValuesIn(auto_cpu_gpu_conf)),
                            IncorrectConfigAPITests::getTestCaseName);


} // namespace
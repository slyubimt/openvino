// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <gmock/gmock.h>
#include "ie_icore.hpp"

class MockICore : public InferenceEngine::ICore {
public:
    MOCK_QUALIFIED_METHOD0(GetTaskExecutor, const, std::shared_ptr<InferenceEngine::ITaskExecutor>());

    MOCK_QUALIFIED_METHOD2(ReadNetwork, const, InferenceEngine::CNNNetwork(const std::string&, const InferenceEngine::Blob::CPtr&));
    MOCK_QUALIFIED_METHOD2(ReadNetwork, const, InferenceEngine::CNNNetwork(const std::string&, const std::string&));

    MOCK_METHOD3(LoadNetwork, InferenceEngine::ExecutableNetwork(
        const InferenceEngine::CNNNetwork&, const std::string&, const std::map<std::string, std::string>&));
    MOCK_METHOD3(LoadNetwork, InferenceEngine::ExecutableNetwork(
        const InferenceEngine::CNNNetwork&, const InferenceEngine::RemoteContext::Ptr &, const std::map<std::string, std::string>&));

    MOCK_METHOD3(ImportNetwork, InferenceEngine::ExecutableNetwork(
        std::istream&, const std::string&, const std::map<std::string, std::string>&));
    MOCK_METHOD3(ImportNetwork, InferenceEngine::ExecutableNetwork(
        std::istream&, const InferenceEngine::RemoteContext::Ptr&, const std::map<std::string, std::string>&));

    MOCK_QUALIFIED_METHOD3(QueryNetwork, const, InferenceEngine::QueryNetworkResult(
        const InferenceEngine::CNNNetwork&, const std::string&, const std::map<std::string, std::string>&));

    MOCK_QUALIFIED_METHOD2(GetMetric, const, InferenceEngine::Parameter(const std::string&, const std::string&));
    MOCK_QUALIFIED_METHOD0(GetAvailableDevices, const, std::vector<std::string>());

    ~MockICore() = default;
};

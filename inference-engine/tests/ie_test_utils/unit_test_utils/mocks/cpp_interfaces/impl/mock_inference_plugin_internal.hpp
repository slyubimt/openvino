// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <map>
#include <memory>

#include "mock_executable_network_internal.hpp"
#include "ie_icore.hpp"
#include <gmock/gmock.h>
#include <string>
#include <vector>
#include <cpp_interfaces/impl/ie_plugin_internal.hpp>
#include <cpp_interfaces/impl/ie_executable_network_internal.hpp>

class MockInferencePluginInternal2 : public InferenceEngine::InferencePluginInternal {
public:
    MOCK_METHOD2(LoadExeNetworkImpl, std::shared_ptr<InferenceEngine::ExecutableNetworkInternal>(
            const InferenceEngine::CNNNetwork &, const std::map<std::string, std::string> &));
    MOCK_METHOD2(LoadNetwork, std::shared_ptr<InferenceEngine::IExecutableNetworkInternal>(
            const InferenceEngine::CNNNetwork &,
            const std::map<std::string, std::string> &));
    MOCK_METHOD1(AddExtension, void(InferenceEngine::IExtensionPtr ext_ptr));
    MOCK_METHOD1(SetConfig, void(const std::map <std::string, std::string> &));
};

class MockInferencePluginInternal : public InferenceEngine::InferencePluginInternal {
public:
    MOCK_METHOD2(LoadExeNetworkImpl, std::shared_ptr<InferenceEngine::ExecutableNetworkInternal>(
            const InferenceEngine::CNNNetwork &, const std::map<std::string, std::string> &));
    MOCK_METHOD1(AddExtension, void(InferenceEngine::IExtensionPtr ext_ptr));
    MOCK_METHOD1(SetConfig, void(const std::map <std::string, std::string> &));

    using InferenceEngine::InferencePluginInternal::ImportNetwork;

    ExecutableNetworkInternal::Ptr ImportNetworkImpl(std::istream& stream, const std::map <std::string, std::string>&) {
        std::getline(stream, importedString);
        return {};
    }

    std::string importedString;
};

class MockInferencePluginInternal3 : public InferenceEngine::InferencePluginInternal {
public:
    MOCK_METHOD2(LoadExeNetworkImpl, std::shared_ptr<InferenceEngine::ExecutableNetworkInternal>(
            const InferenceEngine::CNNNetwork &, const std::map<std::string, std::string> &));
    MOCK_METHOD1(AddExtension, void(InferenceEngine::IExtensionPtr ext_ptr));
    MOCK_METHOD1(SetConfig, void(const std::map <std::string, std::string> &));
};

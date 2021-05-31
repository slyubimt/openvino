// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "shared_test_classes/subgraph/parameter_result.hpp"
#include "common_test_utils/test_constants.hpp"

using namespace SubgraphTestsDefinitions;
using namespace InferenceEngine;

namespace CPULayerTestsDefinitions {

class ParameterResultCustomBlobTest : public ParameterResultSubgraphTest {
 protected:
    void Infer() override {
        constexpr size_t inferIterations = 10lu;

        inferRequest = executableNetwork.CreateInferRequest();

        auto inputBlob = inputs.front();
        const size_t elementsCount = inputBlob->size();
        for (size_t i = 0; i < inferIterations; ++i) {
            CommonTestUtils::fill_data_random<Precision::FP32>(inputBlob, 10, 0, 1, i);
            const auto& inputsInfo = cnnNetwork.getInputsInfo().begin()->second;
            std::string inputName = cnnNetwork.getInputsInfo().begin()->first;

            float* customInpData = new float[elementsCount];
            auto inpBlobData = inputBlob->buffer().as<const float *>();
            std::copy(inpBlobData, inpBlobData + elementsCount, customInpData);

            auto& tensorDesc = inputsInfo->getTensorDesc();
            auto customBlob = make_shared_blob<float>(tensorDesc, customInpData, elementsCount * sizeof(float));
            inferRequest.SetBlob(inputName, customBlob);

            inferRequest.Infer();

            ParameterResultSubgraphTest::Validate();

            delete[] customInpData;
        }
    }
    void Validate() override {
        //Do nothing. We call Validate() in the Infer() method
    }
};

TEST_P(ParameterResultCustomBlobTest, CompareWithRefs) {
    SKIP_IF_CURRENT_TEST_IS_DISABLED()

    // Just to show that it is not possible to set different precisions for inputs and outputs with the same name.
    // If it was possible, the input would have I8 precision and couldn't store data from the custom blob.
    inPrc = Precision::I8;
    outPrc = Precision::FP32;

    Run();
}
namespace {
    INSTANTIATE_TEST_CASE_P(smoke_Check_Custom_Blob, ParameterResultCustomBlobTest,
                            ::testing::Values(CommonTestUtils::DEVICE_CPU),
                            ParameterResultSubgraphTest::getTestCaseName);
} // namespace

class ParameterResultSameBlobTest : public ParameterResultSubgraphTest {
protected:
    void Infer() override {
        constexpr size_t inferIterations = 10lu;

        for (size_t i = 0; i < inferIterations; ++i) {
            ParameterResultSubgraphTest::Infer();
            ParameterResultSubgraphTest::Validate();
        }
    }
    void Validate() override {
        //Do nothing. We call Validate() in the Infer() method
    }
};

TEST_P(ParameterResultSameBlobTest, CompareWithRefs) {
    SKIP_IF_CURRENT_TEST_IS_DISABLED()

    Run();
}
namespace {
    INSTANTIATE_TEST_CASE_P(smoke_Check_Same_Blob, ParameterResultSameBlobTest,
                            ::testing::Values(CommonTestUtils::DEVICE_CPU),
                            ParameterResultSubgraphTest::getTestCaseName);
} // namespace
} // namespace CPULayerTestsDefinitions
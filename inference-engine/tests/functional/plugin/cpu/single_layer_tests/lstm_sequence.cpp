// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "shared_test_classes/single_layer/lstm_sequence.hpp"
#include "ngraph/pass/visualize_tree.hpp"
#include "test_utils/cpu_test_utils.hpp"
#include "transformations/op_conversions/bidirectional_sequences_decomposition.hpp"
#include "transformations/op_conversions/convert_sequences_to_tensor_iterator.hpp"

using namespace InferenceEngine;
using namespace CPUTestUtils;

namespace CPULayerTestsDefinitions {

using LSTMSequenceCpuSpecificParams = typename std::tuple<LayerTestsDefinitions::LSTMSequenceParams, CPUSpecificParams, std::map<std::string, std::string>>;

class LSTMSequenceCPUTest : public testing::WithParamInterface<LSTMSequenceCpuSpecificParams>,
                            virtual public LayerTestsUtils::LayerTestsCommon,
                            public CPUTestsBase {
public:
    static std::string getTestCaseName(const testing::TestParamInfo<LSTMSequenceCpuSpecificParams> &obj) {
        CPUSpecificParams cpuParams;
        LayerTestsDefinitions::LSTMSequenceParams basicParamsSet;
        std::map<std::string, std::string> additionalConfig;

        std::tie(basicParamsSet, cpuParams, additionalConfig) = obj.param;
        std::ostringstream result;

        result << LayerTestsDefinitions::LSTMSequenceTest::getTestCaseName(
            testing::TestParamInfo<LayerTestsDefinitions::LSTMSequenceParams>(basicParamsSet, 0));
        result << CPUTestsBase::getTestCaseName(cpuParams);

        if (!additionalConfig.empty()) {
            result << "_PluginConf";
            for (auto &item : additionalConfig) {
                if (item.second == PluginConfigParams::YES)
                    result << "_" << item.first << "=" << item.second;
            }
        }
        return result.str();
    }

protected:
    void SetUp() {
        LayerTestsDefinitions::LSTMSequenceParams basicParamsSet;
        CPUSpecificParams cpuParams;
        std::map<std::string, std::string> additionalConfig;

        size_t seq_lenghts;
        size_t batch;
        size_t hidden_size;
        size_t input_size;
        std::vector<std::string> activations;
        std::vector<float> activations_alpha;
        std::vector<float> activations_beta;
        float clip;
        ngraph::op::RecurrentSequenceDirection direction;
        InferenceEngine::Precision netPrecision;

        std::tie(basicParamsSet, cpuParams, additionalConfig) = this->GetParam();
        std::tie(inFmts, outFmts, priority, selectedType) = cpuParams;
        std::tie(m_mode, seq_lenghts, batch, hidden_size, input_size, activations, clip, direction, netPrecision, targetDevice) = basicParamsSet;

        size_t num_directions = direction == ngraph::op::RecurrentSequenceDirection::BIDIRECTIONAL ? 2 : 1;
        m_max_seq_len = seq_lenghts;
        std::vector<std::vector<size_t>> inputShapes = {
            {{batch, seq_lenghts, input_size},
             {batch, num_directions, hidden_size},
             {batch, num_directions, hidden_size},
             {batch},
             {num_directions, 4 * hidden_size, input_size},
             {num_directions, 4 * hidden_size, hidden_size},
             {num_directions, 4 * hidden_size}},
        };

        configuration.insert(additionalConfig.begin(), additionalConfig.end());

        if (additionalConfig[PluginConfigParams::KEY_ENFORCE_BF16] == PluginConfigParams::YES) {
            inPrc = outPrc = Precision::BF16;
        } else {
            inPrc = outPrc = netPrecision;
        }

        selectedType += "_";
        selectedType += outPrc.name();

        auto ngPrc = FuncTestUtils::PrecisionUtils::convertIE2nGraphPrc(Precision::FP32);
        auto params = ngraph::builder::makeParams(ngPrc, {inputShapes[0], inputShapes[1], inputShapes[2]});
        if (m_mode == ngraph::helpers::SequenceTestsMode::CONVERT_TO_TI_MAX_SEQ_LEN_PARAM
            || m_mode == ngraph::helpers::SequenceTestsMode::CONVERT_TO_TI_RAND_SEQ_LEN_PARAM) {
            auto seq_lengths = ngraph::builder::makeParams(ngraph::element::i64, {inputShapes[3]}).at(0);
            seq_lengths->set_friendly_name("seq_lengths");
            params.push_back(seq_lengths);
        }
        std::vector<ngraph::Shape> WRB = {inputShapes[4], inputShapes[5], inputShapes[6], inputShapes[3]};
        auto lstm_sequence = ngraph::builder::makeLSTM(ngraph::helpers::convert2OutputVector(ngraph::helpers::castOps2Nodes(params)),
                                                       WRB,
                                                       hidden_size,
                                                       activations,
                                                       {},
                                                       {},
                                                       clip,
                                                       true,
                                                       direction,
                                                       m_mode);
        ngraph::ResultVector results{std::make_shared<ngraph::opset1::Result>(lstm_sequence->output(0)),
                                     std::make_shared<ngraph::opset1::Result>(lstm_sequence->output(1)),
                                     std::make_shared<ngraph::opset1::Result>(lstm_sequence->output(2))};

        function = makeNgraphFunction(ngPrc, params, lstm_sequence, "lstm_sequence");

        if (m_mode != ngraph::helpers::SequenceTestsMode::PURE_SEQ) {
            ngraph::pass::Manager manager;
            if (direction == ngraph::op::RecurrentSequenceDirection::BIDIRECTIONAL)
                manager.register_pass<ngraph::pass::BidirectionalLSTMSequenceDecomposition>();
            manager.register_pass<ngraph::pass::ConvertLSTMSequenceToTensorIterator>();
            manager.run_passes(function);
            bool ti_found = ngraph::helpers::is_tensor_iterator_exist(function);
            EXPECT_EQ(ti_found, true);
        } else {
            bool ti_found = ngraph::helpers::is_tensor_iterator_exist(function);
            EXPECT_EQ(ti_found, false);
        }
    }

    void GenerateInputs() {
        for (const auto &input : executableNetwork.GetInputsInfo()) {
            const auto &info = input.second;
            auto blob = GenerateInput(*info);
            if (input.first == "seq_lengths") {
                blob = FuncTestUtils::createAndFillBlob(info->getTensorDesc(), m_max_seq_len, 0);
            }

            inputs.push_back(blob);
        }
    }

private:
    ngraph::helpers::SequenceTestsMode m_mode;
    int64_t m_max_seq_len = 0;
};

TEST_P(LSTMSequenceCPUTest, CompareWithRefs) {
    SKIP_IF_CURRENT_TEST_IS_DISABLED()

    Run();
    CheckPluginRelatedResults(executableNetwork, "RNNSeq");
}

namespace {
/* CPU PARAMS */
std::vector<std::map<std::string, std::string>> additionalConfig
    = {{{PluginConfigParams::KEY_ENFORCE_BF16, PluginConfigParams::NO}},
       {{PluginConfigParams::KEY_ENFORCE_BF16, PluginConfigParams::YES}}};

CPUSpecificParams cpuParams{{ntc, nc, nc}, {ntc, nc, nc}, {"ref_any"}, "ref_any"};
CPUSpecificParams cpuParamsBatchSizeOne{{tnc, nc, nc}, {tnc, nc, nc}, {"ref_any"}, "ref_any"};

std::vector<ngraph::helpers::SequenceTestsMode> mode{ngraph::helpers::SequenceTestsMode::PURE_SEQ};
std::vector<size_t> seq_lengths_zero_clip{2};
std::vector<size_t> batch_size_one{1};
std::vector<size_t> batch{10};
std::vector<size_t> hidden_size{1, 10};
std::vector<size_t> input_size{10};
// oneDNN supports only sigmoid-tanh-tanh
std::vector<std::vector<std::string>> activations = {{"sigmoid", "tanh", "tanh"}};
// oneDNN supports only zero clip
std::vector<float> clip{0.f};
std::vector<ngraph::op::RecurrentSequenceDirection> direction = {ngraph::op::RecurrentSequenceDirection::FORWARD};
std::vector<InferenceEngine::Precision> netPrecisions = {InferenceEngine::Precision::FP32};

INSTANTIATE_TEST_CASE_P(smoke_LSTMSequenceCPU,
                        LSTMSequenceCPUTest,
                        ::testing::Combine(::testing::Combine(::testing::ValuesIn(mode),
                                                              ::testing::ValuesIn(seq_lengths_zero_clip),
                                                              ::testing::ValuesIn(batch),
                                                              ::testing::ValuesIn(hidden_size),
                                                              ::testing::ValuesIn(input_size),
                                                              ::testing::ValuesIn(activations),
                                                              ::testing::ValuesIn(clip),
                                                              ::testing::ValuesIn(direction),
                                                              ::testing::ValuesIn(netPrecisions),
                                                              ::testing::Values(CommonTestUtils::DEVICE_CPU)),
                                           ::testing::Values(cpuParams),
                                           ::testing::ValuesIn(additionalConfig)),
                        LSTMSequenceCPUTest::getTestCaseName);

INSTANTIATE_TEST_CASE_P(smoke_LSTMSequenceCPUbatchSizeOne,
                        LSTMSequenceCPUTest,
                        ::testing::Combine(::testing::Combine(::testing::ValuesIn(mode),
                                                              ::testing::ValuesIn(seq_lengths_zero_clip),
                                                              ::testing::ValuesIn(batch_size_one),
                                                              ::testing::ValuesIn(hidden_size),
                                                              ::testing::ValuesIn(input_size),
                                                              ::testing::ValuesIn(activations),
                                                              ::testing::ValuesIn(clip),
                                                              ::testing::ValuesIn(direction),
                                                              ::testing::ValuesIn(netPrecisions),
                                                              ::testing::Values(CommonTestUtils::DEVICE_CPU)),
                                           ::testing::Values(cpuParamsBatchSizeOne),
                                           ::testing::ValuesIn(additionalConfig)),
                        LSTMSequenceCPUTest::getTestCaseName);
} // namespace
} // namespace CPULayerTestsDefinitions

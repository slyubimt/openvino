// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <vector>
#include <utility>
#include <string>
#include <type_traits>

#include <legacy/layer_transform.hpp>
#include "gna_graph_tools.hpp"
#include <legacy/details/ie_cnn_network_tools.h>
#include "layer_quantizer.hpp"
#include "scale_factor_calc.hpp"
#include "weights_converter.hpp"

namespace GNAPluginNS {

/**
 * Quantize entire cnn - network
 * @tparam T - type trait for weights and biases
 */
template<class T>
class ModelQuantizer {
 public:
    InferenceEngine::CNNNetwork quantize(const InferenceEngine::CNNNetwork &model, float scaleFactor) const {
        return quantize(model, [](const InferenceEngine::CNNNetwork &, bool runBeforeCopy, bool lowPrecision){}, std::vector<float>({scaleFactor}));
    }

    template <class PreQuantisationCb>
    InferenceEngine::CNNNetwork quantize(const InferenceEngine::CNNNetwork &model, const PreQuantisationCb &cb, float scaleFactor) const {
        return quantize(model, cb, std::vector<float>({scaleFactor}));
    }

    InferenceEngine::CNNNetwork quantize(const InferenceEngine::CNNNetwork &model, std::vector<float> scaleFactor) const {
        return quantize(model, [](InferenceEngine::CNNNetwork &, bool runBeforeCopy, bool lowPrecision){}, scaleFactor);
    }

    template <class PreQuantisationCb>
    InferenceEngine::CNNNetwork quantize(const InferenceEngine::CNNNetwork &model, const PreQuantisationCb &cb, std::vector<float> scaleFactor) const {
        auto visitor = [&](InferenceEngine::CNNLayerPtr lp) {
            auto newLayer = InferenceEngine::injectData<QuantizedLayerParams>(lp);
            transformLayer(newLayer, WeightsConverter());
            return newLayer;
        };
        bool lowPrecision = (T::mandatory().getInputPrecision().size() == sizeof(uint8_t));
        InferenceEngine::CNNNetwork copiedNet = InferenceEngine::CNNNetCopy(model);
        cb(copiedNet, true, lowPrecision);

        copiedNet = InferenceEngine::CNNNetCopy(copiedNet, visitor);

        // allow client code to access copied topology, to avoid copies if user would like to chain quantisation with
        // another preprocessing
        cb(copiedNet, false, lowPrecision);

        if (scaleFactor.empty()) {
            THROW_GNA_EXCEPTION << "Scale factor is empty";
        }

        LayersQuantizer<T> lc(*scaleFactor.begin());
        auto sortedNewNet = InferenceEngine::details::CNNNetSortTopologically(copiedNet);
        gnalog() << "Sorted layers: " << std::endl;
        for (auto &&layer : sortedNewNet) {
            auto quantData = InferenceEngine::getInjectedData<QuantizedLayerParams>(layer);
            quantData->lowPrecision = lowPrecision;
            gnalog() << layer->name << std::endl;
        }
        /// filling scale factors for input layers, memory layers will have scaleFactor of 1.0 by default
        InferenceEngine::InputsDataMap dm = copiedNet.getInputsInfo();
        int scaleIndex = 0;
        for (auto &&inputData : dm) {
            auto inputLayer = getCreatorLayer(inputData.second->getInputData()).lock();
            auto quantData = InferenceEngine::getInjectedData<QuantizedLayerParams>(inputLayer);
            if (scaleFactor.size() <= scaleIndex) {
                THROW_GNA_EXCEPTION << "Scale factors are not set for some of the inputs";
            }
            IE_ASSERT(quantData != nullptr);
            quantData->_src_quant.SetScale(scaleFactor[scaleIndex]);
            scaleIndex++;
        }

        bool isFakeQuantize = std::is_same<T, FakeQuantI8>() || std::is_same<T, FakeQuantI16>();
        propagateScaleFactor(sortedNewNet, T::mandatory().getWeightsPrecision().size(), T::optional().getWeightsPrecision().size(),
                             T::mandatory().getInputPrecision().size(), isFakeQuantize);

        // sorted order gives possibility for propagate quantisation along depended layers
        for (auto &&layer : sortedNewNet) {
            transformLayer(layer, lc);
        }

        return copiedNet;
    }

 private :
    void propagateScaleFactor(std::vector<InferenceEngine::CNNLayerPtr> & net, int mandWeightsBytesSize,
                              int optWeightsBytesSize, int inputsBytesSize, bool fakeQuantize) const {
        ScaleFactorCalculator sf(net, mandWeightsBytesSize, optWeightsBytesSize, inputsBytesSize, fakeQuantize);

        while (!sf.allLayersProcessed()) {
            for (auto &&layer : sf.getStartLayers()) {
                transformLayer(layer, sf);
                // transforming until we reached cases where output scale updated due to situation in downstream layer
                if (sf.needToRestart()) {
                    break;
                }
            }
        }
    }
};
}  // namespace GNAPluginNS

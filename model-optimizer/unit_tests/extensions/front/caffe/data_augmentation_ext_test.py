# Copyright (C) 2018-2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

import unittest
from unittest.mock import patch

import numpy as np

from extensions.front.caffe.data_augmentation_ext import DataAugmentationFrontExtractor
from extensions.ops.data_augmentation import DataAugmentationOp
from mo.ops.op import Op
from unit_tests.utils.extractors import FakeMultiParam
from unit_tests.utils.graph import FakeNode


class FakeDAProtoLayer:
    def __init__(self, val):
        self.augmentation_param = val


class TestDA(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        Op.registered_ops['DataAugmentation'] = DataAugmentationOp

    def test_da_no_pb_no_ml(self):
        self.assertRaises(AttributeError, DataAugmentationFrontExtractor.extract, None)

    @patch('extensions.front.caffe.data_augmentation_ext.merge_attrs')
    def test_da_ext_ideal_numbers(self, merge_attrs_mock):
        params = {
            'crop_width': 0,
            'crop_height': 0,
            'write_augmented': "",
            'max_multiplier': 255.0,
            'augment_during_test': True,
            'recompute_mean': 0,
            'write_mean': "",
            'mean_per_pixel': False,
            'mean': 0,
            'mode': "add",
            'bottomwidth': 0,
            'bottomheight': 0,
            'num': 0,
            'chromatic_eigvec': [0.0]

        }
        merge_attrs_mock.return_value = {
            **params,
            'test': 54,
            'test2': 'test3'
        }
        fake_pl = FakeDAProtoLayer(FakeMultiParam(params))
        fake_node = FakeNode(fake_pl, None)

        DataAugmentationFrontExtractor.extract(fake_node)
        exp_res = {
            'type': 'DataAugmentation',
            'op': 'DataAugmentation',
            'crop_width': 0,
            'crop_height': 0,
            'write_augmented': "",
            'max_multiplier': 255.0,
            'augment_during_test': 1,
            'recompute_mean': 0,
            'write_mean': "",
            'mean_per_pixel': 0,
            'mean': 0,
            'mode': "add",
            'bottomwidth': 0,
            'bottomheight': 0,
            'num': 0,
            'chromatic_eigvec': [0.0],
            'infer': DataAugmentationOp.data_augmentation_infer
        }

        for key in exp_res.keys():
            if key in ('chromatic_eigvec',):
                np.testing.assert_equal(exp_res[key], fake_node[key])
            else:
                self.assertEqual(exp_res[key], fake_node[key])

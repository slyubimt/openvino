# Copyright (C) 2018-2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

import unittest
from unittest.mock import patch

from extensions.front.caffe.simplernms_ext import SimplerNMSFrontExtractor
from extensions.ops.simplernms import SimplerNMSOp
from mo.ops.op import Op
from unit_tests.utils.extractors import FakeMultiParam
from unit_tests.utils.graph import FakeNode


class FakeSimplerNMSProtoLayer:
    def __init__(self, val):
        self.simpler_nms_param = val


class TestSimplerNMSExt(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        Op.registered_ops['SimplerNMS'] = SimplerNMSOp

    def test_simplernms_no_pb_no_ml(self):
        self.assertRaises(AttributeError, SimplerNMSFrontExtractor.extract, None)

    @patch('extensions.front.caffe.simplernms_ext.merge_attrs')
    def test_simplernms_ext_ideal_numbers(self, merge_attrs_mock):
        params = {
            'cls_threshold': 0.5,
            'max_num_proposals': 300,
            'iou_threshold': 0.7,
            'min_bbox_size': 16,
            'feat_stride': 16,
            'pre_nms_topn': 6000,
            'post_nms_topn': 150,
            'scale': [1, 2, 3]
        }
        merge_attrs_mock.return_value = {
            **params
        }

        fake_pl = FakeSimplerNMSProtoLayer(FakeMultiParam(params))
        fake_node = FakeNode(fake_pl, None)

        SimplerNMSFrontExtractor.extract(fake_node)

        exp_res = {
            'cls_threshold': 0.5,
            'max_num_proposals': 300,
            'iou_threshold': 0.7,
            'min_bbox_size': 16,
            'feat_stride': 16,
            'pre_nms_topn': 6000,
            'post_nms_topn': 150,
            'scale': [1, 2, 3],
            'infer': SimplerNMSOp.simplernms_infer
        }

        for key in exp_res.keys():
            self.assertEqual(fake_node[key], exp_res[key])

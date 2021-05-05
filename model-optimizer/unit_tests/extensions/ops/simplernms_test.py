# Copyright (C) 2018-2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

import unittest

import numpy as np

from extensions.ops.simplernms import SimplerNMSOp
from mo.graph.graph import Node
from unit_tests.utils.graph import build_graph

nodes_attributes = {'SimplerNMS_1': {'type': 'SimplerNMS', 'kind': 'op'},
                    'node_1': {'type': 'Identity', 'kind': 'op'},
                    'op_output': { 'kind': 'op', 'op': 'Result'}
                    }


class TestSimplerNMSInfer(unittest.TestCase):
    def test_simplernms_infer_ideal(self):
        graph = build_graph(nodes_attributes,
                            [('SimplerNMS_1', 'node_1'),
                             ('node_1', 'op_output')
                             ],
                            {'node_1': {'shape': None},
                             'SimplerNMS_1': {'feat_stride': 16, 'post_nms_topn': 150, 'scale': [1, 2, 3]}
                             })

        simplernms_node = Node(graph, 'SimplerNMS_1')

        SimplerNMSOp.simplernms_infer(simplernms_node)
        exp_shape = np.array([150, 5])
        res_shape = graph.node['node_1']['shape']
        for i in range(0, len(exp_shape)):
            self.assertEqual(exp_shape[i], res_shape[i])
        self.assertEqual(simplernms_node.scale, ['1', '2', '3'])

    def test_simplernms_infer_no_shape(self):
        graph = build_graph(nodes_attributes,
                            [('SimplerNMS_1', 'node_1'),
                             ('node_1', 'op_output')
                             ],
                            {'node_1': {'shape': None},
                             'SimplerNMS_1': {'feat_stride': 12, 'post_nms_topn': 150, 'scale': [1, 2, 3]}
                             })

        simplernms_node = Node(graph, 'SimplerNMS_1')

        SimplerNMSOp.simplernms_infer(simplernms_node)
        self.assertIsNone(graph.node['node_1']['shape'])

# Copyright (C) 2018-2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

import unittest

import numpy as np

from extensions.ops.sparse_reshape import SparseReshape
from mo.front.common.partial_infer.utils import int64_array
from mo.graph.graph import Node
from unit_tests.utils.graph import build_graph

nodes_attributes = {'input_indices': {'shape': None, 'value': None, 'kind': 'data'},
                    'input_shape': {'shape': None, 'value': None, 'kind': 'data'},
                    'new_shape': {'shape': None, 'value': None, 'kind': 'data'},
                    'sparse_reshape_node': {'op': 'SparseReshape', 'kind': 'op'},
                    'output_indices': {'shape': None, 'value': None, 'kind': 'data'},
                    'output_shape': {'shape': None, 'value': None, 'kind': 'data'}}

# graph 1
edges1 = [('input_indices', 'sparse_reshape_node', {'in': 0}),
          ('input_shape', 'sparse_reshape_node', {'in': 1}),
          ('new_shape', 'sparse_reshape_node', {'in': 2}),
          ('sparse_reshape_node', 'output_indices', {'out': 0}),
          ('sparse_reshape_node', 'output_shape', {'out': 1})]

inputs1 = {'input_indices': {'shape': int64_array([5, 2]), 'value': None},
           'input_shape': {'shape': int64_array([2]), 'value': int64_array([4, 5])},
           'new_shape': {'shape': int64_array([3]), 'value': int64_array([5, -1, 2])}}


class TestSparseReshape(unittest.TestCase):
    def test_partial_infer1(self):
        graph = build_graph(nodes_attributes, edges1, inputs1)
        sparse_reshape_node = Node(graph, 'sparse_reshape_node')
        SparseReshape.infer(sparse_reshape_node)

        # prepare reference results
        ref_output_indices_shape = np.array([5, 3], dtype=np.int32)
        ref_output_shape_value = np.array([5, 2, 2], dtype=np.int32)

        # get the result
        res_output_indices_shape = graph.node['output_indices']['shape']
        res_output_shape_value = graph.node['output_shape']['value']

        self.assertTrue(np.array_equal(ref_output_indices_shape, res_output_indices_shape),
                        'shapes do not match expected: {} and given: {}'.format(ref_output_indices_shape, res_output_indices_shape))
        self.assertTrue(np.array_equal(ref_output_shape_value, res_output_shape_value),
                        'values do not match expected: {} and given: {}'.format(ref_output_shape_value, res_output_shape_value))


# Copyright (C) 2018-2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

import unittest

import numpy as np

from extensions.front.tf.fifo_replacer import FIFOQueue
from unit_tests.utils.graph import build_graph_with_edge_attrs


class TestFIFOQueueReplacement(unittest.TestCase):
    def test_fifo_with_label_batch(self):
        nodes = {
            'placeholder': {'op': 'Parameter', 'data_type': np.int32, 'kind': 'op', 'shape': np.array(1)},
            'batch_join/fifo_queue': {'op': 'FIFOQueueV2', 'name': 'batch_join/fifo_queue',
                                      'shapes': np.array([[1, 2, 3]]), 'types': np.array([np.float32]), 'kind': 'op'},
            'batch_join': {'op': 'QueueDequeueUpToV2', 'kind': 'op'},
            'image_batch': {'op': 'Identity', 'data_type': np.float32, 'kind': 'op'},
            'label_batch': {'op': 'Identity', 'kind': 'op'},
            'label_batch_op_output': {'op': 'Result', 'kind': 'op'},
        }
        edges = [
            ('placeholder', 'batch_join', {'out': 0, 'in': 0}),
            ('batch_join/fifo_queue', 'batch_join', {'out': 0, 'in': 1}),
            ('batch_join', 'image_batch', {'out': 0, 'in': 0}),
            ('batch_join', 'label_batch', {'out': 1, 'in': 0}),
            ('label_batch', 'label_batch_op_output', {'out': 0, 'in': 0})
        ]
        graph = build_graph_with_edge_attrs(nodes, edges)
        tested_class = FIFOQueue()
        tested_class.find_and_replace_pattern(graph=graph)
        after_pattern = graph.nodes()
        self.assertEqual(2, len(after_pattern))
        try:
            new_ph_dict = graph.node[[u for u, v in graph.in_edges('image_batch')][0]]
        except Exception as e:
            self.fail("Can't get new placeholder. Broken edge. Additional information: {}".format(e))
        self.assertEqual(new_ph_dict['name'], 'batch_join/fifo_queue')
        self.assertTrue(np.array_equal(new_ph_dict['shape'], [1, 2, 3]))

    def test_fifo_with_out_label_batch(self):
        nodes_no_label = {
            'placeholder': {'op': 'Parameter', 'data_type': np.int32, 'kind': 'op', 'shape': np.array(0)},
            'batch_join/fifo_queue': {'op': 'FIFOQueueV2', 'name': 'batch_join/fifo_queue',
                                      'shapes': np.array([[1, 2, 3]]), 'types': np.array([np.float32]), 'kind': 'op'},
            'batch_join': {'op': 'QueueDequeueUpToV2', 'kind': 'op'},
            'image_batch': {'op': 'Identity', 'data_type': np.float32, 'kind': 'op'},
        }
        edges_no_label = [
            ('placeholder', 'batch_join', {'out': 0, 'in': 0}),
            ('batch_join/fifo_queue', 'batch_join', {'out': 0, 'in': 1}),
            ('batch_join', 'image_batch', {'out': 0, 'in': 0})
        ]

        graph = build_graph_with_edge_attrs(nodes_no_label, edges_no_label)
        tested_class = FIFOQueue()
        tested_class.find_and_replace_pattern(graph=graph)
        after_pattern = graph.nodes()
        self.assertEqual(2, len(after_pattern))
        try:
            new_ph_dict = graph.node[[u for u, v in graph.in_edges('image_batch')][0]]
        except Exception as e:
            self.fail("Can't get new placeholder. Broken edge. Additional information: {}".format(e))
        self.assertEqual(new_ph_dict['name'], 'batch_join/fifo_queue')
        self.assertTrue(np.array_equal(new_ph_dict['shape'], np.array([1, 2, 3])))

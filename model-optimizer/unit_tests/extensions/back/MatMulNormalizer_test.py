# Copyright (C) 2018-2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

import unittest
from argparse import Namespace

from generator import generate, generator

from extensions.back.MatMulNormalizer import SmartReshape_HC_Reshape_MatMul
from extensions.ops.MatMul import MatMul
from mo.front.common.partial_infer.utils import int64_array
from mo.ops.reshape import Reshape
from mo.utils.ir_engine.compare_graphs import compare_graphs
from unit_tests.utils.graph import build_graph, regular_op_with_shaped_data, valued_const_with_data, \
    result, connect
from unit_tests.utils.graph import regular_op_with_empty_data as op_with_empty_data


@generator
class SmartReshape_HC_Reshape_MatMulTest(unittest.TestCase):
    @generate(
        *[
            ([1, 20, 30], [30, 40], [20, -1], False, False, [-1, 30]),
            ([1, 20, 30], [40, 30], [20, -1], False, True, [-1, 30]),
            ([1, 30, 20], [30, 40], [-1, 20], True, False, [30, -1]),
            ([1, 30, 20], [40, 30], [-1, 20], True, True, [30, -1]),
        ]
    )
    def test_reshape_on_the_A_input(self,
                                    in1_shape, in2_shape, reshape_pattern, transpose_a, transpose_b, updated_pattern):
        nodes = {
            **regular_op_with_shaped_data('in_1', in1_shape, dict(type='Parameter', op='Parameter')),
            **regular_op_with_shaped_data('in_2', in2_shape, dict(type='Parameter', op='Parameter')),
            **valued_const_with_data('dim', int64_array(reshape_pattern)),
            **op_with_empty_data('reshape',
                                 dict(type='Reshape', op='Reshape', infer=Reshape.infer, need_shape_inference=True)),
            **op_with_empty_data('matmul',
                                 dict(type='MatMul', op='MatMul', infer=MatMul.infer, need_shape_inference=True,
                                      transpose_a=transpose_a, transpose_b=transpose_b, dim_attrs={})),
            **result(),
        }
        edges = [
            *connect('in_1:0', '0:reshape'),
            *connect('dim:0', '1:reshape'),
            *connect('reshape:0', '0:matmul'),
            *connect('in_2:0', '1:matmul'),
            *connect('matmul:0', 'output'),
        ]
        graph = build_graph(nodes_attrs=nodes, edges=edges, cli=Namespace(static_shape=True))
        graph.clean_up()
        SmartReshape_HC_Reshape_MatMul().find_and_replace_pattern(graph)
        graph.clean_up()

        graph_ref = build_graph(nodes_attrs=nodes, edges=edges, update_attributes={
            'dim': {'value': int64_array(updated_pattern)}, 'dim_d': {'value': int64_array(updated_pattern)}})
        graph_ref.clean_up()

        (flag, resp) = compare_graphs(graph, graph_ref, 'output', check_op_attrs=True)
        self.assertTrue(flag, resp)

    @generate(*[
        ([20, 30], [1, 30, 40], [-1, 40], False, False, [30, -1]),
        ([20, 30], [1, 40, 30], [40, -1], False, True, [-1, 30]),
        ([30, 20], [1, 30, 40], [-1, 40], True, False, [30, -1]),
        ([30, 20], [1, 40, 30], [40, -1], True, True, [-1, 30]),
    ])
    def test_reshape_on_the_B_input(self,
                                    in1_shape, in2_shape, reshape_pattern, transpose_a, transpose_b, updated_pattern):
        nodes = {
            **regular_op_with_shaped_data('in_1', in1_shape, dict(type='Parameter', op='Parameter')),
            **regular_op_with_shaped_data('in_2', in2_shape, dict(type='Parameter', op='Parameter')),
            **valued_const_with_data('dim', int64_array(reshape_pattern)),
            **op_with_empty_data('reshape',
                                 dict(type='Reshape', op='Reshape', infer=Reshape.infer, need_shape_inference=True)),
            **op_with_empty_data('matmul',
                                 dict(type='MatMul', op='MatMul', infer=MatMul.infer, need_shape_inference=True,
                                      transpose_a=transpose_a, transpose_b=transpose_b, dim_attrs={})),
            **result(),
        }
        edges = [
            *connect('in_1:0', '0:matmul'),
            *connect('in_2:0', '0:reshape'),
            *connect('dim:0', '1:reshape'),
            *connect('reshape:0', '1:matmul'),
            *connect('matmul:0', 'output'),
        ]
        graph = build_graph(nodes_attrs=nodes, edges=edges, cli=Namespace(static_shape=True))
        graph.clean_up()
        SmartReshape_HC_Reshape_MatMul().find_and_replace_pattern(graph)
        graph.clean_up()

        graph_ref = build_graph(nodes_attrs=nodes, edges=edges, update_attributes={
            'dim': {'value': int64_array(updated_pattern)}, 'dim_d': {'value': int64_array(updated_pattern)}})
        graph_ref.clean_up()

        (flag, resp) = compare_graphs(graph, graph_ref, 'output', check_op_attrs=True)
        self.assertTrue(flag, resp)

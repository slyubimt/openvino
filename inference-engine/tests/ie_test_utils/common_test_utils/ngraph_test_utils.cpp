// Copyright (C) 2020-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "ngraph_test_utils.hpp"

#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <sstream>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <vector>

#include <ngraph/function.hpp>
#include <ngraph/op/util/op_types.hpp>
#include <ngraph/op/util/sub_graph_base.hpp>
#include <ngraph/opsets/opset1.hpp>
#include <ngraph/opsets/opset6.hpp>
#include <ngraph/pass/visualize_tree.hpp>

namespace {
bool isTypeRelaxed(const std::string& type) {
    return type.find_first_of("TypeRelaxed") == 0;
}

bool compareTypeInfo(const ngraph::DiscreteTypeInfo& info1, const ngraph::DiscreteTypeInfo& info2) {
    if (!isTypeRelaxed(info1.name) && !isTypeRelaxed(info2.name) &&
        (info1.version != info2.version)) {
        return false;
    }

    const std::string info1Name =
        isTypeRelaxed(info1.name) && (info1.parent != nullptr) ? info1.parent->name : info1.name;
    const std::string info2Name =
        isTypeRelaxed(info2.name) && (info2.parent != nullptr) ? info2.parent->name : info2.name;
    return info1Name == info2Name;
}

template <typename Node>
bool compare_rt_keys(const Node& node1, const Node& node2) {
    const auto& first_node_rt_info = node1->get_rt_info();
    const auto& second_node_rt_info = node2->get_rt_info();

    if (first_node_rt_info.empty() && second_node_rt_info.empty()) {
        return true;
    }

    if (first_node_rt_info.size() != second_node_rt_info.size()) {
        return false;
    }

    auto first_node_rt_info_it = first_node_rt_info.begin();
    auto second_node_rt_info_it = second_node_rt_info.begin();
    while (first_node_rt_info_it != first_node_rt_info.end()) {
        if (first_node_rt_info_it->first != second_node_rt_info_it->first) {
            return false;
        }
        ++first_node_rt_info_it;
        ++second_node_rt_info_it;
    }

    return true;
}

bool less_by_name(
    const std::shared_ptr<ngraph::op::v0::Result>& l,
    const std::shared_ptr<ngraph::op::v0::Result>& r) {
    return l->get_friendly_name() < r->get_friendly_name();
}

template <typename T>
std::string to_str(const T& v) {
    return std::to_string(v);
}

std::string typeInfoToStr(const ngraph::Node::type_info_t& typeInfo) {
    return std::string(typeInfo.name) + "/" + to_str(typeInfo.version);
}

template <typename Node>
std::string name(const Node& n) {
    return n->get_friendly_name();
}

std::string tensor_names(const ngraph::descriptor::Tensor& t) {
    std::string n;
    const char* glue = "";
    for (const auto& name : t.get_names()) {
        n.append(glue).append(name);
        glue = ", ";
    }
    return "\"" + n + "\"";
}

class Comparator {
public:
    using CmpValues = FunctionsComparator::CmpValues;
    using Result = FunctionsComparator::Result;
    using ComparedNodes = std::pair<ngraph::Node*, ngraph::Node*>;

    explicit Comparator(CmpValues f) : m_comparition_flags(f) {}

    Result compare(
        const std::shared_ptr<ngraph::Function>& f1, const std::shared_ptr<ngraph::Function>& f2);

    Result compare(ngraph::Node* node1, ngraph::Node* node2) {
        std::stringstream errors;
        const auto result = compare(node1, node2, errors);
        if (!result.valid) {
            return result;
        }
        const auto msg = errors.str();
        return msg.empty() ? Result::ok() : Result::error(msg);
    }

    Comparator recreate() const {
        return Comparator(m_comparition_flags);
    }

private:
    bool should_compare(CmpValues f) const noexcept {
        return m_comparition_flags & f;
    }

    ///
    /// \param err_log - will be fill by minor errors if happen
    /// \return only fatality error if some minor one appears it will be add to err_log
    ///
    Result compare(ngraph::Node* node1, ngraph::Node* node2, std::ostream& err_log);

    void add_nodes_inputs_to_queue(ngraph::Node* node1, ngraph::Node* node2);

    //-- DATA --
    CmpValues m_comparition_flags;

    std::queue<ComparedNodes> q;
    std::unordered_set<ngraph::Node*> used;
};

namespace attr_comparison {

using AttrName = std::string;

class Result {
public:
    explicit Result(std::string m = {}) : m_message(std::move(m)) {}

    const std::string& message() const {
        return m_message;
    }

    bool has_error() const {
        return !m_message.empty();
    }

    Result& operator+=(const std::string& msg) {
        m_message.append(m_break_line_no, '\n').append(msg);
        m_break_line_no = 1;
        return *this;
    }

private:
    std::string m_message;
    int m_break_line_no{0};
};

using SubGraphOpInputDescription =
    std::vector<std::shared_ptr<ngraph::op::util::SubGraphOp::InputDescription>>;

using SubGraphOpOutputDescription =
    std::vector<std::shared_ptr<ngraph::op::util::SubGraphOp::OutputDescription>>;

using SpecialBodyPorts = ngraph::opset6::Loop::SpecialBodyPorts;

namespace storage {

class MemoryChunk {
public:
    using Data = std::vector<unsigned char>;
    MemoryChunk(Data data) : m_data{std::move(data)} {}

    Data::const_pointer data() const {
        return m_data.data();
    }

    size_t size() const {
        return m_data.size();
    }

private:
    Data m_data;
};

template <typename AttrValue>
class AttributeStorage {
public:
    bool insert_value(AttrName name, AttrValue value) {
        return m_attributes.insert({std::move(name), std::move(value)}).second;
    }

    const AttrValue* get_value(const AttrName& name) const {
        const auto found = m_attributes.find(name);
        if (found != end(m_attributes)) {
            return std::addressof(found->second);
        }
        return {};
    }

    std::size_t get_attributes_number() const {
        return m_attributes.size();
    }

private:
    std::map<AttrName, AttrValue> m_attributes;
};

class Storage : private AttributeStorage<MemoryChunk>,
                private AttributeStorage<bool>,
                private AttributeStorage<std::string>,
                private AttributeStorage<int8_t>,
                private AttributeStorage<int16_t>,
                private AttributeStorage<int32_t>,
                private AttributeStorage<int64_t>,
                private AttributeStorage<uint8_t>,
                private AttributeStorage<uint16_t>,
                private AttributeStorage<uint32_t>,
                private AttributeStorage<uint64_t>,
                private AttributeStorage<float>,
                private AttributeStorage<double>,
                private AttributeStorage<std::vector<int8_t>>,
                private AttributeStorage<std::vector<int16_t>>,
                private AttributeStorage<std::vector<int32_t>>,
                private AttributeStorage<std::vector<int64_t>>,
                private AttributeStorage<std::vector<uint8_t>>,
                private AttributeStorage<std::vector<uint16_t>>,
                private AttributeStorage<std::vector<uint32_t>>,
                private AttributeStorage<std::vector<uint64_t>>,
                private AttributeStorage<std::vector<float>>,
                private AttributeStorage<std::vector<double>>,
                private AttributeStorage<std::vector<std::string>>,
                private AttributeStorage<SubGraphOpInputDescription>,
                private AttributeStorage<SubGraphOpOutputDescription>,
                private AttributeStorage<SpecialBodyPorts> {
public:
    template <typename AttrValue>
    const AttributeStorage<AttrValue>& storage() const {
        return *static_cast<const AttributeStorage<AttrValue>*>(this);
    }
    template <typename AttrValue>
    AttributeStorage<AttrValue>& storage() {
        return *static_cast<AttributeStorage<AttrValue>*>(this);
    }

    size_t stored_attributes_number() const {
        return storage<MemoryChunk>().get_attributes_number() +
               storage<bool>().get_attributes_number() +
               storage<std::string>().get_attributes_number() +
               storage<int8_t>().get_attributes_number() +
               storage<int16_t>().get_attributes_number() +
               storage<int32_t>().get_attributes_number() +
               storage<int64_t>().get_attributes_number() +
               storage<uint8_t>().get_attributes_number() +
               storage<uint16_t>().get_attributes_number() +
               storage<uint32_t>().get_attributes_number() +
               storage<uint64_t>().get_attributes_number() +
               storage<float>().get_attributes_number() +
               storage<double>().get_attributes_number() +
               storage<std::vector<int8_t>>().get_attributes_number() +
               storage<std::vector<int16_t>>().get_attributes_number() +
               storage<std::vector<int32_t>>().get_attributes_number() +
               storage<std::vector<int64_t>>().get_attributes_number() +
               storage<std::vector<uint8_t>>().get_attributes_number() +
               storage<std::vector<uint16_t>>().get_attributes_number() +
               storage<std::vector<uint32_t>>().get_attributes_number() +
               storage<std::vector<uint64_t>>().get_attributes_number() +
               storage<std::vector<float>>().get_attributes_number() +
               storage<std::vector<double>>().get_attributes_number() +
               storage<std::vector<std::string>>().get_attributes_number() +
               storage<SubGraphOpInputDescription>().get_attributes_number() +
               storage<SubGraphOpOutputDescription>().get_attributes_number() +
               storage<SpecialBodyPorts>().get_attributes_number();
    }
};

}  // namespace storage

class ReadAndStoreAttributes : public ngraph::AttributeVisitor, protected storage::Storage {
public:
    void on_adapter(const std::string& name, ngraph::ValueAccessor<void>& adapter) override {
        if (auto inputs =
                ngraph::as_type<ngraph::AttributeAdapter<SubGraphOpInputDescription>>(&adapter)) {
            insert(name, inputs->get());
        } else if (
            auto outputs =
                ngraph::as_type<ngraph::AttributeAdapter<SubGraphOpOutputDescription>>(&adapter)) {
            insert(name, outputs->get());
        } else if (
            auto ports = ngraph::as_type<ngraph::AttributeAdapter<SpecialBodyPorts>>(&adapter)) {
            insert(name, ports->get());
        } else if (
            auto a = ngraph::as_type<
                ngraph::AttributeAdapter<std::shared_ptr<ngraph::runtime::AlignedBuffer>>>(
                &adapter)) {
            const auto beg = static_cast<unsigned char*>(a->get()->get_ptr());
            const auto end = beg + a->get()->size();
            insert(name, storage::MemoryChunk{storage::MemoryChunk::Data(beg, end)});
        } else {
            m_read_result += "store   attr [ ERR ]: " + name +
                             " [drop `void` comparison which is '" + adapter.get_type_info().name +
                             "']";
        }
    }

#define ON_ADAPTER(TYPE)                                                                      \
    void on_adapter(const std::string& name, ngraph::ValueAccessor<TYPE>& adapter) override { \
        insert(name, adapter.get());                                                          \
    }

    ON_ADAPTER(bool)
    ON_ADAPTER(std::string)
    ON_ADAPTER(int8_t)
    ON_ADAPTER(int16_t)
    ON_ADAPTER(int32_t)
    ON_ADAPTER(int64_t)
    ON_ADAPTER(uint8_t)
    ON_ADAPTER(uint16_t)
    ON_ADAPTER(uint32_t)
    ON_ADAPTER(uint64_t)
    ON_ADAPTER(float)
    ON_ADAPTER(double)
    ON_ADAPTER(std::vector<int8_t>)
    ON_ADAPTER(std::vector<int16_t>)
    ON_ADAPTER(std::vector<int32_t>)
    ON_ADAPTER(std::vector<int64_t>)
    ON_ADAPTER(std::vector<uint8_t>)
    ON_ADAPTER(std::vector<uint16_t>)
    ON_ADAPTER(std::vector<uint32_t>)
    ON_ADAPTER(std::vector<uint64_t>)
    ON_ADAPTER(std::vector<float>)
    ON_ADAPTER(std::vector<double>)
    ON_ADAPTER(std::vector<std::string>)

#undef ON_ADAPTER

    void on_adapter(
        const std::string&, ngraph::ValueAccessor<std::shared_ptr<ngraph::Function>>&) override {
        // handled by `compare_functions` drop it here
    }

    template <typename AttrValue>
    const AttrValue* get(const AttrName& name) const {
        return storage<AttrValue>().get_value(name);
    }

    template <typename AttrValue>
    bool insert(AttrName name, AttrValue value) {
        return storage<AttrValue>().insert_value(std::move(name), std::move(value));
    }

    size_t attributes_number() const {
        return stored_attributes_number();
    }

    const Result read_result() const {
        return m_read_result;
    }

private:
    Result m_read_result;
};

namespace equal {

template <typename Value>
struct Equal {
    static bool equal_value(const Value& lhs, const Value& rhs) {
        return lhs == rhs;
    }
};

template <>
struct Equal<ngraph::bfloat16> {
    static bool equal_value(ngraph::bfloat16 lhs, ngraph::bfloat16 rhs) {
        if (lhs.to_bits() == rhs.to_bits()) {
            return true;
        }
        return std::abs(lhs - rhs) < 1e-3;
    }
};

template <>
struct Equal<ngraph::float16> {
    static bool equal_value(ngraph::float16 lhs, ngraph::float16 rhs) {
        if (lhs.to_bits() == rhs.to_bits()) {
            return true;
        }
        return std::abs(lhs - rhs) < 1e-3;
    }
};

template <>
struct Equal<float> {
    static bool equal_value(float lhs, float rhs) {
      if (std::isfinite(lhs) && std::isfinite(rhs)) {
        return std::abs(lhs - rhs) < 1e-4;
      }
      return (std::isinf(lhs) && std::isinf(rhs)) ||
             (std::isnan(lhs) && std::isnan(rhs));
    }
};

template <>
struct Equal<double> {
    static bool equal_value(double lhs, double rhs) {
      if (std::isfinite(lhs) && std::isfinite(rhs)) {
        return std::abs(lhs - rhs) < 1e-5;
      }
      return (std::isinf(lhs) && std::isinf(rhs)) ||
             (std::isnan(lhs) && std::isnan(rhs));
    }
};

template <typename T>
struct Equal<std::vector<T>> {
    static bool equal_value(const std::vector<T>& lhs, const std::vector<T>& rhs) {
        return lhs.size() == rhs.size() &&
               std::equal(begin(lhs), end(lhs), begin(rhs), Equal<T>::equal_value);
    }
};

template <>
struct Equal<SubGraphOpInputDescription::value_type> {
    static bool equal_value(
        SubGraphOpInputDescription::const_reference lhs,
        SubGraphOpInputDescription::const_reference rhs) {
        const auto& lhs_type_info = lhs->get_type_info();
        const auto& rhs_type_info = rhs->get_type_info();
        if (lhs_type_info != rhs_type_info) {
            return false;
        }
        using SubGraphOp = ngraph::op::util::SubGraphOp;
        if (lhs_type_info == SubGraphOp::SliceInputDescription::type_info) {
            const auto& l_input = static_cast<const SubGraphOp::SliceInputDescription&>(*lhs);
            const auto& r_input = static_cast<const SubGraphOp::SliceInputDescription&>(*rhs);
            return l_input.m_start == r_input.m_start && l_input.m_stride == r_input.m_stride &&
                   l_input.m_part_size == r_input.m_part_size && l_input.m_end == r_input.m_end &&
                   l_input.m_axis == r_input.m_axis;
        } else if (lhs_type_info == SubGraphOp::MergedInputDescription::type_info) {
            return true;
        } else if (lhs_type_info == SubGraphOp::InvariantInputDescription::type_info) {
            return true;
        }
        return false;
    }
};

template <>
struct Equal<SubGraphOpInputDescription> {
    static bool equal_value(
        const SubGraphOpInputDescription& lhs, const SubGraphOpInputDescription& rhs) {
        if (lhs.size() != rhs.size()) {
            return false;
        }
        return std::is_permutation(
            begin(lhs), end(lhs), begin(rhs),
            Equal<SubGraphOpInputDescription::value_type>::equal_value);
    }
};

template <>
struct Equal<SubGraphOpOutputDescription::value_type> {
    static bool equal_value(
        SubGraphOpOutputDescription::const_reference lhs,
        SubGraphOpOutputDescription::const_reference rhs) {
        const auto& lhs_type_info = lhs->get_type_info();
        const auto& rhs_type_info = rhs->get_type_info();
        if (lhs_type_info != rhs_type_info) {
            return false;
        }
        using SubGraphOp = ngraph::op::util::SubGraphOp;
        if (lhs_type_info == SubGraphOp::ConcatOutputDescription::type_info) {
            const auto& l_output = static_cast<const SubGraphOp::ConcatOutputDescription&>(*lhs);
            const auto& r_output = static_cast<const SubGraphOp::ConcatOutputDescription&>(*rhs);
            return l_output.m_start == r_output.m_start && l_output.m_stride == r_output.m_stride &&
                   l_output.m_part_size == r_output.m_part_size &&
                   l_output.m_end == r_output.m_end && l_output.m_axis == r_output.m_axis;
        } else if (lhs_type_info == SubGraphOp::BodyOutputDescription::type_info) {
            const auto& l_output = static_cast<const SubGraphOp::BodyOutputDescription&>(*lhs);
            const auto& r_output = static_cast<const SubGraphOp::BodyOutputDescription&>(*rhs);
            return l_output.m_iteration == r_output.m_iteration;
        }
        return false;
    }
};

template <>
struct Equal<SubGraphOpOutputDescription> {
    static bool equal_value(
        const SubGraphOpOutputDescription& lhs, const SubGraphOpOutputDescription& rhs) {
        if (lhs.size() != rhs.size()) {
            return false;
        }
        return std::is_permutation(
            begin(lhs), end(lhs), begin(rhs),
            Equal<SubGraphOpOutputDescription::value_type>::equal_value);
    }
};

template <>
struct Equal<SpecialBodyPorts> {
    static bool equal_value(const SpecialBodyPorts& lhs, const SpecialBodyPorts& rhs) {
        return lhs.current_iteration_input_idx == rhs.current_iteration_input_idx;
    }
};

using Constant = ngraph::opset1::Constant;
template <>
struct Equal<std::shared_ptr<Constant>> {
    static bool equal_value(
        const std::shared_ptr<Constant>& lhs, const std::shared_ptr<Constant>& rhs) {
        const auto lhs_t = lhs->get_element_type();
        const auto rhs_t = rhs->get_element_type();
        if (lhs_t != rhs_t) {
            return false;
        }

        switch (lhs_t) {
        case ngraph::element::Type_t::bf16: {
            auto lhs_v = lhs->cast_vector<ngraph::bfloat16>();
            auto rhs_v = rhs->cast_vector<ngraph::bfloat16>();
            return Equal<std::vector<ngraph::bfloat16>>::equal_value(lhs_v, rhs_v);
            break;
        }
        case ngraph::element::Type_t::f16: {
            const auto& lhs_v = lhs->cast_vector<ngraph::float16>();
            const auto& rhs_v = rhs->cast_vector<ngraph::float16>();
            return Equal<std::vector<ngraph::float16>>::equal_value(lhs_v, rhs_v);
            break;
        }
        case ngraph::element::Type_t::f32: {
            const auto& lhs_v = lhs->cast_vector<float>();
            const auto& rhs_v = rhs->cast_vector<float>();
            return Equal<std::vector<float>>::equal_value(lhs_v, rhs_v);
            break;
        }
        default: {
            const auto& lhs_v = lhs->cast_vector<double>();
            const auto& rhs_v = rhs->cast_vector<double>();
            return Equal<std::vector<double>>::equal_value(lhs_v, rhs_v);
            break;
        }
        }
        return false;
    }
};
}  // namespace equal

namespace str {
template <typename...>
struct Void_t {
    using type = void;
};

template <typename T, typename = void>
struct Get {
    static std::string value(const T&) {
        return std::string("[Ups can't convert this to value: ") + typeid(T).name() + "]";
    }
};

template <typename T>
struct Get<T, typename Void_t<decltype(std::to_string(std::declval<T>()))>::type> {
    static std::string value(const T& v) {
        return "[" + std::to_string(v) + "]";
    }
};

template <>
struct Get<std::string, void> {
    static std::string value(const std::string& v) {
        return "[" + v + "]";
    }
};

template <typename T>
struct Get<
    T,
    typename Void_t<decltype(begin(std::declval<T>())), decltype(end(std::declval<T>()))>::type> {
    template <typename Container>
    static std::string join(const Container& c, const char* glue = ", ") {
        std::stringstream oss;
        const char* s = "";
        for (const auto& v : c) {
            oss << s << v;
            s = glue;
        }
        return oss.str();
    }

    static std::string value(const T& v) {
        return "[" + join(v) + "]";
    }
};

}  // namespace str

class ReadAndCompareAttributes : public ngraph::AttributeVisitor {
public:
    ReadAndCompareAttributes(const ReadAndStoreAttributes& ref)
        : m_attr_ref(ref), m_cmp_result{ref.read_result()} {}

    void on_adapter(const std::string& name, ngraph::ValueAccessor<void>& adapter) override {
        if (should_return()) {
            return;
        }
        m_visited_attributes.insert(name);
        if (auto inputs =
                ngraph::as_type<ngraph::AttributeAdapter<SubGraphOpInputDescription>>(&adapter)) {
            verify(name, inputs->get());
        } else if (
            auto outputs =
                ngraph::as_type<ngraph::AttributeAdapter<SubGraphOpOutputDescription>>(&adapter)) {
            verify(name, outputs->get());
        } else if (
            auto ports = ngraph::as_type<ngraph::AttributeAdapter<SpecialBodyPorts>>(&adapter)) {
            verify(name, ports->get());
        } else if (
            auto a = ngraph::as_type<
                ngraph::AttributeAdapter<std::shared_ptr<ngraph::runtime::AlignedBuffer>>>(
                &adapter)) {
            m_visited_attributes.insert(name);
            const auto ref_value = m_attr_ref.get<storage::MemoryChunk>(name);
            if (!ref_value) {
                m_cmp_result += "missing attribute name: '" + name + "'";
                return;
            }

            if (a->get()->size() != ref_value->size() ||
                std::memcmp(ref_value->data(), a->get()->get_ptr(), ref_value->size()) != 0) {
                m_cmp_result += "mismatch in value: '" + name + "' : look in to the mem buffer";
                return;
            }
        } else {
            m_cmp_result += "compare attr [ ERR ]: " + name +
                            " [drop `void` comparison which is '" + adapter.get_type_info().name +
                            "']";
        }
    }

#define ON_ADAPTER(TYPE)                                                                      \
    void on_adapter(const std::string& name, ngraph::ValueAccessor<TYPE>& adapter) override { \
        verify(name, adapter.get());                                                          \
    }

    ON_ADAPTER(bool)
    ON_ADAPTER(std::string)
    ON_ADAPTER(int8_t)
    ON_ADAPTER(int16_t)
    ON_ADAPTER(int32_t)
    ON_ADAPTER(int64_t)
    ON_ADAPTER(uint8_t)
    ON_ADAPTER(uint16_t)
    ON_ADAPTER(uint32_t)
    ON_ADAPTER(uint64_t)
    ON_ADAPTER(float)
    ON_ADAPTER(double)
    ON_ADAPTER(std::vector<int8_t>)
    ON_ADAPTER(std::vector<int16_t>)
    ON_ADAPTER(std::vector<int32_t>)
    ON_ADAPTER(std::vector<int64_t>)
    ON_ADAPTER(std::vector<uint8_t>)
    ON_ADAPTER(std::vector<uint16_t>)
    ON_ADAPTER(std::vector<uint32_t>)
    ON_ADAPTER(std::vector<uint64_t>)
    ON_ADAPTER(std::vector<float>)
    ON_ADAPTER(std::vector<double>)
    ON_ADAPTER(std::vector<std::string>)

#undef ON_ADAPTER

    void on_adapter(
        const std::string&, ngraph::ValueAccessor<std::shared_ptr<ngraph::Function>>&) override {
        // handled by `compare_functions` drop it here
    }

    bool all_attr_was_compared() const {
        return m_visited_attributes.size() == m_attr_ref.attributes_number();
    }

    size_t compared_attr_number() const {
        return m_visited_attributes.size();
    }

    const Result& cmp_result() const {
        return m_cmp_result;
    }

private:
    bool should_return() const {
        return m_fast_exit && m_cmp_result.has_error();
    }
    template <typename AttrValue>
    void verify(const std::string& name, const AttrValue& attr_value) {
        if (should_return()) {
            return;
        }
        m_visited_attributes.insert(name);
        const auto ref_value = m_attr_ref.get<AttrValue>(name);
        if (!ref_value) {
            m_cmp_result += "missing attribute name: '" + name + "'";
            return;
        }

        if (!equal::Equal<AttrValue>::equal_value(*ref_value, attr_value)) {
            m_cmp_result += "mismatch in value: '" + name +
                            "' : " + str::Get<AttrValue>::value(*ref_value) + " vs " +
                            str::Get<AttrValue>::value(attr_value);
        }
    }

    const ReadAndStoreAttributes& m_attr_ref;
    Result m_cmp_result;
    std::set<AttrName> m_visited_attributes;
    bool m_fast_exit{true};
};

}  // namespace attr_comparison

class CompareNodesAttributes {
public:
    CompareNodesAttributes() : m_compare_attr(m_store_attr) {}

    attr_comparison::ReadAndStoreAttributes& get_ref_reader() {
        return m_store_attr;
    }

    attr_comparison::ReadAndCompareAttributes& get_cmp_reader() {
        return m_compare_attr;
    }

    bool equal() const {
        return m_compare_attr.all_attr_was_compared() && !m_compare_attr.cmp_result().has_error();
    }

    friend std::string to_string(const CompareNodesAttributes& c) {
        const auto& result = c.m_compare_attr.cmp_result();
        if (result.has_error()) {
            return result.message();
        }
        if (!c.m_compare_attr.all_attr_was_compared()) {
            return "not all of attr was compared: " +
                   std::to_string(c.m_compare_attr.compared_attr_number()) + " vs " +
                   std::to_string(c.m_store_attr.attributes_number());
        }
        return "looks good [compared " + std::to_string(c.m_compare_attr.compared_attr_number()) +
               " attributes]";
    }

private:
    attr_comparison::ReadAndStoreAttributes m_store_attr;
    attr_comparison::ReadAndCompareAttributes m_compare_attr;
};

Comparator::Result Comparator::compare(
    const std::shared_ptr<ngraph::Function>& f1, const std::shared_ptr<ngraph::Function>& f2) {
    /*
     * This function compares two nGraph functions and requires them to have exactly one output
     * + Check nodes types
     * + Check number of inputs
     * + Check shapes
     * + Check parent ports
     * + Check node attributes by Visitor API
     */

    auto f1_results = f1->get_results();
    auto f2_results = f2->get_results();

    std::sort(f1_results.begin(), f1_results.end(), less_by_name);
    std::sort(f2_results.begin(), f2_results.end(), less_by_name);

    if (f1_results.size() != f2_results.size()) {
        return Result::error(
            "Number of results is different: " + to_str(f1_results.size()) + " and " +
            to_str(f2_results.size()));
    }

    const auto& f1_sinks = f1->get_sinks();
    const auto& f2_sinks = f2->get_sinks();
    if (f1_sinks.size() != f2_sinks.size()) {
        return Result::error(
            "Number of sinks is different: " + to_str(f1_sinks.size()) + " and " +
            to_str(f2_sinks.size()));
    }

    for (size_t i = 0; i < f1_results.size(); ++i) {
        if (should_compare(CmpValues::NAMES)) {
            if (name(f1_results[i]->get_input_node_shared_ptr(0)) !=
                name(f2_results[i]->get_input_node_shared_ptr(0))) {
                return Result::error(
                    "Different output names: " + name(f1_results[i]->get_input_node_shared_ptr(0)) +
                    " and " + name(f2_results[i]->get_input_node_shared_ptr(0)));
            }
        }
        q.push({f1_results[i].get(), f2_results[i].get()});
        used.insert(f1_results[i].get());
    }

    std::stringstream errors;

    while (!q.empty()) {
        ngraph::Node* const node1 = q.front().first;
        ngraph::Node* const node2 = q.front().second;
        q.pop();

        const auto result = compare(node1, node2, errors);
        if (!result.valid) {
            return result;
        }

        add_nodes_inputs_to_queue(node1, node2);
    }
    const auto msg = errors.str();
    return msg.empty() ? Result::ok() : Result::error(msg);
}

Comparator::Result Comparator::compare(
    ngraph::Node* node1, ngraph::Node* node2, std::ostream& err_log) {
    auto type_info1 = node1->get_type_info();
    auto type_info2 = node2->get_type_info();

    if (!compareTypeInfo(type_info1, type_info2)) {
        return Result::error(typeInfoToStr(type_info1) + " != " + typeInfoToStr(type_info2));
    }

    auto subgraph1 = dynamic_cast<ngraph::op::util::SubGraphOp*>(node1);
    auto subgraph2 = dynamic_cast<ngraph::op::util::SubGraphOp*>(node2);

    if (subgraph1 && subgraph2) {
        auto result = recreate().compare(subgraph1->get_function(), subgraph2->get_function());
        if (!result.valid) {
            return result;
        }
    }

    const auto& dependencies_1 = node1->get_control_dependencies();
    const auto& dependencies_2 = node2->get_control_dependencies();

    if (dependencies_1.size() != dependencies_2.size()) {
        return Result::error(
            "Number of dependencies is different: " + to_str(dependencies_1.size()) + " for " +
            name(node1) + " and " + to_str(dependencies_2.size()) + " for " + name(node2));
    }

    if (node1->inputs().size() != node2->inputs().size()) {
        return Result::error(
            "Number of inputs is different: " + to_str(node1->inputs().size()) + " for " +
            name(node1) + " and " + to_str(node2->inputs().size()) + " for " + name(node2));
    }

    if (node1->outputs().size() != node2->outputs().size()) {
        return Result::error(
            "Number of outputs is different: " + to_str(node1->inputs().size()) + " for " +
            name(node1) + " and " + to_str(node2->inputs().size()) + " for " + name(node2));
    }

    for (int i = 0; i < node1->inputs().size(); ++i) {
        if (should_compare(CmpValues::CONST_VALUES)) {
            using Constant = ngraph::opset1::Constant;
            auto const1 = ngraph::as_type_ptr<Constant>(node1->get_input_node_shared_ptr(i));
            auto const2 = ngraph::as_type_ptr<Constant>(node2->get_input_node_shared_ptr(i));
            using namespace ::attr_comparison::equal;
            if (const1 && const2 &&
                !Equal<std::shared_ptr<Constant>>::equal_value(const1, const2)) {
                err_log << "Different Constant values detected\n"
                        << node1->description() << " Input(" << i << ") and "
                        << node2->description() << " Input(" << i << ")" << std::endl;
            }
        }

        if (should_compare(CmpValues::PRECISIONS)) {
            if (node1->input(i).get_element_type() != node2->input(i).get_element_type()) {
                err_log << "Different element type detected\n"
                        << name(node1) << " Input(" << i << ") "
                        << node1->input(i).get_element_type() << " and " << name(node2) << " Input("
                        << i << ") " << node2->input(i).get_element_type() << std::endl;
            }
        }

        if (!node1->input(i).get_partial_shape().same_scheme(node2->input(i).get_partial_shape())) {
            err_log << "Different shape detected\n"
                    << name(node1) << " Input(" << i << ") " << node1->input(i).get_partial_shape()
                    << " and " << name(node2) << " Input(" << i << ") "
                    << node2->input(i).get_partial_shape() << std::endl;
        }

        if (node1->get_input_source_output(i).get_index() !=
            node2->get_input_source_output(i).get_index()) {
            auto idx1 = node1->get_input_source_output(i).get_index();
            auto idx2 = node2->get_input_source_output(i).get_index();
            err_log << "Different ports detected\n"
                    << name(node1) << " Input(" << i << ") connected to parent port " << idx1
                    << " and " << name(node2) << " Input(" << i << ") connected to parent port "
                    << idx2 << std::endl;
        }

        if (should_compare(CmpValues::RUNTIME_KEYS) && !compare_rt_keys(node1, node2)) {
            err_log << "Different runtime info detected\n"
                    << name(node1) << " and " << name(node2) << " not equal runtime info."
                    << std::endl;
        }
    }

    for (int i = 0; i < node1->outputs().size(); ++i) {
        const auto& tensor1 = node1->output(i).get_tensor();
        const auto& tensor2 = node2->output(i).get_tensor();

        if (tensor1.get_names() != tensor2.get_names()) {
            err_log << "Output tensors names " << tensor_names(tensor1) << " and "
                    << tensor_names(tensor2)
                    << " are different for nodes: " << node1->get_friendly_name() << " and "
                    << node2->get_friendly_name() << std::endl;
        }

        if (!node1->output(i).get_partial_shape().same_scheme(
                node2->output(i).get_partial_shape())) {
            err_log << "Different shape detected\n"
                    << name(node1) << " Output(" << i << ") "
                    << node1->output(i).get_partial_shape() << " and " << name(node2) << " Output("
                    << i << ") " << node2->output(i).get_partial_shape() << std::endl;
        }
    }

    if (should_compare(CmpValues::ATTRIBUTES)) {
        CompareNodesAttributes compare_nodes;
        node1->visit_attributes(compare_nodes.get_ref_reader());
        node2->visit_attributes(compare_nodes.get_cmp_reader());
        if (!compare_nodes.equal()) {
            return Result::error(
                "Comparison of attributes failed for nodes " + name(node1) + ", " + name(node2) +
                " [cmp status: " + to_string(compare_nodes) + "]");
        }
    }

    return Result::ok("Check if any minor error was log in to err_log");
}

void Comparator::add_nodes_inputs_to_queue(ngraph::Node* node1, ngraph::Node* node2) {
    for (int i = 0; i < node1->inputs().size(); ++i) {
        if (!used.count(node1->input_value(i).get_node())) {
            q.push({node1->input_value(i).get_node(), node2->input_value(i).get_node()});
            used.insert(node1->input_value(i).get_node());
        }
    }
}

}  // namespace

FunctionsComparator::Result FunctionsComparator::compare(
    const std::shared_ptr<ngraph::Function>& f1,
    const std::shared_ptr<ngraph::Function>& f2) const {
    return Comparator(m_comparition_flags).compare(f1, f2);
}

void check_rt_info(const std::shared_ptr<ngraph::Function>& f) {
    static const std::vector<std::string> attrs_to_check{"Variant::RuntimeAttribute::FusedNames"};

    std::ostringstream err_log;
    for (auto& op : f->get_ops()) {
        if (ngraph::op::is_constant(op)) continue;

        const auto& rt_info = op->get_rt_info();
        for (const auto& attr_name : attrs_to_check) {
            if (!rt_info.count(attr_name)) {
                err_log << "Node: " << op->get_friendly_name() << " has no attribute: " << attr_name
                        << std::endl;
            }
        }
    }

    auto err_msg = err_log.str();
    if (!err_msg.empty()) {
        throw ngraph::ngraph_error(err_msg);
    }
}

NGRAPH_RTTI_DEFINITION(TestOpMultiOut, "TestOp", 0);

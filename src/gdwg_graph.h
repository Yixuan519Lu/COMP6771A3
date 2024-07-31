#ifndef GDWG_GRAPH_H
#define GDWG_GRAPH_H
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
namespace std {
	template<typename T1, typename T2>
	struct hash<std::pair<T1, T2>> {
		auto operator()(const std::pair<T1, T2>& p) const -> std::size_t {
			auto h1 = std::hash<T1>{}(p.first);
			auto h2 = std::hash<T2>{}(p.second);
			return (h1 << 1) ^ h2;
		}
	};
} // namespace std
namespace gdwg {
	template<typename N, typename E>
	class graph;
	template<typename N, typename E>
	class edge {
	 public:
		virtual ~edge() noexcept = default;
		virtual auto print_edge() const -> std::string = 0;
		virtual auto is_weighted() const noexcept -> bool = 0;
		virtual auto get_weight() const noexcept -> std::optional<E> = 0;
		virtual auto get_nodes() const noexcept -> std::pair<N, N> = 0;
		virtual auto operator==(const edge<N, E>& rhs) const noexcept -> bool = 0;

	 private:
	};
	template<typename N, typename E>
	class weighted_edge : public edge<N, E> {
	 public:
		weighted_edge(const N& src, const N& dst, const E& weight) noexcept
		: src_{src}
		, dst_{dst}
		, weight_{weight} {}
		auto print_edge() const -> std::string override {
			auto oss = std::ostringstream{};
			oss << src_ << " -> " << dst_ << " | W | " << weight_;
			return oss.str();
		}
		auto is_weighted() const noexcept -> bool override {
			return true;
		}
		auto get_weight() const noexcept -> std::optional<E> override {
			return weight_;
		}
		auto get_nodes() const noexcept -> std::pair<N, N> override {
			return {src_, dst_};
		}
		auto operator==(const edge<N, E>& rhs) const noexcept -> bool override {
			if (auto* obj = dynamic_cast<const weighted_edge<N, E>*>(&rhs)) {
				return src_ == obj->src_ and dst_ == obj->dst_ and weight_ == obj->weight_;
			}
			return false;
		}

	 private:
		N src_;
		N dst_;
		E weight_;
	};
	template<typename N, typename E>
	class unweighted_edge : public edge<N, E> {
	 public:
		unweighted_edge(const N& src, const N& dst) noexcept
		: src_{src}
		, dst_{dst} {}
		auto print_edge() const -> std::string override {
			auto oss = std::ostringstream{};
			oss << src_ << " -> " << dst_ << " | U";
			return oss.str();
		}
		auto is_weighted() const noexcept -> bool override {
			return false;
		}
		auto get_weight() const noexcept -> std::optional<E> override {
			return std::nullopt;
		}
		auto get_nodes() const noexcept -> std::pair<N, N> override {
			return {src_, dst_};
		}
		auto operator==(const edge<N, E>& rhs) const noexcept -> bool override {
			if (auto* obj = dynamic_cast<const unweighted_edge<N, E>*>(&rhs)) {
				return src_ == obj->src_ and dst_ == obj->dst_;
			}
			return false;
		}

	 private:
		N src_;
		N dst_;
	};
	template<typename N, typename E>
	class graph {
	 private:
		struct shared_ptr_less {
			auto operator()(const std::shared_ptr<N>& lhs, const std::shared_ptr<N>& rhs) const -> bool {
				return *lhs < *rhs;
			}
		};
		struct pair_less {
			auto operator()(const std::pair<std::shared_ptr<N>, std::optional<E>>& lhs,
			                const std::pair<std::shared_ptr<N>, std::optional<E>>& rhs) const -> bool {
				if (*lhs.first != *rhs.first) {
					return *lhs.first < *rhs.first;
				}
				return lhs.second < rhs.second;
			}
		};
		class my_iterator {
			using inner_iterator =
			    typename std::set<std::pair<std::shared_ptr<N>, std::optional<E>>, pair_less>::const_iterator;
			using outer_iterator = typename std::map<std::shared_ptr<N>,
			                                         std::set<std::pair<std::shared_ptr<N>, std::optional<E>>, pair_less>,
			                                         shared_ptr_less>::const_iterator;

		 public:
			struct value_type {
				N from;
				N to;
				std::optional<E> weight;
			};
			using reference = value_type;
			using pointer = void;
			using difference_type = std::ptrdiff_t;
			using iterator_category = std::bidirectional_iterator_tag;
			auto operator*() const -> reference {
				return value_type{*outer_begin_->first, *inner_->first, inner_->second};
			}
			auto operator++() -> my_iterator& {
				if (outer_begin_ != outer_end_) {
					++inner_;
					while (outer_begin_ != outer_end_ and inner_ == outer_begin_->second.end()) {
						++outer_begin_;
						if (outer_begin_ != outer_end_) {
							inner_ = outer_begin_->second.begin();
						}
					}
					if (outer_begin_ == outer_end_) {
						inner_ = inner_iterator{};
					}
				}
				return *this;
			}
			auto operator++(int) -> my_iterator {
				auto temp = *this;
				++(*this);
				return temp;
			}
			auto operator--() -> my_iterator& {
				if (outer_begin_ == outer_end_) {
					--outer_begin_;
					inner_ = outer_begin_->second.end();
				}
				while (inner_ == outer_begin_->second.begin()) {
					--outer_begin_;
					inner_ = outer_begin_->second.end();
				}
				--inner_;
				return *this;
			}
			auto operator--(int) -> my_iterator {
				auto temp = *this;
				--(*this);
				return temp;
			}
			auto operator==(const my_iterator& other) const -> bool {
				return outer_begin_ == other.outer_begin_ and inner_ == other.inner_;
			}

		 private:
			my_iterator() = default;
			explicit my_iterator(outer_iterator outer_begin, outer_iterator outer_end)
			: outer_begin_(outer_begin)
			, outer_end_(outer_end)
			, inner_{(outer_begin == outer_end) ? inner_iterator{} : outer_begin->second.begin()} {}
			explicit my_iterator(outer_iterator outer_begin, outer_iterator outer_end, inner_iterator inner)
			: outer_begin_(outer_begin)
			, outer_end_(outer_end)
			, inner_(inner) {}

		 private:
			outer_iterator outer_begin_;
			outer_iterator outer_end_;
			inner_iterator inner_;
			friend class graph<N, E>;
		};

	 public:
		using iterator = typename graph<N, E>::my_iterator;
		graph() = default;
		graph(graph&& other) noexcept {
			nodes_ = std::move(other.nodes_);
			edges_ = std::move(other.edges_);
			other.clear();
		}
		graph(const graph& other) {
			for (const auto& node : other.nodes_) {
				auto new_node = std::make_shared<N>(*node);
				nodes_.emplace(new_node);
			}
			for (const auto& [src, dst_set] : other.edges_) {
				auto new_src = find_node(*src);
				for (const auto& [dst, weight] : dst_set) {
					auto new_dst = find_node(*dst);
					edges_[new_src].emplace(new_dst, weight);
				}
			}
		}
		graph(std::initializer_list<N> il)
		: graph(il.begin(), il.end()) {}
		template<typename InputIt>
		graph(InputIt first, InputIt last) {
			for (auto ite = first; ite != last; ++ite) {
				nodes_.emplace(std::make_shared<N>(*ite));
			}
		}
		auto operator=(graph&& other) noexcept -> graph& {
			if (this != &other) {
				nodes_ = std::move(other.nodes_);
				edges_ = std::move(other.edges_);
				other.clear();
			}
			return *this;
		}
		auto operator=(const graph& other) -> graph& {
			if (this != &other) {
				nodes_.clear();
				edges_.clear();
				for (const auto& node : other.nodes_) {
					auto new_node = std::make_shared<N>(*node);
					nodes_.emplace(new_node);
				}
				for (const auto& [src, dst_set] : other.edges_) {
					auto new_src = find_node(*src);
					for (const auto& [dst, weight] : dst_set) {
						auto new_dst = find_node(*dst);
						edges_[new_src].emplace(new_dst, weight);
					}
				}
			}
			return *this;
		}
		auto clear() noexcept -> void {
			nodes_.clear();
			edges_.clear();
		}
		auto insert_node(const N& value) noexcept -> bool {
			for (const auto& node : nodes_) {
				if (*node == value) {
					return false;
				}
			}
			return nodes_.emplace(std::make_shared<N>(value)).second;
		}
		auto insert_edge(const N& src, const N& dst, std::optional<E> weight = std::nullopt) -> bool {
			const auto src_sp = find_node(src);
			const auto dst_sp = find_node(dst);
			if (not is_node(src) or not is_node(dst)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::insert_edge when either src or dst node does "
				                         "not exist");
			}
			auto& edge_set = edges_[src_sp];
			for (const auto& edge : edge_set) {
				if (*edge.first == dst and edge.second == weight) {
					return false;
				}
			}
			return edge_set.emplace(dst_sp, weight).second;
		}
		[[nodiscard]] auto is_node(const N& value) const noexcept -> bool {
			return find_node(value) != nullptr;
		}
		[[nodiscard]] auto empty() const noexcept -> bool {
			return nodes_.empty() and edges_.empty();
		}
		[[nodiscard]] auto nodes() const -> std::vector<N> {
			auto res = std::vector<N>{};
			for (const auto& node : nodes_) {
				res.push_back(*node);
			}
			return res;
		}
		[[nodiscard]] auto is_connected(const N& src, const N& dst) const -> bool {
			const auto src_sp = find_node(src);
			const auto dst_sp = find_node(dst);
			if (not is_node(src) or not is_node(dst)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::is_connected if src or dst node don't exist "
				                         "in the graph");
			}
			const auto src_it = edges_.find(src_sp);
			if (src_it != edges_.end()) {
				for (const auto& edge : src_it->second) {
					if (*edge.first == dst) {
						return true;
					}
				}
			}
			return false;
		}
		[[nodiscard]] auto edges(const N& src, const N& dst) const -> std::vector<std::unique_ptr<edge<N, E>>> {
			const auto src_sp = find_node(src);
			const auto dst_sp = find_node(dst);
			if (not is_node(src) or not is_node(dst)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::edges if src or dst node don't exist in the "
				                         "graph");
			}
			auto res = std::vector<std::unique_ptr<edge<N, E>>>{};
			const auto src_it = edges_.find(src_sp);
			if (src_it != edges_.end()) {
				for (const auto& edge_pair : src_it->second) {
					if (*edge_pair.first == dst) {
						if (edge_pair.second.has_value()) {
							res.emplace_back(std::make_unique<weighted_edge<N, E>>(src, dst, *edge_pair.second));
						}
						else {
							res.emplace_back(std::make_unique<unweighted_edge<N, E>>(src, dst));
						}
					}
				}
			}
			return res;
		}
		[[nodiscard]] auto find(const N& src, const N& dst, std::optional<E> weight = std::nullopt) const -> iterator {
			if (not is_node(src) or not is_node(dst)) {
				return end();
			}
			const auto src_sp = find_node(src);
			const auto dst_sp = find_node(dst);
			const auto src_it = edges_.find(src_sp);
			if (src_it == edges_.end()) {
				return end();
			}
			const auto& dst_set = src_it->second;
			const auto edge_it = dst_set.find({dst_sp, weight});
			if (edge_it != dst_set.end()) {
				return iterator(src_it, edges_.end(), edge_it);
			}
			return end();
		}
		[[nodiscard]] auto connections(const N& src) const -> std::vector<N> {
			const auto src_sp = find_node(src);
			if (not is_node(src)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::connections if src doesn't exist in the "
				                         "graph");
			}
			const auto it = edges_.find(src_sp);
			auto res = std::vector<N>{};
			auto last_node = std::optional<N>{};
			if (it != edges_.end()) {
				for (const auto& edge : it->second) {
					if (not last_node or last_node.value() != *edge.first) {
						res.push_back(*edge.first);
						last_node = *edge.first;
					}
				}
			}
			return res;
		}
		auto erase_node(const N& value) -> bool {
			const auto node_sp = find_node(value);
			if (not is_node(value)) {
				return false;
			}
			nodes_.erase(node_sp);
			edges_.erase(node_sp);
			for (auto& [src, dst_set] : edges_) {
				for (auto it = dst_set.begin(); it != dst_set.end();) {
					if (*(it->first) == value) {
						it = dst_set.erase(it);
					}
					else {
						++it;
					}
				}
			}
			return true;
		}
		auto erase_edge(const N& src, const N& dst, std::optional<E> weight = std::nullopt) -> bool {
			const auto src_sp = find_node(src);
			const auto dst_sp = find_node(dst);
			if (not is_node(src) or not is_node(dst)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::erase_edge on src or dst if they don't exist "
				                         "in the graph");
			}
			const auto src_it = edges_.find(src_sp);
			if (src_it == edges_.end()) {
				return false;
			}
			auto& dst_set = src_it->second;
			const auto edge_it = dst_set.find({dst_sp, weight});
			if (edge_it != dst_set.end()) {
				dst_set.erase(edge_it);
				if (dst_set.empty()) {
					edges_.erase(src_it);
				}
				return true;
			}
			return false;
		}
		auto erase_edge(iterator i) -> iterator {
			const auto src = i.outer_begin_->first;
			const auto dst = i.inner_->first;
			const auto weight = i.inner_->second;
			auto next_it = i;
			++next_it;
			if (next_it == end()) {
				erase_edge(*src, *dst, weight);
				return end();
			}
			else {
				const auto nsrc = next_it.outer_begin_->first;
				const auto ndst = next_it.inner_->first;
				const auto nweight = next_it.inner_->second;
				erase_edge(*src, *dst, weight);
				return find(*nsrc, *ndst, nweight);
			}
		}
		auto erase_edge(iterator i, iterator s) -> iterator {
			while (i != s) {
				i = erase_edge(i);
			}
			return s;
		}
		auto replace_node(const N& old_data, const N& new_data) -> bool {
			if (not is_node(old_data)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::replace_node on a node that doesn't exist");
			}
			if (is_node(new_data)) {
				return false;
			}
			if (old_data == new_data) {
				return true;
			}
			auto old_node_sp = find_node(old_data);
			auto new_node_sp = std::make_shared<N>(new_data);
			nodes_.emplace(new_node_sp);
			if (auto it = edges_.find(old_node_sp); it != edges_.end()) {
				for (const auto& edge : it->second) {
					edges_[new_node_sp].emplace(edge);
				}
				edges_.erase(it);
			}
			for (auto& [src, dst_set] : edges_) {
				for (auto it = dst_set.begin(); it != dst_set.end();) {
					if (*(it->first) == old_data) {
						dst_set.emplace(new_node_sp, it->second);
						it = dst_set.erase(it);
					}
					else {
						++it;
					}
				}
			}
			nodes_.erase(old_node_sp);
			return true;
		}
		auto merge_replace_node(const N& old_data, const N& new_data) -> void {
			if (not is_node(old_data) or not is_node(new_data)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::merge_replace_node on old or new data if they "
				                         "don't exist in the graph");
			}
			if (old_data == new_data) {
				return;
			}
			auto old_node_sp = find_node(old_data);
			auto new_node_sp = find_node(new_data);
			if (auto it = edges_.find(old_node_sp); it != edges_.end()) {
				for (const auto& edge : it->second) {
					auto& new_dst_set = edges_[new_node_sp];
					auto exists = false;
					for (const auto& new_edge : new_dst_set) {
						if (*(new_edge.first) == *(edge.first) and new_edge.second == edge.second) {
							exists = true;
							break;
						}
					}
					if (not exists) {
						new_dst_set.emplace(edge);
					}
				}
				edges_.erase(it);
			}
			for (auto& [src, dst_set] : edges_) {
				for (auto it = dst_set.begin(); it != dst_set.end();) {
					if (*(it->first) == old_data) {
						auto exists = false;
						for (const auto& new_edge : dst_set) {
							if (*(new_edge.first) == new_data and new_edge.second == it->second) {
								exists = true;
								break;
							}
						}
						if (not exists) {
							dst_set.emplace(new_node_sp, it->second);
						}
						it = dst_set.erase(it);
					}
					else {
						++it;
					}
				}
			}
			nodes_.erase(old_node_sp);
		}
		friend auto operator<<(std::ostream& os, const graph& g) -> std::ostream& {
			if (g.nodes_.empty()) {
				return os;
			}
			for (const auto& node : g.nodes_) {
				os << *node << " (\n";
				if (auto it = g.edges_.find(node); it != g.edges_.end()) {
					for (const auto& edge_pair : it->second) {
						if (edge_pair.second == std::nullopt) {
							os << "  " << *node << " -> " << *(edge_pair.first) << " | U\n";
						}
					}
					for (const auto& edge_pair : it->second) {
						if (edge_pair.second != std::nullopt) {
							os << "  " << *node << " -> " << *(edge_pair.first) << " | W | " << *edge_pair.second << "\n";
						}
					}
				}
				os << ")\n";
			}
			return os;
		}
		[[nodiscard]] auto begin() const -> iterator {
			return iterator(edges_.cbegin(), edges_.cend());
		}
		[[nodiscard]] auto end() const -> iterator {
			return iterator(edges_.cend(), edges_.cend());
		}
		[[nodiscard]] auto operator==(const graph& other) const -> bool {
			if (nodes_.size() != other.nodes_.size() or edges_.size() != other.edges_.size()) {
				return false;
			}
			for (const auto& node : nodes_) {
				auto found = false;
				for (const auto& other_node : other.nodes_) {
					if (*node == *other_node) {
						found = true;
						break;
					}
				}
				if (not found) {
					return false;
				}
			}
			for (const auto& [src, dst_set] : edges_) {
				const auto other_it = std::find_if(other.edges_.begin(), other.edges_.end(), [&](const auto& other_pair) {
					return *src == *other_pair.first;
				});
				if (other_it == other.edges_.end()) {
					return false;
				}
				const auto& other_dst_set = other_it->second;
				if (dst_set.size() != other_dst_set.size()) {
					return false;
				}
				for (const auto& edge : dst_set) {
					auto found = false;
					for (const auto& other_edge : other_dst_set) {
						if (*edge.first == *other_edge.first and edge.second == other_edge.second) {
							found = true;
							break;
						}
					}
					if (not found) {
						return false;
					}
				}
			}
			return true;
		}

	 private:
		std::set<std::shared_ptr<N>, shared_ptr_less> nodes_;
		std::map<std::shared_ptr<N>, std::set<std::pair<std::shared_ptr<N>, std::optional<E>>, pair_less>, shared_ptr_less> edges_;
		auto find_node(const N& value) const noexcept -> std::shared_ptr<N> {
			for (const auto& node : nodes_) {
				if (*node == value) {
					return node;
				}
			}
			return nullptr;
		}
	};
} // namespace gdwg

#endif // GDWG_GRAPH_H
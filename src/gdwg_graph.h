#ifndef GDWG_GRAPH_H
#define GDWG_GRAPH_H
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

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
		friend class graph<N, E>;
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
		friend class graph<N, E>;
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
		friend class graph<N, E>;
	};
	template<typename N, typename E>
	class graph {
	 public:
		graph() = default;
		graph(graph&& other) noexcept {
			nodes_ = std::move(other.nodes_);
			edges_ = std::move(other.edges_);
			other.clear();
		}
		graph(const graph& other) {
			nodes_ = other.nodes_;
			edges_ = other.edges_;
		}
		graph(std::initializer_list<N> il)
		: graph(il.begin(), il.end()) {}
		template<typename InputIt>
		graph(InputIt first, InputIt last) {
			for (auto ite = first; ite != last; ++ite) {
				nodes_.emplace(*ite);
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
				nodes_ = other.nodes_;
				edges_ = other.edges_;
			}
			return *this;
		}
		void clear() noexcept {
			nodes_.clear();
			edges_.clear();
		}

		auto insert_node(const N& value) -> bool {
			if (is_node(value)) {
				throw std::runtime_error("Duplicate node");
			}
			return nodes_.emplace(value).second;
		}
		auto insert_edge(const N& src, const N& dst, std::optional<E> weight = std::nullopt) -> bool {
			if (not is_node(src) or not is_node(dst)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::insert_edge when either src or dst node does "
				                         "not exist");
			}
			auto& edge_set = edges_[src];
			for (const auto& edge : edge_set) {
				if (edge.first == dst and edge.second == weight) {
					return false;
				}
			}
			return edge_set.emplace(dst, weight).second;
		}
		auto is_node(const N& value) const noexcept -> bool {
			return nodes_.find(value) != nodes_.end();
		}
		auto erase_node(const N& value) -> bool {
			if (not is_node(value)) {
				return false;
			}
			nodes_.erase(value);
			edges_.erase(value);
			for (auto& [src, dst_set] : edges_) {
				for (auto it = dst_set.begin(); it != dst_set.end();) {
					if (it->first == value) {
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
			if (not is_node(src) or not is_node(dst)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::erase_edge on src or dst if they don't exist "
				                         "in the graph");
			}
			const auto src_it = edges_.find(src);
			if (src_it == edges_.end()) {
				return false;
			}
			auto& dst_set = src_it->second;
			auto erased = false;
			if (weight) {
				const auto edge_it = dst_set.find({dst, *weight});
				if (edge_it != dst_set.end()) {
					dst_set.erase(edge_it);
					erased = true;
				}
			}
			else {
				for (auto it = dst_set.begin(); it != dst_set.end();) {
					if (it->first == dst) {
						it = dst_set.erase(it);
						erased = true;
					}
					else {
						++it;
					}
				}
			}
			if (dst_set.empty()) {
				edges_.erase(src);
			}
			return erased;
		}

	 private:
		std::unordered_set<N> nodes_;
		std::unordered_map<N, std::unordered_set<std::pair<N, E>>> edges_;
	};
} // namespace gdwg

#endif // GDWG_GRAPH_H
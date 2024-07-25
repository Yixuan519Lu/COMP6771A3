#ifndef GDWG_GRAPH_H
#define GDWG_GRAPH_H
#include <memory>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// TODO: Make both graph and edge generic
//       ... this won't just compile
//       straight away
namespace gdwg {
	template<typename N, typename E>
	class graph;

	template<typename N, typename E>
	class edge {
	 public:
		virtual ~edge() = default;
		virtual auto print_edge() const -> std::string = 0;
		virtual auto is_weighted() const -> bool = 0;
		virtual auto get_weight() const -> std::optional<E> = 0;
		virtual auto get_nodes() const -> std::pair<N, N> = 0;
		virtual auto operator==(edge const& other) const -> bool = 0;

	 private:
		friend class graph<N, E>;
	};
	template<typename N, typename E>
	class weighted_edge : public edge<N, E> {
	 public:
		weighted_edge(const N& src, const N& dst, const E& weight)
		: src_{src}
		, dst_{dst}
		, weight_{weight} {}

		auto print_edge() const -> std::string override {
			auto oss = std::ostringstream{};
			oss << src_ << " -> " << dst_ << " | W | " << weight_;
			return oss.str();
		}
		auto is_weighted() const -> bool override {
			return true;
		}
		auto get_weight() const -> std::optional<E> override {
			return weight_;
		}
		auto get_nodes() const -> std::pair<N, N> override {
			return {src_, dst_};
		}
		auto operator==(edge<N, E> const& rhs) const -> bool override {
			if (auto* obj = dynamic_cast<weighted_edge<N, E> const*>(&rhs)) {
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
	class graph {
	 public:
		graph() = default;
		~graph() = default;

	 private:
	};
} // namespace gdwg

#endif // GDWG_GRAPH_H

#ifndef GDWG_GRAPH_H
#define GDWG_GRAPH_H
#include <optional>
#include <string>
#include <utility>

// TODO: Make both graph and edge generic
//       ... this won't just compile
//       straight away
namespace gdwg {
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
		friend class graph;
	};
	class graph {
	 public:
		// Your member functions go here
	 private:
	};
} // namespace gdwg

#endif // GDWG_GRAPH_H

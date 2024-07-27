#include "gdwg_graph.h"

#include <catch2/catch.hpp>

TEST_CASE("gdwg::graph") {
	SECTION("Constructors") {
		SECTION("Default constructor") {
			auto g1 = gdwg::graph<int, std::string>{};
			CHECK(g1.empty());
			auto g2 = gdwg::graph<double, double>{};
			CHECK(g2.empty());
		}
		SECTION("Move constructor") {
			auto g1 = gdwg::graph<int, std::string>{1, 2, 3};
			auto n = 5;
			g1.insert_node(n);
			auto g2 = std::move(g1);
			CHECK(not g1.is_node(1));
			CHECK(not g1.is_node(2));
			CHECK(not g1.is_node(3));
			CHECK(not g1.is_node(5));
			CHECK(g1.empty());
			CHECK(g2.is_node(1));
			CHECK(g2.is_node(2));
			CHECK(g2.is_node(3));
			CHECK(g2.is_node(5));
		}
	}
	SECTION("Modifiers") {
		SECTION("insert_node") {
			auto g = gdwg::graph<int, std::string>{};

			SECTION("Insert a new node") {
				CHECK(g.insert_node(1));
				CHECK(g.is_node(1));
			}

			SECTION("Insert a duplicate node") {
				g.insert_node(2);
				CHECK(not g.insert_node(2));
			}

			SECTION("Insert multiple nodes") {
				CHECK(g.insert_node(3));
				CHECK(g.insert_node(4));
				CHECK(g.is_node(3));
				CHECK(g.is_node(4));
			}
		}
	}
	SECTION("Extractor") {
		SECTION("Example Test") {
			using graph = gdwg::graph<int, int>;
			auto const v = std::vector<std::tuple<int, int, std::optional<int>>>{
			    {4, 1, -4},
			    {3, 2, 2},
			    {2, 4, std::nullopt},
			    {2, 1, 1},
			    {6, 2, 5},
			    {6, 3, 10},
			    {1, 5, -1},
			    {3, 6, -8},
			    {4, 5, 3},
			    {5, 2, std::nullopt},
			};

			auto g = graph{};
			for (const auto& [from, to, weight] : v) {
				g.insert_node(from);
				g.insert_node(to);
				if (weight.has_value()) {
					g.insert_edge(from, to, weight.value());
				}
				else {
					g.insert_edge(from, to);
				}
			}
			g.insert_node(64);
			auto out = std::ostringstream{};
			out << g;
			auto const expected_output = std::string_view(R"(
1 (
  1 -> 5 | W | -1
)
2 (
  2 -> 4 | U
  2 -> 1 | W | 1
)
3 (
  3 -> 2 | W | 2
  3 -> 6 | W | -8
)
4 (
  4 -> 1 | W | -4
  4 -> 5 | W | 3
)
5 (
  5 -> 2 | U
)
6 (
  6 -> 2 | W | 5
  6 -> 3 | W | 10
)
64 (
)
)");
			CHECK(out.str() == expected_output);
		}
	}
}

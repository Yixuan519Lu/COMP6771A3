#include "gdwg_graph.h"

#include <catch2/catch.hpp>

TEST_CASE("gdwg::graph") {
	SECTION("Constructors") {
		using graph = gdwg::graph<int, int>;
		SECTION("Default constructor") {
			auto g1 = graph{};
			CHECK(g1.empty());
		}
		SECTION("Copy and Move constructor") {
			SECTION("nonempty") {
				auto g1 = graph{1, 2};
				g1.insert_edge(1, 2, 2);
				auto g2 = graph{g1};
				CHECK(g2.is_node(1));
				CHECK(g2.is_node(2));
				CHECK(g2.is_connected(1, 2));
				CHECK(g2 == g1);
				auto g3 = std::move(g1);
				CHECK(g1.empty());
				CHECK(g3.is_node(1));
				CHECK(g3.is_node(2));
				CHECK(g3.is_connected(1, 2));
			}
			SECTION("empty") {
				auto g1 = graph{};
				auto g2 = graph{g1};
				CHECK(g1.empty());
				CHECK(g2 == g1);
				auto g3 = std::move(g1);
				CHECK(g3.empty());
			}
		}
		SECTION("Initializer list constructor") {
			auto g = graph{1, 2, 3};
			CHECK(g.is_node(1));
			CHECK(g.is_node(2));
			CHECK(g.is_node(3));
			CHECK(not g.is_connected(1, 2));
		}

		SECTION("Range constructor") {
			auto nodes = std::vector<int>{1, 2, 3};
			auto g = graph(nodes.begin(), nodes.end());
			CHECK(g.is_node(1));
			CHECK(g.is_node(2));
			CHECK(g.is_node(3));
			CHECK(not g.is_connected(1, 2));
		}
		SECTION("Copy and Move assignment operator") {
			auto g1 = graph{1, 2};
			g1.insert_edge(1, 2, 2);
			auto g2 = graph{};
			g2 = std::move(g1);
			CHECK(g1.empty());
			CHECK(g2.is_node(1));
			CHECK(g2.is_node(2));
			CHECK(g2.is_connected(1, 2));
			auto edges_1_2 = g2.edges(1, 2);
			CHECK(edges_1_2.size() == 1);
			CHECK(edges_1_2[0]->get_weight() == 2);
			auto g3 = graph{};
			g3 = g2;
			CHECK(g3 == g2);
		}
	}
	SECTION("Modifiers") {
		using graph = gdwg::graph<int, int>;
		SECTION("insert_node") {
			auto g = graph{};
			SECTION("new node") {
				CHECK(g.insert_node(1));
				CHECK(g.is_node(1));
			}
			SECTION("duplicate node") {
				g.insert_node(2);
				CHECK(not g.insert_node(2));
			}
			SECTION("multiple nodes") {
				CHECK(g.insert_node(3));
				CHECK(g.insert_node(4));
				CHECK(g.is_node(3));
				CHECK(g.is_node(4));
			}
		}
		SECTION("replace_node") {
			auto g = graph{1, 2, 3};
			g.insert_edge(1, 2, 1);
			g.insert_edge(2, 3, 2);
			SECTION("success") {
				CHECK(g.replace_node(1, 5));
				auto nodes = g.nodes();
				const auto expected_nodes = std::vector<int>{2, 3, 5};
				CHECK(nodes == expected_nodes);
				auto edges_5_2 = g.edges(5, 2);
				CHECK(edges_5_2.size() == 1);
				CHECK(edges_5_2[0]->get_weight() == 1);
				auto edges_2_3 = g.edges(2, 3);
				CHECK(edges_2_3.size() == 1);
				CHECK(edges_2_3[0]->get_weight() == 2);
			}
			SECTION("node not exists") {
				CHECK_THROWS_WITH(g.replace_node(4, 5),
				                  "Cannot call gdwg::graph<N, E>::replace_node on a node that doesn't exist");
			}
			SECTION("node exists") {
				CHECK(not g.replace_node(1, 2));
				auto nodes = g.nodes();
				const auto expected_nodes = std::vector<int>{1, 2, 3};
				CHECK(nodes == expected_nodes);
				auto edges_1_2 = g.edges(1, 2);
				CHECK(edges_1_2.size() == 1);
				CHECK(edges_1_2[0]->get_weight() == 1);
				auto edges_2_3 = g.edges(2, 3);
				CHECK(edges_2_3.size() == 1);
				CHECK(edges_2_3[0]->get_weight() == 2);
			}
		}
		SECTION("merge_node") {
			SECTION("Merging success") {
				auto g = gdwg::graph<int, int>{1, 2, 3};
				g.insert_edge(1, 2, 1);
				g.insert_edge(1, 2);
				g.insert_edge(1, 1, 3);
				g.insert_edge(1, 3, 2);
				g.insert_edge(2, 3, 3);
				g.insert_edge(3, 2);
				g.insert_edge(3, 1, 4);
				g.insert_edge(3, 2, 5);
				g.merge_replace_node(1, 3);
				auto nodes = g.nodes();
				const auto expected_nodes = std::vector<int>{2, 3};
				CHECK(nodes == expected_nodes);
				auto edges_2_3 = g.edges(2, 3);
				CHECK(edges_2_3.size() == 1);
				CHECK(edges_2_3[0]->get_weight() == 3);
				auto edges_3_2 = g.edges(3, 2);
				CHECK(edges_3_2.size() == 3);
				const auto expected_weights_3_2 = std::vector<std::optional<int>>{std::nullopt, 1, 5};
				auto expected_it = expected_weights_3_2.begin();
				for (const auto& edge : edges_3_2) {
					CHECK(edge->get_weight() == *expected_it);
					++expected_it;
				}
				auto edges_3_3 = g.edges(3, 3);
				CHECK(edges_3_3.size() == 3);
				const auto expected_weights_3_3 = std::vector<int>{2, 3, 4};
				auto expected_it_3_3 = expected_weights_3_3.begin();
				for (const auto& edge : edges_3_3) {
					CHECK(edge->get_weight() == *expected_it_3_3);
					++expected_it_3_3;
				}
			}
			SECTION("Merging success 2") {
				auto g = gdwg::graph<char, int>{'A', 'B', 'C', 'D'};
				g.insert_edge('A', 'B', 3);
				g.insert_edge('C', 'B', 2);
				g.insert_edge('D', 'B', 4);

				g.merge_replace_node('B', 'A');
				auto nodes = g.nodes();
				const auto expected_nodes = std::vector<char>{'A', 'C', 'D'};
				CHECK(nodes == expected_nodes);
				auto edges_A_A = g.edges('A', 'A');
				CHECK(edges_A_A.size() == 1);
				CHECK(edges_A_A[0]->get_weight() == 3);
				auto edges_C_A = g.edges('C', 'A');
				CHECK(edges_C_A.size() == 1);
				CHECK(edges_C_A[0]->get_weight() == 2);
				auto edges_D_A = g.edges('D', 'A');
				CHECK(edges_D_A.size() == 1);
				CHECK(edges_D_A[0]->get_weight() == 4);
			}
		}
		SECTION("Insert edges") {
			auto g = graph{1, 2, 3};

			SECTION("unweighted") {
				CHECK(g.insert_edge(1, 2));
				auto edges_1_2 = g.edges(1, 2);
				CHECK(edges_1_2.size() == 1);
				CHECK(edges_1_2[0]->get_weight() == std::nullopt);
			}

			SECTION("weighted") {
				CHECK(g.insert_edge(1, 2, 1));
				auto edges_1_2 = g.edges(1, 2);
				CHECK(edges_1_2.size() == 1);
				CHECK(edges_1_2[0]->get_weight() == 1);
			}

			SECTION("duplicate unweighted") {
				g.insert_edge(1, 2);
				CHECK(not g.insert_edge(1, 2));
				auto edges_1_2 = g.edges(1, 2);
				CHECK(edges_1_2.size() == 1);
				CHECK(edges_1_2[0]->get_weight() == std::nullopt);
			}

			SECTION("multiple weighted") {
				g.insert_edge(1, 2, 1);
				CHECK(not g.insert_edge(1, 2, 1));
				g.insert_edge(1, 2);
				g.insert_edge(1, 2, 3);
				g.insert_edge(1, 2, 2);
				auto edges_1_2 = g.edges(1, 2);
				CHECK(edges_1_2.size() == 4);
				CHECK(edges_1_2[0]->get_weight() == std::nullopt);
				CHECK(edges_1_2[1]->get_weight() == 1);
				CHECK(edges_1_2[2]->get_weight() == 2);
				CHECK(edges_1_2[3]->get_weight() == 3);
			}
		}
		SECTION("Erase node") {
			auto g = graph{1, 2, 3, 4};
			g.insert_edge(1, 2, 10);
			g.insert_edge(1, 3, 20);
			g.insert_edge(2, 3, 30);
			g.insert_edge(3, 1, 30);
			SECTION("node with edges") {
				CHECK(g.erase_node(1));
				CHECK(not g.is_node(1));
				CHECK_THROWS_WITH(g.is_connected(1, 2),
				                  "Cannot call gdwg::graph<N, E>::is_connected if src or dst node don't exist in the "
				                  "graph");
				CHECK(g.is_node(2));
				CHECK(g.is_node(3));
			}
			SECTION("node without edges") {
				CHECK(g.erase_node(4));
				CHECK(not g.is_node(4));
				CHECK_THROWS_WITH(g.is_connected(1, 4),
				                  "Cannot call gdwg::graph<N, E>::is_connected if src or dst node don't exist in the "
				                  "graph");
			}

			SECTION("dne") {
				CHECK(not g.erase_node(5));
			}
		}
		SECTION("Erase edge") {
			auto g = graph{1, 2, 3};
			g.insert_edge(1, 2, 10);
			g.insert_edge(1, 3, 20);
			g.insert_edge(2, 3, 30);
			g.insert_edge(1, 2);
			SECTION("weighted edge") {
				CHECK(g.erase_edge(1, 2, 10));
				auto edges_1_2 = g.edges(1, 2);
				CHECK(edges_1_2.size() == 1);
				CHECK(edges_1_2[0]->get_weight() == std::nullopt);
			}
			SECTION("unweighted edge") {
				CHECK(g.erase_edge(1, 2));
				auto edges_1_2 = g.edges(1, 2);
				CHECK(edges_1_2.size() == 1);
				CHECK(edges_1_2[0]->get_weight() == 10);
			}
			SECTION("edge dne") {
				CHECK(not g.erase_edge(1, 2, 15));
			}
			SECTION("node dne") {
				CHECK_THROWS_WITH(g.erase_edge(4, 2),
				                  "Cannot call gdwg::graph<N, E>::erase_edge on src or dst if they don't exist in the "
				                  "graph");
				CHECK_THROWS_WITH(g.erase_edge(1, 4),
				                  "Cannot call gdwg::graph<N, E>::erase_edge on src or dst if they don't exist in the "
				                  "graph");
			}
			SECTION("erase all") {
				CHECK(g.erase_edge(1, 2, 10));
				CHECK(g.erase_edge(1, 3, 20));
				CHECK(g.erase_edge(2, 3, 30));
				CHECK(g.erase_edge(1, 2));
				CHECK(g.edges(1, 2).empty());
				CHECK(g.edges(1, 3).empty());
				CHECK(g.edges(2, 3).empty());
				CHECK(g.find(1, 2, 10) == g.end());
			}
		}
		SECTION("Erase edge iterator i") {
			SECTION("single edge") {
				auto g = graph{1, 2, 3};
				g.insert_edge(1, 2, 10);
				auto it = g.find(1, 2, 10);
				it = g.erase_edge(it);
				CHECK(it == g.end());
				auto edges_1_2 = g.edges(1, 2);
				CHECK(edges_1_2.empty());
			}
			SECTION("return next iterator") {
				auto g = graph{1, 2, 3};
				g.insert_edge(1, 2, 10);
				g.insert_edge(1, 3, 20);
				auto it = g.find(1, 2, 10);
				it = g.erase_edge(it);
				CHECK(it != g.end());
				CHECK(it == g.find(1, 3, 20));
			}
			SECTION("Erase all edges from a node") {
				auto g = graph{1, 2, 3};
				g.insert_edge(1, 2, 10);
				g.insert_edge(1, 3, 20);
				g.insert_edge(2, 3, 20);
				auto it = g.find(1, 2, 10);
				it = g.erase_edge(it);
				it = g.erase_edge(it);
				auto edges_1_2 = g.edges(1, 3);
				CHECK(edges_1_2.empty());
				auto edges_1_3 = g.edges(1, 3);
				CHECK(edges_1_3.empty());
				auto edge = *(g.find(2, 3, 20));
				CHECK(edge.from == 2);
				CHECK(edge.to == 3);
				CHECK(edge.weight == 20);
				CHECK(it != g.end());
				CHECK(it == g.find(2, 3, 20));
			}
			SECTION("all edges") {
				auto g = graph{1, 2, 3};
				g.insert_edge(1, 2, 10);
				g.insert_edge(1, 3, 20);
				g.insert_edge(2, 3, 30);
				auto it = g.begin();
				it = g.erase_edge(it);
				CHECK(it == g.find(1, 3, 20));
				for (auto it = g.begin(); it != g.end();) {
					it = g.erase_edge(it);
				}
				for (auto node : g.nodes()) {
					CHECK(g.connections(node).empty());
				}
			}
		}
		SECTION("Erase edge range") {
			SECTION("erase all") {
				auto g = graph{1, 2, 3};
				g.insert_edge(1, 2, 10);
				g.insert_edge(1, 3, 20);
				g.insert_edge(2, 3, 30);
				auto i = g.begin();
				auto s = g.end();
				g.erase_edge(i, s);
				CHECK(g.edges(1, 2).empty());
				CHECK(g.edges(1, 3).empty());
				CHECK(g.edges(2, 3).empty());
			}

			SECTION("erase subset") {
				auto g = graph{1, 2, 3};
				g.insert_edge(1, 2, 10);
				g.insert_edge(1, 3, 20);
				g.insert_edge(2, 3, 30);
				g.insert_edge(3, 1, 40);
				auto i = g.find(1, 2, 10);
				auto s = g.find(2, 3, 30);
				g.erase_edge(i, s);
				CHECK(g.edges(1, 2).empty());
				CHECK(g.edges(1, 3).empty());
				CHECK(g.edges(2, 3).size() == 1);
				CHECK(g.edges(3, 1).size() == 1);
				CHECK(g.edges(2, 3)[0]->get_weight() == 30);
				CHECK(g.edges(3, 1)[0]->get_weight() == 40);
			}

			SECTION("i=s") {
				auto g = graph{1, 2, 3};
				g.insert_edge(1, 2, 10);
				g.insert_edge(1, 3, 20);
				auto i = g.find(1, 2, 10);
				auto s = i;
				g.erase_edge(i, s);
				CHECK(g.edges(1, 2).size() == 1);
				CHECK(g.edges(1, 2)[0]->get_weight() == 10);
				CHECK(g.edges(1, 3).size() == 1);
				CHECK(g.edges(1, 3)[0]->get_weight() == 20);
			}
		}
		SECTION("clear") {
			auto g = graph{1, 2, 3};
			g.insert_edge(1, 2, 10);
			g.insert_edge(1, 3, 20);
			g.insert_edge(2, 3, 30);
			g.clear();
			CHECK(g.empty());
		}
	}
	SECTION("Iterator tests") {
		using graph = gdwg::graph<int, int>;
		SECTION("traverse") {
			auto g = graph{1, 2, 3};
			g.insert_edge(1, 2, 10);
			g.insert_edge(2, 3, 20);
			auto it = g.begin();
			auto edge = *it;
			CHECK(edge.from == 1);
			CHECK(edge.to == 2);
			CHECK(edge.weight == 10);
			++it;
			edge = *it;
			CHECK(edge.from == 2);
			CHECK(edge.to == 3);
			CHECK(edge.weight == 20);
			++it;
			CHECK(it == g.end());
		}
		SECTION("++ and --") {
			auto g = graph{1, 2, 3};
			g.insert_edge(1, 2, 10);
			g.insert_edge(2, 3, 20);
			auto it = g.begin();
			++it;
			++it;
			--it;
			--it;
			auto edge = *it;
			CHECK(edge.from == 1);
			CHECK(edge.to == 2);
			CHECK(edge.weight == 10);
		}
		SECTION("Comparison") {
			auto g = graph{1, 2, 3};
			g.insert_edge(1, 2, 10);
			g.insert_edge(2, 3, 20);
			auto it1 = g.begin();
			auto it2 = g.begin();
			CHECK(it1 == it2);
			++it1;
			CHECK(it1 != it2);
			auto it3 = g.end();
			auto it4 = g.end();
			CHECK(it3 == it4);
		}
	}
	SECTION("Accessors") {
		using graph = gdwg::graph<int, int>;
		SECTION("Graph edges()") {
			auto g = graph{};
			g.insert_node(1);
			g.insert_node(2);
			g.insert_node(3);
			g.insert_edge(1, 2, 5);
			g.insert_edge(1, 2, 10);
			g.insert_edge(1, 2);
			g.insert_edge(1, 3, 15);

			SECTION("Edges exists") {
				auto edges_1_2 = g.edges(1, 2);
				CHECK(edges_1_2.size() == 3);
				CHECK(edges_1_2[0]->get_weight() == std::nullopt);
				CHECK(edges_1_2[1]->get_weight() == 5);
				CHECK(edges_1_2[2]->get_weight() == 10);

				auto edges_1_3 = g.edges(1, 3);
				CHECK(edges_1_3.size() == 1);
				CHECK(edges_1_3[0]->get_weight() == 15);
			}
			SECTION("Edges not exists") {
				auto edges_2_3 = g.edges(2, 3);
				CHECK(edges_2_3.empty());
			}
			SECTION("Nodes not exists") {
				CHECK_THROWS_WITH(g.edges(4, 2),
				                  "Cannot call gdwg::graph<N, E>::edges if src or dst node don't exist in the "
				                  "graph");
				CHECK_THROWS_WITH(g.edges(1, 4),
				                  "Cannot call gdwg::graph<N, E>::edges if src or dst node don't exist in the "
				                  "graph");
			}
		}
		SECTION("is_node") {
			auto g = graph{};
			SECTION("exists") {
				g.insert_node(1);
				CHECK(g.is_node(1));
				CHECK(not g.is_node(2));
			}
		}
		SECTION("empty") {
			auto g = graph{};
			SECTION("empty") {
				CHECK(g.empty());
			}
			SECTION("not empty") {
				g.insert_node(1);
				CHECK(not g.empty());
			}
		}
		SECTION("is_connected") {
			auto g = graph{};
			g.insert_node(1);
			g.insert_node(2);
			g.insert_edge(1, 2, 1);
			SECTION("connected") {
				CHECK(g.is_connected(1, 2));
			}
			SECTION("not connected") {
				CHECK(not g.is_connected(2, 1));
			}
			SECTION("dne") {
				CHECK_THROWS_WITH(g.is_connected(1, 3),
				                  "Cannot call gdwg::graph<N, E>::is_connected if src or dst node don't exist in the "
				                  "graph");
			}
		}
		SECTION("Insert nodes") {
			auto g = graph{};
			SECTION("empty") {
				auto nodes = g.nodes();
				CHECK(nodes.empty());
			}
			SECTION("not empty") {
				g.insert_node(2);
				g.insert_node(1);
				auto nodes = g.nodes();
				auto expected_nodes = std::vector<int>{1, 2};
				CHECK(nodes == expected_nodes);
			}
		}
		SECTION("is_connected") {
			auto g = graph{};
			g.insert_node(1);
			g.insert_node(2);
			g.insert_node(3);
			g.insert_edge(1, 2, 10);
			SECTION("connected") {
				CHECK(g.is_connected(1, 2));
			}
			SECTION("not connected") {
				CHECK(not g.is_connected(2, 3));
			}
			SECTION("exception") {
				CHECK_THROWS_WITH(g.is_connected(4, 2),
				                  "Cannot call gdwg::graph<N, E>::is_connected if src or dst node don't exist in the "
				                  "graph");
				CHECK_THROWS_WITH(g.is_connected(1, 4),
				                  "Cannot call gdwg::graph<N, E>::is_connected if src or dst node don't exist in the "
				                  "graph");
			}
		}
		SECTION("nodes") {
			auto g = graph{};
			g.insert_node(1);
			g.insert_node(2);
			g.insert_node(3);
			SECTION("nonept") {
				auto nodes = g.nodes();
				auto expected_nodes = std::vector<int>{1, 2, 3};
				CHECK(nodes == expected_nodes);
			}
			SECTION("ept") {
				auto ept = graph{};
				auto nodes = ept.nodes();
				auto expected_nodes = std::vector<int>{};
				CHECK(nodes == expected_nodes);
			}
		}
		SECTION("Find edges") {
			auto g = graph{};
			g.insert_node(1);
			g.insert_node(2);
			g.insert_node(3);
			g.insert_edge(1, 2, 5);
			g.insert_edge(1, 2, 10);
			g.insert_edge(1, 2);
			g.insert_edge(1, 3, 15);

			SECTION("weighted edge exists") {
				auto it = g.find(1, 2, 5);
				CHECK(it != g.end());
				auto edge = *it;
				CHECK(edge.from == 1);
				CHECK(edge.to == 2);
				CHECK(edge.weight == 5);
			}

			SECTION("unweighted edge exists") {
				auto it = g.find(1, 2, std::nullopt);
				CHECK(it != g.end());
				auto edge = *it;
				CHECK(edge.from == 1);
				CHECK(edge.to == 2);
				CHECK(edge.weight == std::nullopt);
			}

			SECTION("edge not exists") {
				auto it = g.find(2, 3);
				CHECK(it == g.end());
			}

			SECTION("node not exists") {
				auto it = g.find(4, 2);
				CHECK(it == g.end());
			}
		}
		SECTION("Connections") {
			auto g = graph{};
			g.insert_node(3);
			g.insert_node(1);
			g.insert_node(2);
			g.insert_node(4);
			g.insert_edge(1, 4, 10);
			g.insert_edge(1, 2, 10);
			g.insert_edge(1, 3, 20);
			g.insert_edge(1, 2);

			SECTION("outgoing") {
				auto connections = g.connections(1);
				const auto expected_connections = std::vector<int>{2, 3, 4};
				CHECK(connections == expected_connections);
			}

			SECTION("incoming") {
				auto connections = g.connections(4);
				const auto expected_connections = std::vector<int>{};
				CHECK(connections == expected_connections);
			}

			SECTION("dne") {
				CHECK_THROWS_WITH(g.connections(5),
				                  "Cannot call gdwg::graph<N, E>::connections if src doesn't exist in the graph");
			}
		}
	}
	SECTION("Extractor") {
		SECTION("Example test") {
			using graph = gdwg::graph<int, int>;
			const auto v = std::vector<std::tuple<int, int, std::optional<int>>>{
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
			const auto expected_output = std::string_view(R"(
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
		SECTION("Empty graph test") {
			using graph = gdwg::graph<int, std::string>;
			auto g = graph{};
			auto out = std::ostringstream{};
			out << g;
			CHECK(out.str() == "");
		}
	}
	SECTION("Comparisons") {
		using graph = gdwg::graph<int, int>;
		SECTION("Empty") {
			auto g1 = graph{};
			auto g2 = graph{};
			CHECK(g1 == g2);
		}
		SECTION("nonempty equal") {
			auto g1 = graph{1, 2};
			g1.insert_edge(1, 2);
			auto g2 = graph{1, 2};
			g2.insert_edge(1, 2);
			CHECK(g1 == g2);
		}
		SECTION("nonequal") {
			auto g1 = graph{1, 2};
			g1.insert_edge(1, 2, 3);
			auto g2 = graph{1, 2, 3};
			g2.insert_edge(2, 3);
			CHECK(g1 != g2);
		}
	}
}
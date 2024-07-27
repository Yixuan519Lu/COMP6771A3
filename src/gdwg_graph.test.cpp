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

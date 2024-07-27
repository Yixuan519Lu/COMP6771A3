#include "gdwg_graph.h"

#include <catch2/catch.hpp>

TEST_CASE("gdwg::graph") {
	SECTION("Constructors") {
		SECTION("Default constructor") {
			auto g = gdwg::graph<int, std::string>{};
			CHECK(g.empty());
		}
	}
}

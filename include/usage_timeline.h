#pragma once
#include <string>
#include <map>
#include <vector>
#include <utility>
#include <array>
#include <cstdint>

namespace spirtsaway::cpu_web_monitor
{
	using timeline_data = std::vector<std::pair<std::uint64_t, std::uint32_t>>;
	class usage_data_collector
	{
		constexpr static std::size_t level = 5;
		const static std::array<std::size_t, level> gap_level;
		std::array<timeline_data, level> timed_data;
	public:
		void add_data(const timeline_data& new_data, std::size_t cur_level);
		timeline_data query_data(std::uint64_t min_ts, std::uint64_t max_ts);
	};
}
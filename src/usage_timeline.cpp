#include "usage_timeline.h"
namespace spirtsaway::cpu_web_monitor
{
	const std::array<std::size_t, usage_data_collector::level> usage_data_collector::gap_level = { 5, 30, 120, 300, 600 };
	void usage_data_collector::add_data(const timeline_data& new_data, std::size_t cur_level)
	{
		if (new_data.empty())
		{
			return;
		}
		if (cur_level >= level)
		{
			return;
		}
		std::size_t min_ts = 0;
		if (!timed_data[cur_level].empty())
		{
			min_ts = timed_data[cur_level].back().first;
		}
		else
		{
			min_ts = new_data.front().first - 1;
		}
		std::size_t pre_idx = timed_data[cur_level].size();
		std::size_t max_ts = new_data.back().first;
		timed_data[cur_level].insert(timed_data[cur_level].end(), new_data.begin(), new_data.end());
		std::size_t begin_gap_idx = min_ts / gap_level[cur_level];
		std::size_t end_gap_idx = max_ts / gap_level[cur_level];
		if (begin_gap_idx ==  end_gap_idx)
		{
			return;
		}

		while (pre_idx > 0 && timed_data[cur_level][pre_idx - 1].first / gap_level[cur_level] == begin_gap_idx)
		{
			pre_idx--;
		}
		timeline_data next_level_data;
		std::size_t count = 0;
		std::size_t total = 0;
		while (pre_idx < timed_data[cur_level].size())
		{
			if (timed_data[cur_level][pre_idx].first / gap_level[cur_level] != begin_gap_idx)
			{
				next_level_data.push_back(std::make_pair(begin_gap_idx * gap_level[cur_level], total / count));
				begin_gap_idx = timed_data[cur_level][pre_idx].first / gap_level[cur_level];
				count = 0;
				total = 0;
			}
			count += 1;
			total += timed_data[cur_level][pre_idx].second;
		}
		if (!next_level_data.empty())
		{
			add_data(next_level_data, cur_level++);
		}


	}
}

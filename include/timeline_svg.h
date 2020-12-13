#pragma once
#include "usage_timeline.h"
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <ostream>

namespace spiritsaway::cpu_web_monitor
{
	enum class color_schema
	{
		red,
		green,
		blue,
		yellow,
		purple,
		aqua,
		orange,
		hot,
		mem,
		io,
		palette,
	};
	struct color
	{
		std::uint8_t r = 0;
		std::uint8_t g = 0;
		std::uint8_t b = 0;
		color(color_schema schema, std::size_t hash);
		color(std::uint8_t r = 0, std::uint8_t g = 0, std::uint8_t b = 0);
		std::string to_string() const;
	};
	std::string html_encode(const std::string& data);
	class cpu_usage_svg
	{
	public:
		bool add_data(const std::string& name, const std::vector<std::pair<std::uint64_t, std::uint32_t>>& usage_data);
		cpu_usage_svg()
		{

		}
	private:
		std::map<std::string, std::vector<std::pair<std::uint64_t, std::uint32_t>>> total_usage_data;
		void prepare();
		void make_header(std::ostream& output);
		void make_time_axis(std::ostream& output);
		void make_usage_axis(std::ostream& output);
		void make_process_names(std::ostream& output);
		void make_cpu_usage_line(std::ostream& output);
	public:
		std::uint32_t min_width = 1200;
		std::uint32_t frame_height = 16;
		std::string fonttype = "Verdana";
		std::uint32_t font_size = 12;
		double font_width = 0.59;
		std::string title;
		int ypad_after_title = 50;
		int ypad_before_title = 50;
		int x_frame_pad = 100;
		int names_x_pad = 200;
		int names_y_pad = 50;
		int usage_height = 500;
		int label_x_size = 60;
		int label_x_bar_size = 20;
		int name_bar_size = 20;
		int label_y_bar_size = 10;
		int usage_unit = 10;

		color_schema color_provider = color_schema::red;

		std::string render_graph();
	private:
		int title_x;
		int title_y;
		int width;
		int height;
		int max_cpu_usage = 100;
		std::uint64_t min_ts = std::numeric_limits<std::uint64_t>::max();
		std::uint64_t max_ts = 0;
		int ts_label_gap = 0;
		double widthpertime = 0;
		double widthperusage = 0;
		int name_lines = 1;
		int name_per_line = 0;
	};
}
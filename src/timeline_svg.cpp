#include "timeline_svg.h"
#include "fmt/format.h"
#include <ctime>
#include <chrono>
#include <sstream>
namespace
{
	std::string format_timepoint(std::uint64_t seconds_since_epoch, bool with_date = true)
	{
		auto epoch_begin = std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>();
		auto cur_timepoint = epoch_begin + std::chrono::seconds(seconds_since_epoch);
		auto cur_time_t = std::chrono::system_clock::to_time_t(cur_timepoint);

		struct tm* timeinfo;
		char buffer[80];

		timeinfo = localtime(&cur_time_t);
		if (with_date)
		{
			strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S ", timeinfo);

		}
		else
		{
			strftime(buffer, sizeof(buffer), "%H:%M:%S ", timeinfo);
		}
		return std::string(buffer);
	}
}
namespace spiritsaway::cpu_web_monitor
{
	std::string html_encode(const std::string& data)
	{
		std::string buffer;
		buffer.reserve(data.size());
		for (size_t pos = 0; pos != data.size(); ++pos)
		{
			switch (data[pos])
			{
			case '&':  buffer.append("&amp;");       break;
			case '\"': buffer.append("&quot;");      break;
			case '\'': buffer.append("&apos;");      break;
			case '<':  buffer.append("&lt;");        break;
			case '>':  buffer.append("&gt;");        break;
			default:   buffer.append(&data[pos], 1); break;
			}
		}
		return buffer;
	}
	color::color(color_schema schema, std::size_t hash)
	{
		std::uint8_t hash_1 = hash & ((1 << 8) - 1);
		std::uint8_t hash_2 = (hash >> 8) & ((1 << 8) - 1);
		std::uint8_t hash_3 = (hash >> 16) & ((1 << 8) - 1);
		switch (schema)
		{
		case color_schema::hot:
		{
			r = 205 + 50 * hash_3;
			g = 230 * hash_1;
			b = 55 * hash_2;
			break;
		}
		case color_schema::mem:
		{
			r = 0;
			g = 190 + 50 * hash_2;
			b = 210 * hash_1;
			break;
		}
		case color_schema::io:
		{
			r = 80 + 60 * hash_1;
			g = r;
			b = 190 + 55 * hash_2;
			break;
		}
		case color_schema::red:
		{
			r = 200 + 55 * hash_1;
			g = 50 + 80 * hash_1;
			b = g;
			break;
		}
		case color_schema::green:
		{
			r = 50 + 60 * hash_1;
			g = 200 + 55 * hash_1;
			b = r;
			break;
		}
		case color_schema::blue:
		{
			b = 205 + 50 * hash_1;
			r = 80 + 60 * hash_1;
			g = r;
			break;
		}
		case color_schema::yellow:
		{
			r = 175 + 55 * hash_1;
			g = r;
			b = 50 + 20 * hash_1;
			break;
		}
		case color_schema::purple:
		{
			r = 190 + 65 * hash_1;
			g = 80 + 60 * hash_1;
			b = r;
			break;
		}
		case color_schema::aqua:
		{
			r = 50 + 60 * hash_1;
			g = 165 + 55 * hash_1;
			b = g;
			break;
		}
		case color_schema::orange:
		{
			r = 190 + 65 * hash_1;
			g = 90 + 65 * hash_1;
			b = 0;
			break;
		}

		}
	}
	color::color(std::uint8_t in_r, std::uint8_t in_g, std::uint8_t in_b)
		: r(in_r)
		, g(in_g)
		, b(in_b)
	{

	}
	std::string color::to_string()const
	{
		return "rgb(" + std::to_string(r) + "," + std::to_string(g) + "," + std::to_string(b) + ")";
	}
	bool cpu_usage_svg::add_data(const std::string& name, const std::vector<std::pair<std::uint64_t, std::uint32_t>>& usage_data)
	{
		if (total_usage_data.find(name) != total_usage_data.end())
		{
			return false;
		}
		total_usage_data[name] = usage_data;
		return true;
	}
	void cpu_usage_svg::prepare()
	{
		for (const auto& one_item : total_usage_data)
		{
			for (const auto& one_usage : one_item.second)
			{
				if (one_usage.second > max_cpu_usage)
				{
					max_cpu_usage = one_usage.second;
				}
			}
			if (!one_item.second.empty())
			{
				min_ts = min_ts > one_item.second.front().first ? one_item.second.front().first : min_ts;
				max_ts = max_ts < one_item.second.back().first ? one_item.second.back().first : max_ts;
			}
		}
		max_cpu_usage = ((max_cpu_usage + 99) / 100) * 100;
		widthperusage = usage_height * 1.0 / max_cpu_usage;
		std::uint64_t ts_diff = max_ts - min_ts;
		if (ts_diff < 5 * 60)
		{
			ts_label_gap = 5;
		}
		else if (ts_diff < 60 * 60)
		{
			ts_label_gap = 60;
		}
		else if (ts_diff < 24 * 60 * 60)
		{
			ts_label_gap = 60 * 60;
		}
		else
		{
			ts_label_gap = 24 * 60 * 60;
		}

		min_ts = (min_ts / ts_label_gap) * ts_label_gap;
		max_ts = ((max_ts / ts_label_gap) + 1) * ts_label_gap;
		int total_labels = (max_ts - min_ts) / ts_label_gap;
		width = total_labels * label_x_size + 2 * x_frame_pad;
		if (width < min_width)
		{
			width = min_width;
		}
		widthpertime = (width - 2 * x_frame_pad) * 1.0 / (max_ts - min_ts);
		name_per_line = (width - 2 * x_frame_pad) / names_x_pad;
		auto name_lines = (total_usage_data.size() + name_per_line - 1) / name_per_line;
		height = ypad_before_title + font_size * 2 + ypad_after_title + usage_height + label_y_bar_size + font_size + (name_lines + 1) * (font_size + names_y_pad) + 2 * names_y_pad;
		title = "cpu usage graph from " + format_timepoint(min_ts) + " to " + format_timepoint(max_ts);
	}
	void cpu_usage_svg::make_header(std::ostream& output)
	{
		int begin_y = 0;
		title_x = x_frame_pad * 2;
		title_y = ypad_before_title + 2 * font_size;
		begin_y = title_y + ypad_after_title;

		output<<R"(<?xml version="1.0" standalone="no"?>)"<<std::endl;
		output << R"(<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.1//EN" "http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd">)" << std::endl;
		std::string temp;
		temp = fmt::format("<svg version=\"1.1\" width=\"{0}\" height=\"{1}\" viewBox=\"0 0 {0} {1}\" onload=\"Init(evt)\" ", width, height);
		output << temp;
		output << R"(xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink">)" << std::endl;

		std::string svg_scripts = R"( <script type="text/ecmascript"><![CDATA[
      var SVGDocument = null;
      var SVGRoot = null;

      function Init(evt)
      {
         SVGDocument = evt.target.ownerDocument;
         SVGRoot = SVGDocument.documentElement;

      }
	  function ToggleOpacity(targetId)
      {
         var newTarget = SVGDocument.getElementById(targetId);;

         var newValue = newTarget.getAttributeNS(null, 'opacity')

         if ('0' != newValue)
         {
            newValue = '0';
         }
         else
         {
            newValue = '1';
         }
         newTarget.setAttributeNS(null, 'opacity', newValue);
      }
      function ToggleGroupOpacity(evt)
      {
         var cur_target = evt.target;
         var group_name = cur_target.getAttributeNS(null, "group_name");
         if(!group_name)
         {
            return;
         }
         var usage_ele_id = "usage_for_" +group_name;
         ToggleOpacity(usage_ele_id);
      }
]]></script>
)";
		output << svg_scripts << std::endl;

		temp = fmt::format("<text x=\"{}\" y=\"{}\" font-family=\"{}\" font-size=\"{}\" fill=\"{}\">{}</text>\n", x_frame_pad, ypad_before_title + 2 * font_size, fonttype, 2 * font_size, "red", title);
		output << temp;

		std::string frame_stroke_color = "red";
		temp = fmt::format("<polyline points=\"{0},{1} {2},{3} {4},{5} {6},{7} {0},{1}\" stroke=\"{8}\" stroke-width=\"{9}\" fill=\"none\"/>\n", x_frame_pad, begin_y, x_frame_pad, begin_y + usage_height, width - x_frame_pad, begin_y + usage_height, width - x_frame_pad, begin_y, frame_stroke_color, 2);
		output << temp;


	}
	void cpu_usage_svg::make_usage_axis(std::ostream& output)
	{
		auto axis_y_begin = ypad_before_title + ypad_after_title + 2 * font_size;
		auto axis_y_end = axis_y_begin + usage_height;
		float opacity = 0.1;
		auto axis_x_begin = x_frame_pad;
		auto axis_x_end = width - x_frame_pad;
		for (int i = 1; i < max_cpu_usage / usage_unit; i++)
		{
			output << "<g>\n";
			color cur_color(color_provider, i);
			auto text_string = fmt::format("<text x=\"{0}\" y=\"{1}\" font-family=\"{2}\" font-size=\"{3}\" fill=\"{4}\">{5}</text>\n", axis_x_begin - 3 * font_size, int(axis_y_end - i * 10 * widthperusage), fonttype, font_size, cur_color.to_string(), std::to_string(i * 10));
			output << text_string;
			auto line_string = fmt::format("<line x1=\"{}\" x2=\"{}\" y1=\"{}\" y2=\"{}\" stroke=\"{}\" stroke-width=\"{}\" opacity=\"{}\"/>\n", axis_x_begin, axis_x_end, int(axis_y_end - i * 10 * widthperusage), int(axis_y_end - i * 10 * widthperusage), cur_color.to_string(), 2, opacity);
			output << line_string;
			output << "</g>\n";
		}
	}
	void cpu_usage_svg::make_time_axis(std::ostream& output)
	{
		auto axis_y_begin = ypad_before_title + ypad_after_title + 2 * font_size + usage_height;
		auto axis_x_begin = x_frame_pad;
		std::string label_color = "black";
		std::string temp;
		for (std::size_t i = 1; i < (max_ts - min_ts) / ts_label_gap; i++)
		{
			auto cur_ts = min_ts + ts_label_gap * i;

			output << "<g>\n";
			auto current_begin = int(axis_x_begin + i * ts_label_gap * widthpertime);

			temp = fmt::format("<line x1=\"{}\" x2=\"{}\" y1=\"{}\" y2=\"{}\" stroke=\"{}\" stroke-width=\"{}\"/>\n", current_begin, current_begin, axis_y_begin, axis_y_begin + label_y_bar_size, label_color, 2);
			output << temp;
			std::string ts_string = format_timepoint(cur_ts,false);
			temp = fmt::format("<text x=\"{}\" y=\"{}\" font-family=\"{}\" font-size=\"{}\" fill=\"{}\">{}</text>\n", current_begin - ts_string.size() / 4 * font_size, axis_y_begin + label_y_bar_size + font_size, fonttype, font_size, label_color, ts_string);
			output << temp;
			output << "</g>\n";
		}
	}
	void cpu_usage_svg::make_process_names(std::ostream& output)
	{
		std::size_t i = 0;
		std::string temp;
		std::string label_color = "black";
		for (const auto& one_item : total_usage_data)
		{
			auto cur_name = html_encode(one_item.first);
			auto cur_color = color(color_provider, i);
			auto cur_line = i / name_per_line;
			auto cur_begin_x = x_frame_pad + (i % name_per_line)* names_x_pad;
			auto cur_begin_y = ypad_before_title + ypad_after_title + 2 * font_size + usage_height + names_y_pad + cur_line*(font_size + names_y_pad);
			temp = fmt::format("<line x1=\"{}\" x2=\"{}\" y1=\"{}\" y2=\"{}\" stroke=\"{}\" stroke-width=\"{}\" onclick=\"ToggleGroupOpacity(evt)\" group_name=\"{}\"/>\n", cur_begin_x, cur_begin_x + name_bar_size, cur_begin_y - font_size/2, cur_begin_y -font_size/2, cur_color.to_string(), font_size, cur_name);
			output << temp;
			temp = fmt::format("<text x=\"{}\" y=\"{}\" font-family=\"{}\" font-size=\"{}\" fill=\"{}\">{}</text>\n", cur_begin_x + font_size + name_bar_size, cur_begin_y, fonttype, font_size, cur_color.to_string(), cur_name);
			output << temp;
			i++;

		}
	}
	void cpu_usage_svg::make_cpu_usage_line(std::ostream& output)
	{
		std::size_t i = 0;
		std::size_t base_y = ypad_before_title + ypad_after_title + 2 * font_size + usage_height;
		std::string temp;
		for (const auto& one_item : total_usage_data)
		{
			output << "<g id=\"" <<"usage_for_" + html_encode(one_item.first) <<"\">\n";
			auto cur_color = color(color_provider, i);
			for (std::size_t j = 1; j < one_item.second.size(); j++)
			{
				auto cur_ts = one_item.second[j].first;
				auto cur_value = one_item.second[j].second;
				auto pre_ts = one_item.second[j - 1].first;
				auto pre_value = one_item.second[j - 1].second;
				std::size_t pre_point_x = x_frame_pad + (pre_ts - min_ts) * widthpertime;
				std::size_t cur_point_x = x_frame_pad + (cur_ts - min_ts) * widthpertime;
				std::size_t pre_point_y = base_y - pre_value * widthperusage;
				std::size_t cur_point_y = base_y - cur_value * widthperusage;
				temp = fmt::format("<line x1=\"{}\" x2=\"{}\" y1=\"{}\" y2=\"{}\" stroke=\"{}\" stroke-width=\"{}\"/>\n", pre_point_x, cur_point_x, pre_point_y, cur_point_y, cur_color.to_string(), 2);
				output << temp;
			}
			i++;
			output << "</g>\n";

		}
	}
	std::string cpu_usage_svg::render_graph()
	{
		std::ostringstream output_buffer;
		prepare();
		make_header(output_buffer);
		make_time_axis(output_buffer);
		make_usage_axis(output_buffer);
		make_process_names(output_buffer);
		make_cpu_usage_line(output_buffer);
		output_buffer << "</svg>\n";
		return output_buffer.str();
	}
}
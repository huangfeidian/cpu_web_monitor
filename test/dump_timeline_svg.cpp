#include "timeline_svg.h"
#include <nlohmann/json.hpp>
#include <streambuf>
#include <fstream>
#include <sstream>

using namespace std;
using json = nlohmann::json;
using namespace spiritsaway::cpu_web_monitor;

std::map<std::string, std::vector<std::pair<std::uint64_t, std::uint32_t>>> read_from_json_file(const std::string& file_name)
{
	std::ifstream input(file_name);
	std::stringstream buffer;
	buffer << input.rdbuf();
	auto cur_str = buffer.str();
	json file_obj;
	if (!json::accept(cur_str))
	{
		return {};
	}
	file_obj = json::parse(cur_str);
	std::map<std::string, std::vector<std::pair<std::uint64_t, std::uint32_t>>> result;
	if (!file_obj.is_object())
	{
		return result;
	}
	for (auto one_item : file_obj.items())
	{
		if (!one_item.value().is_array())
		{
			return result;
		}
		std::vector<std::pair<std::uint64_t, std::uint32_t>> temp_array;
		for (auto array_item : one_item.value().get<json::array_t>())
		{
			if (!array_item.is_array())
			{
				return result;
			}
			if (array_item.size() != 2)
			{
				return result;
			}
			if (!array_item[0].is_number_unsigned() || !array_item[1].is_number_unsigned())
			{
				return result;
			}
			temp_array.push_back(std::make_pair(array_item[0].get<std::uint64_t>(), array_item[1].get<std::uint32_t>()));
		}
		result[one_item.key()] = temp_array;
	}
	return result;
}

int main()
{
	std::string file_name = "../data/one_item.json";
	auto file_content = read_from_json_file(file_name);
	cpu_usage_svg cur_svg;
	for (auto one_item : file_content)
	{
		cur_svg.add_data(one_item.first, one_item.second);

	}
	std::string svg_file_name = "export.svg";
	std::ofstream svg_output(svg_file_name);
	svg_output << cur_svg.render_graph() << std::endl;
	svg_output.close();
	return 1;
}
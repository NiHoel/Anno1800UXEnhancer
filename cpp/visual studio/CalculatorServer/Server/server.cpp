#include "server.hpp"

#include "version.hpp"

using namespace std;
using namespace web;
using namespace utility;
using namespace http;
using namespace web::http::experimental::listener;

using namespace reader;



server::server(bool verbose)
	:
	recog(verbose),
	stats(recog)
{
}

server::server(bool verbose, std::wstring window_regex, utility::string_t url) :
	recog(verbose, image_recognition::to_string(window_regex)),
	stats(recog),
	m_listener(url)
{
	m_listener.support(methods::GET, std::bind(&server::handle_get, this, std::placeholders::_1));
}


void server::repack_statistics(web::json::value& result, bool optimal_productivity)
{
	auto values = stats.get_all();

	for (const auto& asset : values) {
		web::json::value entry;

		for (const auto& prop : asset.second)
		{
			if (prop.first != statistics_screen::KEY_PRODUCTIVITY || !optimal_productivity)
				entry[recog.to_wstring(prop.first)] = web::json::value(prop.second);
		}

		result[std::to_wstring(asset.first)] = entry;
	}

	if (optimal_productivity)
	{
		auto pair = stats.get_optimal_productivity();

		if (pair.first)
			if (result.has_field(std::to_wstring(pair.first)))
				result.at(std::to_wstring(pair.first))[recog.to_wstring(statistics_screen::KEY_PRODUCTIVITY)] = pair.second;
			else {
				web::json::value entry;
				entry[std::wstring(recog.to_wstring(statistics_screen::KEY_PRODUCTIVITY))] = web::json::value(pair.second);
				result[std::to_wstring(pair.first)] = entry;
			}
	}
}


void server::read_islands(web::json::value& result)
{

	auto islands = stats.get_current_islands();
	std::vector<web::json::value> list;

	for (const auto& i : islands) {
		web::json::value entry;
		entry[L"name"] = web::json::value(image_recognition::to_wstring(i.first));
		entry[L"session"] = web::json::value(i.second);
		try {
			entry[L"region"] = web::json::value(recog.session_to_region[i.second]);
		}
		catch (const std::exception& e)
		{}
		list.push_back(entry);
	}
	result[L"islands"] = web::json::value::array(list);
}

void server::handle_get(http_request request)
{
	if (mutex_.try_lock()) {
		try {
			const auto& query_params = request.absolute_uri().split_query(request.absolute_uri().query());

			ucout << "request received: " << request.absolute_uri().query() << endl;


			std::string language("english");
			bool optimal_productivity = false;
			if (!query_params.empty())
			{
				if (query_params.find(L"lang") != query_params.end())
				{
					std::string lang = image_recognition::to_string(query_params.find(L"lang")->second);
					if (stats.has_language(lang))
						language = lang;
				}

				if (query_params.find(L"optimalProductivity") != query_params.end())
				{
					std::wstring string = query_params.find(L"optimalProductivity")->second;
					optimal_productivity = string.compare(L"true") == 0 || string.compare(L"1") == 0;
				}

			}



			cv::Rect2i window(recog.find_anno());
			if (!window.area())
			{
				std::cout << "Couldn't take screenshot" << std::endl;
				web::http::http_response response(status_codes::NoContent);
				response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
				const auto t = request.reply(response);
				mutex_.unlock();
				return;
			}

			cv::Mat screenshot(recog.take_screenshot(window));
			stats.update(language, screenshot);

			web::json::value json_message;

			json_message[U("version")] = web::json::value(std::wstring(version::VERSION_TAG.begin(), version::VERSION_TAG.end()));
			std::string island_name = stats.get_selected_island();

			if (!island_name.empty())
			{
				json_message[U("islandName")] = web::json::value(image_recognition::to_wstring(island_name));

				repack_statistics(json_message, optimal_productivity);
				read_islands(json_message);
			}


			web::http::http_response response(status_codes::OK);
			response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
			response.set_body(json_message);
			const auto t = request.reply(response);
		}
		catch (...)
		{
			web::http::http_response response(status_codes::InternalError);
			response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
			const auto t = request.reply(response);
		}

		mutex_.unlock();
	}
	else {
		web::http::http_response response(status_codes::TooManyRequests);
		response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
		const auto t = request.reply(response);
	}
};
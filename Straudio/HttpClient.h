#pragma once

#include "yhirose/httplib.h"
#include <iostream>
#include <utility>
#include "nlohmann/json.hpp"

class HttpClient {

public:
	static std::pair<bool, std::string> login(std::string email, std::string password) {
		httplib::Client client("localhost", 3000);
		
		httplib::Params params {
			{"email", email},
			{"password", password}
		};
		
		auto res = client.Post("/api/login", params);
		if (res && res->status == 200){
			auto j3 = nlohmann::json::parse(res->body);
			std::pair<bool, std::string> successAndText(true, j3["auth"]);
			return successAndText;
		} else if (res && (res->status == 401 || res->status == 422)) {
			auto j3 = nlohmann::json::parse(res->body);
			std::pair<bool, std::string> successAndText(false, j3["errors"][0]["msg"]);
			return successAndText;
		} else {
			std::cout << "Looks like the server is down." << std::endl;
			std::pair<bool, std::string> successAndText(false, "Uh oh. Looks like the servers are down.");
			return successAndText;
		}
	}

	static std::pair<bool, std::string> request_server(std::string token) {
		httplib::Client client("localhost", 3000);
		
		httplib::Params params {
			{"authToken", token}
		};
		
		auto res = client.Post("/request", params);
		if (res && res->status == 200){
			auto j3 = nlohmann::json::parse(res->body);
			std::string host = j3["host"];
			std::string port = j3["port"];
			std::pair<bool, std::string> successAndText(true, host.append(":").append(j3["port"]));
			return successAndText;
		} else if (res && res->status == 401) {
			auto j3 = nlohmann::json::parse(res->body);
			std::pair<bool, std::string> successAndText(false, j3["errors"][0]["msg"]);
			return successAndText;
		} else {
			std::cout << "Looks like the server is down." << std::endl;
			std::pair<bool, std::string> successAndText(false, "Uh oh. Looks like the servers are down.");
			return successAndText;
		}
	}
};


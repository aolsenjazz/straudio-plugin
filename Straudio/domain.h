#pragma once

#include "nlohmann/json.hpp"

class Participant {

public:
	
	std::string pId;
	std::string displayName;
	
	Participant(nlohmann::json j) {
		pId = j["id"].get<std::string>();
		displayName = j["displayName"].get<std::string>();
	}
	
	std::string toString() {
		std::ostringstream os;
		os << "Participant:\n\tId: " << pId << "\n\tDisplay Name: " << displayName << "std::endl";
		return os.str();
	}
};

class Host {

public:
	
	std::string hId;
	std::string displayName;
	
	Host(nlohmann::json j) {
		hId = j["id"].get<std::string>();
		displayName = j["displayName"].get<std::string>();
	}
	
	std::string toString() {
		std::ostringstream os;
		os << "Host:\n\tId: " << hId << "\n\tDisplay Name: " << displayName << std::endl;
		return os.str();
	}
};

class Room {
public:
	
	std::string rId;
	Host* host;
	std::vector<Participant> participants;
	
	Room(nlohmann::json j) {
		try { host = new Host(j["host"]);}
		catch (int e) { Loggy::info("Failed to parse room host"); }
		
		rId = j["id"].get<std::string>();
		
		
	}
	
	void from_json(const nlohmann::json& j, Room& r) {
//		rId = j["id"].get<std::string>();

//		const nlohmann::json& pj = j.at("participants");
//		r.participants.resize(pj.size());
//		std::copy(pj.begin(), pj.end(), r.participants.begin());
	}
	
	std::string toString() {
		std::ostringstream os;
		os << "ROOM\n\tId: " << rId << std::endl;
		
		if (host) os << host->toString();
		else os << "Host: none\n";
		
		for(Participant p: participants) {
			os << p.toString();
		}
		
		return os.str();
	}
};

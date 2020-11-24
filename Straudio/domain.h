#pragma once

#include "nlohmann/json.hpp"

class Participant {

public:
	
	std::string pId;
	
	Participant(nlohmann::json j) {
		pId = j["id"].get<std::string>();
	}
	
	std::string toString() {
		std::ostringstream os;
		os << "Participant:\n\tId: " << pId << std::endl;
		return os.str();
	}
};

class Host {

public:
	
	std::string hId;
	
	Host(nlohmann::json j) {
		hId = j["id"].get<std::string>();
	}
	
	std::string toString() {
		std::ostringstream os;
		os << "Host:\n\tId: " << hId << std::endl;
		return os.str();
	}
};

class Room {
public:
	
	std::string state;
	std::string rId;
	
	std::vector<std::shared_ptr<Participant>> participants;
	std::shared_ptr<Host> host;
	
	Room(nlohmann::json j) {
		try { host = std::make_shared<Host>(j["host"]); }
		catch (int e) {  }
		
		rId = j["id"].get<std::string>();
		state = j["state"].get<std::string>();
	}
	
	Room(){}
	
	static std::shared_ptr<Room> CLOSED_PTR() {
		std::shared_ptr<Room> r = std::make_shared<Room>();
		r->state = "closed";
		return r;
	}
	
	std::string toString() {
		std::ostringstream os;
		os << "ROOM\n\tId: " << rId << std::endl;
		
		if (host != nullptr) os << host->toString();
		else os << "Host: none\n";
		
		for(std::shared_ptr<Participant> p: participants) {
			os << p->toString();
		}
		
		return os.str();
	}
	
	bool isEmpty() {
		return participants.size() == 0;
	}
	
	void addParticipant(Participant p) {
		participants.push_back(std::make_shared<Participant>(p));
	}
	
	void removeParticipant(std::string pId) {
		int idx = -1;
		for (int i = 0; i < participants.size(); ++i) {
			if (participants[i]->pId == pId) {
				idx = i;
			}
		}
		
		if (idx != -1) {
			participants.erase(participants.begin() + idx);
		} else {
			PLOG_INFO << "tried and failed to remove Participant[" << pId << "]" << std::endl;
		}
	}
	
};

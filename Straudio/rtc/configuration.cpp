/**
 * Copyright (c) 2019 Paul-Louis Ageneau
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "configuration.hpp"

#include <regex>

namespace rtc {

IceServer::IceServer(const string &url) {
	// Modified regex from RFC 3986, see https://tools.ietf.org/html/rfc3986#appendix-B
	static const char *rs =
	    R"(^(([^:.@/?#]+):)?(/{0,2}((([^:@]*)(:([^@]*))?)@)?(([^:/?#]*)(:([^/?#]*))?))?([^?#]*)(\?([^#]*))?(#(.*))?)";
	static const std::regex r(rs, std::regex::extended);

	std::smatch m;
	if (!std::regex_match(url, m, r) || m[10].length() == 0)
		throw std::invalid_argument("Invalid ICE server URL: " + url);

	std::vector<std::optional<string>> opt(m.size());
	std::transform(m.begin(), m.end(), opt.begin(), [](const auto &sm) {
		return sm.length() > 0 ? std::make_optional(string(sm)) : nullopt;
	});

	string scheme = opt[2].value_or("stun");
	relayType = RelayType::TurnUdp;
	if (scheme == "stun" || scheme == "STUN")
		type = Type::Stun;
	else if (scheme == "turn" || scheme == "TURN")
		type = Type::Turn;
	else if (scheme == "turns" || scheme == "TURNS") {
		type = Type::Turn;
		relayType = RelayType::TurnTls;
	} else
		throw std::invalid_argument("Unknown ICE server protocol: " + scheme);

	if (auto &query = opt[15]) {
		if (query->find("transport=udp") != string::npos)
			relayType = RelayType::TurnUdp;
		if (query->find("transport=tcp") != string::npos)
			relayType = RelayType::TurnTcp;
		if (query->find("transport=tls") != string::npos)
			relayType = RelayType::TurnTls;
	}

	username = opt[6].value_or("");
	password = opt[8].value_or("");
	hostname = opt[10].value();
	service = opt[12].value_or(relayType == RelayType::TurnTls ? "5349" : "3478");

	while (!hostname.empty() && hostname.front() == '[')
		hostname.erase(hostname.begin());
	while (!hostname.empty() && hostname.back() == ']')
		hostname.pop_back();
}

IceServer::IceServer(string hostname_, uint16_t port_)
    : IceServer(std::move(hostname_), std::to_string(port_)) {}

IceServer::IceServer(string hostname_, string service_)
    : hostname(std::move(hostname_)), service(std::move(service_)), type(Type::Stun) {}

IceServer::IceServer(string hostname_, uint16_t port_, string username_, string password_,
                     RelayType relayType_)
    : IceServer(hostname_, std::to_string(port_), std::move(username_), std::move(password_),
                relayType_) {}

IceServer::IceServer(string hostname_, string service_, string username_, string password_,
                     RelayType relayType_)
    : hostname(std::move(hostname_)), service(std::move(service_)), type(Type::Turn),
      username(std::move(username_)), password(std::move(password_)), relayType(relayType_) {}

ProxyServer::ProxyServer(Type type_, string ip_, uint16_t port_, string username_, string password_)
    : type(type_), ip(ip_), port(port_), username(username_), password(password_) {}

} // namespace rtc

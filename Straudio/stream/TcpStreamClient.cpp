//#include "TcpStreamClient.h"
//#include "iostream"
//#include <boost/interprocess/shared_memory_object.hpp>
//#include "boost/beast/core.hpp"
//#include "boost/beast/websocket.hpp"
//#include "../HttpClient.h"
//
//namespace beast = boost::beast;
//namespace http = beast::http;
//namespace websocket = beast::websocket;
//using tcp = boost::asio::ip::tcp;
//
//const std::string TEST_IP = "127.0.0.1";
//const std::string TEST_PORT = "3004";
//
//const std::string REMOTE_TEST_URL = "52.14.162.232";
//
//const int BYTES_PER_FLOAT = 4;
//
//void printStart(float* data, int length);
//void printEnd(float* data, int length);
//
//boost::asio::io_context ioc;
//
//tcp::resolver resolver{ioc};
//websocket::stream<tcp::socket> ws{ioc};
//
//
//void TcpStreamClient::stream(float* data, int length) {
//	try {
//		auto const results = resolver.resolve(TEST_IP, TEST_PORT);
//
//		boost::asio::connect(ws.next_layer(), results.begin(), results.end());
//		ws.stream::binary(true);
//		ws.set_option(websocket::stream_base::decorator(
//			[](websocket::request_type& req)
//			{
//				req.set(http::field::user_agent,
//					std::string(BOOST_BEAST_VERSION_STRING) +
//						" websocket-client-coro");
//			}));
//		ws.handshake(TEST_IP, "/");
//		
//		ws.write(boost::asio::buffer(data, length * BYTES_PER_FLOAT));
//
//		beast::flat_buffer buffer;
//		ws.read(buffer);
//
//		ws.close(websocket::close_code::normal);
//		
////		std::cout << beast::make_printable(buffer.data()) << std::endl;
//	} catch (boost::system::system_error e) {
//		if (e.code() == boost::asio::error::connection_refused) {
//			// If we try to a use a port that isn't open, or the streaming server is down.
//			std::cout << "refused";
//		} else if (e.code() == boost::asio::error::timed_out) {
//			// Host won't respond.
//			std::cout << "timed out";
//		} else if (e.code() == boost::asio::error::netdb_errors::host_not_found) {
//			// Bad host. This should never happen.
//			std::cout << "bad host";
//		} else {
//			std::cout << e.code();
//		}
//	}
//}
//
//void printStart(float* data, int length) {
//	if (length <= 10) {
//		std::cout << "Tried to print start, but size is < 10.\n";
//		return;
//	}
//	
//	std::cout << "Start: ";
//	for (int i = 0; i < 10; i++) {
//		std::cout << data[i] << " ";
//	}
//	std::cout << std::endl;
//}
//
//void printEnd(float* data, int length) {
//	if (length <= 10) {
//		std::cout << "Tried to print end, but size is < 10.\n";
//		return;
//	}
//	
//	std::cout << "End: ";
//	for (int i = length - 10; i < length; i++) {
//		std::cout << data[i] << " ";
//	}
//	std::cout << std::endl;
//}

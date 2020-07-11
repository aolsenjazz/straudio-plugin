#define BOOST_TEST_MODULE My Test
#include <boost/test/included/unit_test.hpp>
#include "pcm_gen.h"

BOOST_AUTO_TEST_CASE(first_test) {
	auto cb = [](double** data) {
		std::cout << "Hello from callback!";
	};
	
	PcmGenerator::generate(cb, 44100, 2, 2048, 3);
	
	int i = 1;
	BOOST_TEST(i);
	BOOST_TEST(i == 2);
}

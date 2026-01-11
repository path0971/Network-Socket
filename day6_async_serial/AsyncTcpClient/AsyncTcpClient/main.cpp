#include <iostream>
#include <boost/asio.hpp>
#include "AsyncTcpClient.h"

int main() {
	try {
		boost::asio::io_context io_context;
		AsyncTcpClient client(io_context, "127.0.0.1", "7777");
		io_context.run();
	}
	catch (std::exception& e) {
		cerr << "¿¹¿Ü¸Ç: " << e.what() << endl;
	}


	return 0;
}
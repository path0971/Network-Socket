#include <iostream>
#include <boost/asio.hpp>
#include "AsyncTcpServer.h"

int main() {
	try {
		boost::asio::io_context io_context;
		AsyncTcpServer server(io_context, 7777);
		io_context.run();
	}
	catch (std::exception& e) {
		cerr << "¿¹¿Ü¸Ç: " << e.what() << endl;
	}


	return 0;
}
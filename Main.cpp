#include "Header.h"

int main() {
	string content = "Hello, this is a test email!";
	
	MyCurl myCurl;
	myCurl.SendEmail(content);
	while (true) {
		myCurl.ReadEmail();
	}
	
	return 0;
}

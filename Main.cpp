#include "Header.h"

int main() {
	string content = "Hello, this is a test email!";
	vector<string> clientIDs = { "C1", "C2"};
	string id = "A1";
	MyCurl myCurl;
	myCurl.SetID(id);
	for (auto& id : clientIDs) {
		myCurl.AddClientId(id);
	}
	//myCurl.SendEmail(content);
	while (true) {
		myCurl.ReadEmail();
	}
	
	return 0;
}

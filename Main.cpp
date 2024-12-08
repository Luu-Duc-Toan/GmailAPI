#include <iostream>
#include <string>
#include <sstream>
#include <curl/curl.h>
#include <regex>
using namespace std;
string URL = "imaps://imap.gmail.com:993/RemoteUltilities";


// Email data
static const char* email_payload_text =
"Subject: Test Email from libcurl\r\n"
"\r\n"
"Another tests\r\n";

struct upload_status {
	int lines_read;
};



// Callback function to read the email payload
static size_t payload_source(void* ptr, size_t size, size_t nmemb, void* userp) {
	struct upload_status* upload_ctx = (struct upload_status*)userp;
	const char* data;

	if ((size == 0) || (nmemb == 0) || ((size * nmemb) < 1)) {
		return 0;
	}

	data = &email_payload_text[upload_ctx->lines_read];
	if (data) {
		size_t len = strlen(data);
		memcpy(ptr, data, len);
		upload_ctx->lines_read += len;
		return len;
	}

	return 0;
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
	((string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}
void InitSession(string& URL, CURL*& receiver, string& receiveBuffer, string& account, string& password) {
	receiver = curl_easy_init();
	curl_easy_setopt(receiver, CURLOPT_URL, URL.c_str());
	curl_easy_setopt(receiver, CURLOPT_USERNAME, account.c_str());
	curl_easy_setopt(receiver, CURLOPT_PASSWORD, password.c_str());
	curl_easy_setopt(receiver, CURLOPT_USE_SSL, CURLUSESSL_ALL);
	curl_easy_setopt(receiver, CURLOPT_SSL_VERIFYPEER, 1L);
	curl_easy_setopt(receiver, CURLOPT_SSL_VERIFYHOST, 2L);
	curl_easy_setopt(receiver, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
	curl_easy_setopt(receiver, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(receiver, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(receiver, CURLOPT_WRITEDATA, &receiveBuffer);
}
void CleanSession(CURL*& session, string& buffer) {
	buffer.clear();
	curl_easy_cleanup(session);
	session = nullptr;
}

vector<int> GetListUIDUnseen(const string receiveBuffer) {
	stringstream ss(receiveBuffer);
	string word;
	//Skip useless text
	getline(ss, word, ' '); getline(ss, word, ' ');
	vector<int> UIDs;
	while (getline(ss, word, ' ')) {
		UIDs.push_back(stoi(word));
	}
	return UIDs;
}

int main() {
	string clientAccount = "chillseverctt2@gmail.com";
	string clientPassword = "frmc opak fwnu ftos";
	CURL* sender = curl_easy_init();
	CURLcode res = CURLE_OK;
	struct curl_slist* recipients = nullptr;
	struct upload_status upload_ctx = { 0 };

	CURL* receiver = nullptr;
	CURL* reader = curl_easy_init();
	string receiveBuffer;
	string readerBuffer;
	string emailBody;

	/*if (sender) {
		 //Set the SMTP server URL
		curl_easy_setopt(sender, CURLOPT_URL, "smtp://smtp.gmail.com:587");
		// Enable TLS for secure connection
		curl_easy_setopt(sender, CURLOPT_USE_SSL, CURLUSESSL_ALL);

		 //Set the login credentials
		curl_easy_setopt(sender, CURLOPT_USERNAME, "chillclientctt2@gmail.com");
		curl_easy_setopt(sender, CURLOPT_PASSWORD, "kdsr vqde utzq wtic");

		 //Set the sender email address
		curl_easy_setopt(sender, CURLOPT_MAIL_FROM, "chillclientctt2@gmail.com");

		 //Add recipient(s)
		recipients = curl_slist_append(recipients, "chillseverctt2@gmail.com");
		curl_easy_setopt(sender, CURLOPT_MAIL_RCPT, recipients);

		// Specify the payload
		curl_easy_setopt(sender, CURLOPT_READFUNCTION, payload_source);
		curl_easy_setopt(sender, CURLOPT_READDATA, &upload_ctx);
		curl_easy_setopt(sender, CURLOPT_UPLOAD, 1L);

		// Perform the email send
		res = curl_easy_perform(sender);

		// Check for errors
		if (res != CURLE_OK) {
			cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
		}
		else {
			cout << "Email sent successfully!" << endl;
		}
	}*/

	//Search
	InitSession(URL, receiver, receiveBuffer, clientAccount, clientPassword);
	curl_easy_setopt(receiver, CURLOPT_CUSTOMREQUEST, "UID SEARCH UNSEEN");
	//curl_easy_setopt(receiver, CURLOPT_VERBOSE, 1L); // Debug
	res = curl_easy_perform(receiver);
	if (res != CURLE_OK) {
		cout << "Failed to search email" << endl;
		return 0;
	}
	cout << receiveBuffer << endl;
	curl_easy_setopt(receiver, CURLOPT_CUSTOMREQUEST, nullptr);
	vector<int> UIDs = GetListUIDUnseen(receiveBuffer); //for read
	CleanSession(receiver, receiveBuffer);

	//Read
	for (int UID : UIDs) {
		string emailURL = URL + ";UID=" + to_string(UID);
		InitSession(emailURL, receiver, receiveBuffer, clientAccount, clientPassword);
		res = curl_easy_perform(receiver);
		size_t bodyStart = receiveBuffer.find("\r\n\r\n");
		receiveBuffer = receiveBuffer.substr(bodyStart + 4);
		cout << "Email Body " << UID << ": " << receiveBuffer << endl;
		CleanSession(receiver, receiveBuffer);
	}

	// Clean up
	curl_slist_free_all(recipients);
	curl_easy_cleanup(sender);
	curl_easy_cleanup(receiver);
	return 0;
}

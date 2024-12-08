#pragma once
#ifndef GMAILAPI_H
#define GMAILAPI_H
#include "Header.h"

extern string imapsURL;
extern string smtpURL;
struct upload_status {
	int lines_read;
};
extern string email_payload_text;
static size_t payload_source(void* ptr, size_t size, size_t nmemb, void* userp) {
	upload_status* upload_ctx = (upload_status*)userp;
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
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

struct MyCurl {
	CURLcode res = CURLE_OK;
	//Sender
	string content;
	string serverAccount;
	string serverPassword;
	string clientAccount;
	string clientPassword;
	CURL* sender = nullptr;
	curl_slist* recipients = nullptr;
	upload_status upload_ctx = { 0 };
	//Reader
	CURL* receiver = nullptr;
	vector<string> clientIDs; //Get from MYSQL
	vector<int> UIDs;
	string receiveBuffer;
	void CreateEmail(string& id) {
		email_payload_text =
			"Subject: Test email" + id + "\r\n"
			"\r\n"
			+ content + "\r\n";
	}
	void InitSender() {
		//Set the SMTP server URL
		curl_easy_setopt(sender, CURLOPT_URL, smtpURL.c_str());
		// Enable TLS for secure connection
		curl_easy_setopt(sender, CURLOPT_USE_SSL, CURLUSESSL_ALL);
		//Set the login credentials
		curl_easy_setopt(sender, CURLOPT_USERNAME, clientAccount.c_str());
		curl_easy_setopt(sender, CURLOPT_PASSWORD, clientPassword.c_str());
		//Set the sender email address
		curl_easy_setopt(sender, CURLOPT_MAIL_FROM, clientAccount.c_str());
		//Add recipient(s)
		recipients = curl_slist_append(recipients, serverAccount.c_str());
		curl_easy_setopt(sender, CURLOPT_MAIL_RCPT, recipients);
		// Specify the payload
		curl_easy_setopt(sender, CURLOPT_READFUNCTION, payload_source);
		curl_easy_setopt(sender, CURLOPT_READDATA, &upload_ctx);
		curl_easy_setopt(sender, CURLOPT_UPLOAD, 1L);
	}
	void InitReceiverSession(string &URL) {
		receiver = curl_easy_init();
		curl_easy_setopt(receiver, CURLOPT_URL, URL.c_str());
		curl_easy_setopt(receiver, CURLOPT_USERNAME, serverAccount.c_str());
		curl_easy_setopt(receiver, CURLOPT_PASSWORD, serverPassword.c_str());
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
	vector<int> GetListUIDUnseen() {
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
	void SendEmail(string content) {
		this->content = content;
		for (auto& id : clientIDs) {
			CreateEmail(id);
			res = curl_easy_perform(sender);
			// Check for errors
			if (res != CURLE_OK) {
				cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
			}
			else {
				cout << "Email sent successfully!" << endl;
			}
		}
	}
	void ReadEmail() {
		//Search
		if (UIDs.size() == 0) {
			InitReceiverSession(imapsURL);
			curl_easy_setopt(receiver, CURLOPT_CUSTOMREQUEST, "UID SEARCH UNSEEN");
			//curl_easy_setopt(receiver, CURLOPT_VERBOSE, 1L); // Debug
			res = curl_easy_perform(receiver);
			curl_easy_setopt(receiver, CURLOPT_CUSTOMREQUEST, nullptr);
			UIDs = GetListUIDUnseen(); //for read
			CleanSession(receiver, receiveBuffer);
		}
		//Read
		for (int UID : UIDs) {
			string emailURL = imapsURL + ";UID=" + to_string(UID);
			InitReceiverSession(emailURL);
			res = curl_easy_perform(receiver);
			size_t bodyStart = receiveBuffer.find("\r\n\r\n");
			receiveBuffer = receiveBuffer.substr(bodyStart + 4);
			//***********************************************************************************
			//Do something with the email body
			cout << "Email Body " << UID << ": " << receiveBuffer << endl;
			CleanSession(receiver, receiveBuffer);
		}
		UIDs.clear();
	}
	MyCurl() {
		content = "Hello, this is a test email!";
		serverAccount = "chillseverctt2@gmail.com";
		serverPassword = "frmc opak fwnu ftos";
		clientAccount = "chillclientctt2@gmail.com";
		clientPassword = "kdsr vqde utzq wtic";
		sender = nullptr;
		res = CURLE_OK;
		recipients = nullptr;
		upload_ctx = { 0 };

		receiver = nullptr;
		vector<int> UIDs;
		string receiveBuffer;
		InitSender();
	}
	~MyCurl() {
		if(!recipients) curl_slist_free_all(recipients);
		if(!sender) curl_easy_cleanup(sender);
		if(!receiver) curl_easy_cleanup(receiver);
	}
};
#endif // !GMAILAPI_H
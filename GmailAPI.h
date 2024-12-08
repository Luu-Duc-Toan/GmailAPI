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
	string gmail;
	string clientAccount;
	string clientPassword;
	string id;
	CURL* sender = nullptr;
	curl_slist* recipients = nullptr;
	upload_status upload_ctx = { 0 };
	//Reader
	CURL* receiver = nullptr;
	vector<string> clientIDs; //Get from MYSQL
	vector<int> UIDs;
	string searchQuery;
	string receiveBuffer;
	void SetID(string& id);
	void SetAccount(string& serverAccount, string& serverPassword, string& gmail);
	void AddClientId(string id);
	void RemoveClientId(string id);
	void CreateEmail(string& id);
	void InitSender();
	void InitReceiverSession(string& URL);
	void CleanSession(CURL*& session, string& buffer);
	void UpdateSearchQuery();
	vector<int> GetListUIDUnseen();
	void SendEmail(string content);
	void ReadEmail();
	MyCurl();
	~MyCurl();
};
#endif // !GMAILAPI_H
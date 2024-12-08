#include "GmailAPI.h"

string imapsURL = "imaps://imap.gmail.com:993/RemoteUltilities";
string smtpURL = "smtp://smtp.gmail.com:587";
string email_payload_text = "";
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
	((string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}
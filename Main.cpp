#include <iostream>
#include <string>
#include <sstream>
#include <curl/curl.h>
using namespace std;

// Email data
static const char* email_payload_text =
"From: <chillclientctt2@gmail.com>\r\n"
"To: <chillseverctt2@gmail.com>\r\n"
"Subject: Test Email from libcurl\r\n"
"\r\n"
"This is a test email sent using libcurl!\r\n";

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

int main() {
	CURL* sender;
	CURLcode res = CURLE_OK;
	struct curl_slist* recipients = nullptr;
	struct upload_status upload_ctx = { 0 };

	CURL* receiver;
	string readBuffer;

	sender = curl_easy_init();
	receiver = curl_easy_init();

	//if (sender) {
	//	// Set the SMTP server URL
	//	curl_easy_setopt(sender, CURLOPT_URL, "smtp://smtp.gmail.com:587");
	//	// Enable TLS for secure connection
	//	curl_easy_setopt(sender, CURLOPT_USE_SSL, CURLUSESSL_ALL);

	//	// Set the login credentials
	//	curl_easy_setopt(sender, CURLOPT_USERNAME, "chillclientctt2@gmail.com");
	//	curl_easy_setopt(sender, CURLOPT_PASSWORD, "kdsr vqde utzq wtic");

	//	// Set the sender email address
	//	curl_easy_setopt(sender, CURLOPT_MAIL_FROM, "chillclientctt2@gmail.com");

	//	// Add recipient(s)
	//	recipients = curl_slist_append(recipients, "chillseverctt2@gmail.com");
	//	curl_easy_setopt(sender, CURLOPT_MAIL_RCPT, recipients);

	//	// Specify the payload
	//	curl_easy_setopt(sender, CURLOPT_READFUNCTION, payload_source);
	//	curl_easy_setopt(sender, CURLOPT_READDATA, &upload_ctx);
	//	curl_easy_setopt(sender, CURLOPT_UPLOAD, 1L);

	//	// Perform the email send
	//	res = curl_easy_perform(sender);

	//	// Check for errors
	//	if (res != CURLE_OK) {
	//		cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
	//	}
	//	else {
	//		cout << "Email sent successfully!" << endl;
	//	}
	//}

	if (receiver) {
		// URL tới máy chủ IMAP (ví dụ: Gmail)
		curl_easy_setopt(receiver, CURLOPT_URL, "imaps://imap.gmail.com:993/INBOX;UID=12");

		// Cung cấp thông tin đăng nhập
		curl_easy_setopt(receiver, CURLOPT_USERNAME, "chillseverctt2@gmail.com");
		curl_easy_setopt(receiver, CURLOPT_PASSWORD, "frmc opak fwnu ftos");
		
		// SSL/TLS Settings
		curl_easy_setopt(receiver, CURLOPT_USE_SSL, CURLUSESSL_ALL);
		curl_easy_setopt(receiver, CURLOPT_SSL_VERIFYPEER, 1L);
		curl_easy_setopt(receiver, CURLOPT_SSL_VERIFYHOST, 2L);
		curl_easy_setopt(receiver, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
		
		
		curl_easy_setopt(receiver, CURLOPT_SSL_VERIFYPEER, 0L);

		// Callback để xử lý dữ liệu nhận được
		curl_easy_setopt(receiver, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(receiver, CURLOPT_WRITEDATA, &readBuffer);


		//curl_easy_setopt(receiver, CURLOPT_CUSTOMREQUEST, "UID SEARCH ALL");

		curl_easy_setopt(receiver, CURLOPT_VERBOSE, 1L);
		res = curl_easy_perform(receiver);


		if (res != CURLE_OK) {
			cerr << "Failed to send FETCH command: " << curl_easy_strerror(res) << endl;
		}
		else {
			cout << "Message content: " << readBuffer << endl;
		}

		// Clean up
		curl_slist_free_all(recipients);
		curl_easy_cleanup(sender);
		curl_easy_cleanup(receiver);
	}

	return 0;
}

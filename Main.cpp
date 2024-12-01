#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
using namespace std;
using json = nlohmann::json;

// Callback function to capture the HTTP response
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

// Step 1: Get the access token using OAuth 2.0
std::string getAccessToken(const std::string& clientId, const std::string& clientSecret, const std::string& authCode) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        std::string postFields = "code=" + authCode + "&client_id=" + clientId + "&client_secret=" + clientSecret +
            "&redirect_uri=http://localhost&grant_type=authorization_code";

        curl_easy_setopt(curl, CURLOPT_URL, "https://oauth2.googleapis.com/token");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Curl error: " << curl_easy_strerror(res) << std::endl;
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    auto jsonResponse = json::parse(readBuffer);
    return jsonResponse["access_token"];
}

// Step 2: Send an email using Gmail API
std::string sendEmail(const std::string& accessToken, const std::string& toEmail, const std::string& subject, const std::string& body) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    std::string url = "https://gmail.googleapis.com/upload/gmail/v1/users/me/messages/send?uploadType=multipart";

    // Construct the MIME email message
    std::string message = "To: " + toEmail + "\r\n" +
        "Subject: " + subject + "\r\n\r\n" +
        body;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
    headers = curl_slist_append(headers, "Content-Type: message/rfc822");

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, message.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Curl error: " << curl_easy_strerror(res) << std::endl;
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    return readBuffer;
}

int main() {
    std::string clientId = "your-client-id";  // Replace with your actual client ID
    std::string clientSecret = "your-client-secret";  // Replace with your actual client secret
    std::string authCode = "authorization-code-from-user";  // Replace with the actual authorization code

    // Step 1: Get the access token
    std::string accessToken = getAccessToken(clientId, clientSecret, authCode);
    std::cout << "Access Token: " << accessToken << std::endl;

    // Step 2: Send an email
    std::string toEmail = "recipient@example.com";  // Replace with the recipient's email
    std::string subject = "Test Email from C++";
    std::string body = "This is a test email sent from C++ using Gmail API.";

    std::string response = sendEmail(accessToken, toEmail, subject, body);
    std::cout << "Email sent. Response: " << response << std::endl;

    return 0;
}
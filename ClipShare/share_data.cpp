#include "share_data.h"

ShareData::ShareData() {}

ShareData ShareData::parse(const std::string& data)
{
	ShareData shareData = ShareData();

	json jsonData = json::parse(data);

	shareData.text = jsonData["clipboard"]["text"];
	shareData.files = jsonData["clipboard"]["files"];

	shareData.request_files = jsonData["request_files"];
	shareData.response_files = jsonData["response_files"];

	shareData.id = jsonData["id"];

	return shareData;
}

std::string ShareData::build() const {
  json jsonData;

  jsonData["clipboard"]["text"] = this->text;
  jsonData["clipboard"]["files"] = this->files;

  jsonData["request_files"] = this->request_files;
  jsonData["response_files"] = this->response_files;

  jsonData["id"] = this->id;

  return jsonData.dump();
}
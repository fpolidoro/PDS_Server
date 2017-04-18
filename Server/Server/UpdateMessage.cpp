#include "UpdateMessage.h"

UpdateMessage::UpdateMessage() {
	this->type = UpdateType::WND_FOCUSED;
	this->wndId = -1;
	this->wndName = "";
	this->wndIcon = std::string("");
}

UpdateMessage::UpdateMessage(UpdateType type, int wndId, std::string wndName, std::string wndIcon) {
	this->type = type;
	this->wndId = wndId;
	this->wndName = wndName;
	this->wndIcon = wndIcon;
}

UpdateMessage::~UpdateMessage() {
}

void UpdateMessage::serialize(cereal::JSONOutputArchive& archive) {
	archive(CEREAL_NVP(type), CEREAL_NVP(wndId), CEREAL_NVP(wndName), CEREAL_NVP(wndIcon));
}

void UpdateMessage::deserialize(cereal::JSONInputArchive& archive) {
	archive(type, wndId, wndName, wndIcon);
}
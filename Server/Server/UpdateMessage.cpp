#include "UpdateMessage.h"

UpdateMessage::UpdateMessage() {
	this->type = UpdateType::WND_FOCUSED;
	this->wndId = -1;
	this->wndName = "";
}

UpdateMessage::UpdateMessage(UpdateType type, int wndId, std::string wndName) {
	this->type = type;
	this->wndId = wndId;
	this->wndName = wndName;
}

UpdateMessage::~UpdateMessage() {
}

void UpdateMessage::serialize(cereal::JSONOutputArchive& archive) {
	archive(CEREAL_NVP(type), CEREAL_NVP(wndId), CEREAL_NVP(wndName));
}

void UpdateMessage::deserialize(cereal::JSONInputArchive& archive) {
	archive(type, wndId, wndName);
}
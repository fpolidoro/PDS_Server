#include "UpdateMessage.h"

UpdateMessage::UpdateMessage() {
	this->type = UpdateType::WND_FOCUSED;
	this->wndId = 0;
	this->wndName = "";
	this->procName = "";
	this->wndIcon = "";
}

UpdateMessage::UpdateMessage(UpdateType type, int wndId, std::string wndName, std::string procName, std::string wndIcon) {
	this->type = type;
	this->wndId = wndId;
	this->wndName = wndName;
	this->procName = procName;
	this->wndIcon = wndIcon;
}

void UpdateMessage::serialize(cereal::JSONOutputArchive& archive) {
	archive(CEREAL_NVP(type), CEREAL_NVP(wndId), CEREAL_NVP(wndName), CEREAL_NVP(procName), CEREAL_NVP(wndIcon));
}

void UpdateMessage::deserialize(cereal::JSONInputArchive& archive) {
	archive(type, wndId, wndName, procName, wndIcon);
}
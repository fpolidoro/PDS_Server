#include "KeyMessage.h"


KeyMessage::KeyMessage() {
}


KeyMessage::~KeyMessage() {
}


void KeyMessage::serialize(cereal::JSONOutputArchive& archive) {
	archive(CEREAL_NVP(nKeys), CEREAL_NVP(keys));
}

void KeyMessage::deserialize(cereal::JSONInputArchive& archive) {
	archive(nKeys, keys);
}
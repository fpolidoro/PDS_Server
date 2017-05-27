#include "KeyMessage.h"

void KeyMessage::serialize(cereal::JSONOutputArchive& archive) {
	archive(CEREAL_NVP(procName), CEREAL_NVP(nKeys), CEREAL_NVP(keys));
}

void KeyMessage::deserialize(cereal::JSONInputArchive& archive) {
	archive(procName, nKeys, keys);
}
#pragma once
#include "cereal\archives\json.hpp"

class KeyMessage {
public:
	int wndId;
	int nKeys;
	int keys[5];

	void serialize(cereal::JSONOutputArchive& archive);
	void deserialize(cereal::JSONInputArchive& archive);
};


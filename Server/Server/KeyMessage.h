#pragma once
#include "cereal\archives\json.hpp"

class KeyMessage {
public:
	int nKeys;
	int keys[4];

	KeyMessage();
	~KeyMessage();

	void serialize(cereal::JSONOutputArchive& archive);
	void deserialize(cereal::JSONInputArchive& archive);
};


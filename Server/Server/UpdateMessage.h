#pragma once
#include "MessageTypes.h"
#include "cereal\archives\json.hpp"
#include <string>

class UpdateMessage {


public:

	UpdateType type;
	int wndId;
	std::string wndName;

	UpdateMessage();
	UpdateMessage(UpdateType type, int wndId, std::string wndName);
	~UpdateMessage();

	void serialize(cereal::JSONOutputArchive& archive);
	void deserialize(cereal::JSONInputArchive& archive);
};
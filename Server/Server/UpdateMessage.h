#pragma once
#include "MessageTypes.h"
#include "cereal\archives\json.hpp"
#include "cereal\types\memory.hpp"
#include <string>

class UpdateMessage {


public:

	UpdateType type;
	int wndId;
	std::string wndName;
	std::string wndIcon;

	UpdateMessage();
	UpdateMessage(UpdateType type, int wndId, std::string wndName, std::string wndIcon);
	~UpdateMessage();

	void serialize(cereal::JSONOutputArchive& archive);
	void deserialize(cereal::JSONInputArchive& archive);
};
#pragma once
#include "rapidjson\prettywriter.h"
#include "rapidjson\document.h"

class WindowCreated {
public:
	int id;
	char title[100];

	template <typename Writer>
	void Serialize(Writer& writer) const {
		writer.StartObject();
		
		writer.String("id:");
		writer.Int(id);
		writer.String("title:");
		writer.String(title);

		writer.EndObject();
	}

	template <typename Document>
	void Deserialize(Document& document) {
		id = document["id"].GetInt();
		strcpy_s(title, document["title"].GetString());
	}
};


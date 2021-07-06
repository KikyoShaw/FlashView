#include "KeyFrameData.h"

shine::KeyFrameData::KeyFrameData()
{
}

shine::KeyFrameData::KeyFrameData(const Json::Value& oneFrame, bool hasMark)
{
	texName = oneFrame["texName"].asString();
	x = oneFrame["x"].asFloat();
	y = oneFrame["y"].asFloat();
	scaleX = oneFrame["scaleX"].asFloat();
	scaleY = oneFrame["scaleY"].asFloat();
	skewX = oneFrame["skewX"].asFloat();
	skewY = oneFrame["skewY"].asFloat();
	alpha = oneFrame["alpha"].asInt() / 255.0f;
	blendColor = QColor(oneFrame["r"].asInt(), oneFrame["g"].asInt(), oneFrame["b"].asInt(), oneFrame["a"].asInt());
	if (hasMark) {
		std::string m = oneFrame["mark"].asString();
		if (!m.empty()) {
			mark = m;
		}
	}
}

shine::KeyFrameData::~KeyFrameData()
{
}

shine::KeyFrameData *shine::KeyFrameData::deepClone()
{
	KeyFrameData *newData = new KeyFrameData;
	newData->x = this->x;
	newData->y = this->y;
	newData->scaleX = this->scaleX;
	newData->scaleY = this->scaleY;
	newData->skewX = this->skewX;
	newData->skewY = this->skewY;
	newData->alpha = this->alpha;
	newData->blendColor = this->blendColor;
	newData->mark = this->mark;
	return newData;
}

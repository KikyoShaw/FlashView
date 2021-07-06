#pragma once

#include <iostream>
#include "json.h"
#include <QColor>

namespace shine {

class KeyFrameData
{
	public:
		std::string texName; // image name
		float x; // pos x
		float y; // pos y
		float scaleX;
		float scaleY;
		float skewX;
		float skewY;
		float alpha; // image alpha
		QColor blendColor;
		std::string mark;
	public:
		KeyFrameData();
		KeyFrameData(const Json::Value& oneFrame, bool hasMark);
		~KeyFrameData();
		//Éî¿½±´
		KeyFrameData *deepClone();
};

}

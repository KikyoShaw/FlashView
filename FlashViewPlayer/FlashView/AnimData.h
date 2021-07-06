#pragma once
#include <unordered_map>
#include <iostream>
#include <vector>
#include "KeyFrameData.h"

namespace shine {

	class AnimData
	{
		public:
			std::unordered_map<std::string, std::vector<KeyFrameData*>*>* keyFrameData = nullptr;
			unsigned int animFrameLen;

		public:
			AnimData();
			~AnimData();
	};
}


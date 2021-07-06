#include "AnimData.h"

shine::AnimData::AnimData()
{
}

shine::AnimData::~AnimData()
{
	for (auto & keyFramesPair : (*keyFrameData)) {
		std::vector<KeyFrameData*>* keyFrames = keyFramesPair.second;
		if (keyFrames == nullptr) {
			continue;
		}
		for (int i = 0, c = keyFrames->size(); i < c; i++) {
			delete (*keyFrames)[i];
		}
		keyFrames->clear();
		delete keyFrames;
	}
	delete keyFrameData;
}

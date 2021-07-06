#include "FlashViewCallback.h"

shine::FlashViewEventData::FlashViewEventData()
{
}

shine::FlashViewEventData::~FlashViewEventData()
{
	if (data != nullptr) {
		delete data;
	}
}

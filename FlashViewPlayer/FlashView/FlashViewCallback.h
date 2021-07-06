#pragma once
#include <iostream>
#include <functional>
#include "KeyFrameData.h"

namespace shine {
	
class FlashViewEventData
{
public:
	int index;
	std::string mark;
	KeyFrameData *data = nullptr;

public:
	FlashViewEventData();
	~FlashViewEventData();
};

enum FlashViewEvent {
	START,
	ONELOOPEND,
	STOP,
	MARK,
	FRAME,
};

typedef std::function<void(FlashViewEvent e, FlashViewEventData *data)> FlashViewCallback;
	
}



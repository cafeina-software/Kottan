/*
 * Copyright 2025 Cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 *
 */
#ifndef __MSG_INFO_WINDOW__
#define __MSG_INFO_WINDOW__

#include <CheckBox.h>
#include <TextControl.h>
#include <Window.h>
#include <private/interface/ColumnListView.h>

#define STRINGIFY(x) (#x)

class MsgInfoWindow : public BWindow
{
public:
					MsgInfoWindow(BRect frame, BMessage* data);
private:
			void 	InitUIData(const BMessage* data);
	const 	char* 	StringForSpecifierWhat(uint32 what);
private:
	BCheckBox* 		fCbIsSystem;
	BCheckBox* 		fCbIsReply;
	BCheckBox* 		fCbWasDelivered;
	BCheckBox* 		fCbWasDropped;
	BCheckBox* 		fCbHasSpecifiers;
	BTextControl* 	fTcWhat;
	BTextControl* 	fTcCountNames;
	BTextControl* 	fTcFlattenedSize;
};

#endif /* __MSG_INFO_WINDOW__ */

/*
 * Copyright 2025 Cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 *
 */
#ifndef __WHAT_WINDOW__
#define __WHAT_WINDOW__

#include <MenuField.h>
#include <PopUpMenu.h>
#include <RadioButton.h>
#include <TextControl.h>
#include <Window.h>

enum WhatCmds {
	WCMD_PREDEFINED_SELECTED,
	WCMD_CUSTOM_SELECTED,
	WCMD_PREDEFINED_MODIFIED,
	WCMD_CUSTOM_MODIFIED,
	WCMD_SAVE_WHAT_REQUESTED,
	WCMD_CLOSE_REQUESTED
};

class WhatWindow : public BWindow
{
public:
							WhatWindow(BRect frame, BMessage* data = NULL);

	virtual	void			MessageReceived(BMessage* msg);
private:
			void			BuildPredefinedValuesMenu();
			void			SetCustomValue(const char* valueAsText);
private:
	uint32					fInitialValue;
	uint32 					fInternalWhat;

	BButton*				fBtSave;
	BButton*				fBtCancel;
	BMenuField*				fMfPredefinedWhats;
	BPopUpMenu*				fPumPredefinedWhatMenu;
	BRadioButton*			fRbUsePredefined;
	BRadioButton*			fRbUseCustomValue;
	BTextControl*			fTcWhatNumber;
};

#define PREDEFINED_ENTRY(x) { x, #x }

struct predefined_cmds {
	uint32 command;
	const char* name;
};

#endif /* __WHAT_WINDOW__*/

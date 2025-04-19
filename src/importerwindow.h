/*
 * Copyright 2025 Cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 *
 */
#ifndef __IMPORTER_WINDOW__
#define __IMPORTER_WINDOW__

#include <RadioButton.h>
#include <TextControl.h>
#include <Window.h>

enum ImporterCmds {
	IMP_MODE_MEMBER = 'imp0',
	IMP_MODE_CONTENTS_ONLY,
	IMP_PATH_MODIFIED,
	IMP_NAME_MODIFIED,
	IMP_OPEN_REQUESTED,
	IMP_OPEN_REPLY,
	IMP_SAVE_REQUESTED,
	IMP_CLOSE_REQUESTED
};

class ImporterWindow : public BWindow
{
public:
					ImporterWindow(BRect frame);
	virtual void 	MessageReceived(BMessage* msg);
private:
			bool	CanImport();
private:
	BButton* 		fBtBrowse;
	BButton* 		fBtSave;
	BButton* 		fBtCancel;
	BRadioButton*	fRbModeMember;
	BRadioButton*	fRbModeContents;
	BTextControl*	fTcMessageName;
	BTextControl*	fTcPathString;
};

#endif /* __IMPORTER_WINDOW__ */

/*
 * Copyright 2019-2021 Andi Machovec <andi.machovec@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 *
 */

#ifndef EDITWINDOW_H
#define EDITWINDOW_H

#include <Window.h>
#include <Button.h>

#include "editview.h"


enum
{
	EW_BUTTON_CANCEL='ew00',
	EW_BUTTON_SAVE,
	EW_REFS_RECEIVED,
	EW_DATA_SAVE
};


class EditWindow : public BWindow {
public:
						EditWindow(BRect frame,
							BMessage *data_message,
							type_code data_type,
							const char *data_label,
							int32 data_index, bool creating = false);
	virtual	void 		MessageReceived(BMessage *msg);
private:
	BMessage    *dataMessage;
	BString		 dataLabel;
	type_code	 dataType;
	int32		 dataIndex;
	bool		 isCreating;

	BButton		*fCancelButton;
	BButton		*fSaveButton;
	EditView	*fEditView;
};

#endif

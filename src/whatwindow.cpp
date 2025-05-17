#include <Application.h>
#include <Button.h>
#include <Catalog.h>
#include <LayoutBuilder.h>
#include <ctype.h>
#include "whatwindow.h"

static predefined_cmds app_defs [] = {
	PREDEFINED_ENTRY(B_SET_PROPERTY),
	PREDEFINED_ENTRY(B_GET_PROPERTY),
	PREDEFINED_ENTRY(B_CREATE_PROPERTY),
	PREDEFINED_ENTRY(B_DELETE_PROPERTY),
	PREDEFINED_ENTRY(B_COUNT_PROPERTIES),
	PREDEFINED_ENTRY(B_EXECUTE_PROPERTY),
	PREDEFINED_ENTRY(B_GET_SUPPORTED_SUITES),
	PREDEFINED_ENTRY(B_UNDO),
	PREDEFINED_ENTRY(B_REDO),
	PREDEFINED_ENTRY(B_CUT),
	PREDEFINED_ENTRY(B_COPY),
	PREDEFINED_ENTRY(B_PASTE),
	PREDEFINED_ENTRY(B_SELECT_ALL),
	PREDEFINED_ENTRY(B_SAVE_REQUESTED),
	PREDEFINED_ENTRY(B_MESSAGE_NOT_UNDERSTOOD),
	PREDEFINED_ENTRY(B_NO_REPLY),
	PREDEFINED_ENTRY(B_REPLY),
	PREDEFINED_ENTRY(B_SIMPLE_DATA),
	PREDEFINED_ENTRY(B_MIME_DATA),
	PREDEFINED_ENTRY(B_ARCHIVED_OBJECT),
	// PREDEFINED_ENTRY(B_UPDATE_STATUS_BAR),
	// PREDEFINED_ENTRY(B_RESET_STATUS_BAR),
	PREDEFINED_ENTRY(B_NODE_MONITOR),
	PREDEFINED_ENTRY(B_QUERY_UPDATE),
	PREDEFINED_ENTRY(B_ENDORSABLE),
	// PREDEFINED_ENTRY(B_COPY_TARGET),
	// PREDEFINED_ENTRY(B_MOVE_TARGET),
	// PREDEFINED_ENTRY(B_TRASH_TARGET),
	// PREDEFINED_ENTRY(B_LINK_TARGET),
	// PREDEFINED_ENTRY(B_INPUT_DEVICES_CHANGED),
	// PREDEFINED_ENTRY(B_INPUT_METHOD_EVENT),
	// PREDEFINED_ENTRY(B_WINDOW_MOVE_TO),
	// PREDEFINED_ENTRY(B_WINDOW_MOVE_BY),
	// PREDEFINED_ENTRY(B_SILENT_RELAUNCH),
	// PREDEFINED_ENTRY(B_OBSERVER_NOTICE_CHANGE),
	// PREDEFINED_ENTRY(B_CONTROL_INVOKED),
	// PREDEFINED_ENTRY(B_CONTROL_MODIFIED),
};

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "What window?"

WhatWindow::WhatWindow(BRect frame, BMessage* data)
: BWindow(frame, B_TRANSLATE("Edit message type"), B_FLOATING_WINDOW,
	B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS),
  fInitialValue(0),
  fInternalWhat(0)
{
	if(data) {
		fInternalWhat = data->what;
		fInitialValue = data->what;
	}

	fRbUsePredefined = new BRadioButton(B_TRANSLATE("Use a predefined value"),
		new BMessage(WCMD_PREDEFINED_SELECTED));
	fPumPredefinedWhatMenu = new BPopUpMenu("");
	fMfPredefinedWhats = new BMenuField(B_TRANSLATE("Select predefined value:"),
		fPumPredefinedWhatMenu);
	BuildPredefinedValuesMenu();
	fMfPredefinedWhats->SetEnabled(false);

	fRbUseCustomValue = new BRadioButton(B_TRANSLATE("Use a custom value"),
		new BMessage(WCMD_CUSTOM_SELECTED));
	fRbUseCustomValue->SetValue(B_CONTROL_ON); // Assume the initial value as custom
	fTcWhatNumber = new BTextControl(B_TRANSLATE("Write a value:"), "", new BMessage(WCMD_CUSTOM_MODIFIED));
	fTcWhatNumber->SetModificationMessage(new BMessage(WCMD_CUSTOM_MODIFIED));
	for(auto i = 0; i < 256; i++)
		fTcWhatNumber->TextView()->DisallowChar(i);
	for(auto i = '0'; i <= '9'; i++)
		fTcWhatNumber->TextView()->AllowChar(i);
	fTcWhatNumber->TextView()->AllowChar('-');
	fTcWhatNumber->SetText(BString().SetToFormat("%d", fInitialValue).String());

	fBtSave = new BButton(B_TRANSLATE("Save"), new BMessage(WCMD_SAVE_WHAT_REQUESTED));
	fBtSave->SetEnabled(fInternalWhat != fInitialValue);
	fBtCancel = new BButton(B_TRANSLATE("Cancel"), new BMessage(WCMD_CLOSE_REQUESTED));

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_SMALL_INSETS)
		.Add(fRbUsePredefined)
		.Add(fMfPredefinedWhats)
		.AddStrut(4.0f)
		.Add(fRbUseCustomValue)
		.Add(fTcWhatNumber)
		.AddStrut(4.0f)
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(fBtSave)
			.Add(fBtCancel)
			.AddGlue()
		.End()
	.End();
}

void
WhatWindow::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
		case WCMD_PREDEFINED_SELECTED:
		case WCMD_CUSTOM_SELECTED:
		{
			bool isCustom = msg->what == WCMD_CUSTOM_SELECTED;

			/* The value changes to the one stored in the active section */
			if(isCustom) {
				if(fTcWhatNumber->TextLength() == 0)
					fInternalWhat = 0;
				else
					SetCustomValue(fTcWhatNumber->Text());
			}
			else {
				BMenuItem* marked = fMfPredefinedWhats->Menu()->FindMarked();
				if(!marked) 			// Avoid segfault when choosing the predefined
					fInternalWhat = 0; 	//	option without anything marked
				else {
					BMessage* item_msg = marked->Message();
					if(item_msg)
						fInternalWhat = item_msg->GetUInt32("what_value", 0);
					else // In case the menu item has the BMessage* as NULL
						fInternalWhat = 0;
				}
			}

			/* Update user interface */
			fMfPredefinedWhats->SetEnabled(!isCustom);
			fTcWhatNumber->SetEnabled(isCustom);
			fBtSave->SetEnabled(fInternalWhat != fInitialValue);
			break;
		}
		case WCMD_PREDEFINED_MODIFIED:
		{
			fInternalWhat = msg->GetUInt32("what_value", 0);
			BString valueAsText;
			valueAsText << fInternalWhat;
			fTcWhatNumber->SetText(valueAsText);

			fBtSave->SetEnabled(fInternalWhat != fInitialValue);
			break;
		}
		case WCMD_CUSTOM_MODIFIED:
		{
			SetCustomValue(fTcWhatNumber->Text());

			fBtSave->SetEnabled(fInternalWhat != fInitialValue);
			break;
		}
		case WCMD_SAVE_WHAT_REQUESTED:
		{
			BMessage request(msg->what);
			request.AddUInt32("what", fInternalWhat);
			be_app->PostMessage(&request);
			Quit();
			break;
		}
		case WCMD_CLOSE_REQUESTED:
			Quit();
			break;
		default:
			return BWindow::MessageReceived(msg);
	}
}

void
WhatWindow::BuildPredefinedValuesMenu()
{
	if(!fPumPredefinedWhatMenu)
		return;

	BMessage* message = NULL;
	for(const auto& it : app_defs) {
		message = new BMessage(WCMD_PREDEFINED_MODIFIED);
		message->AddUInt32("what_value", it.command);
		fPumPredefinedWhatMenu->AddItem(new BMenuItem(it.name, message));
	}
}

void
WhatWindow::SetCustomValue(const char* valueAsText)
{
	if(!valueAsText)
		return;

	BString result;

	// Sanitize
	for(size_t i = 0; i < strlen(valueAsText); i++) {
		if(isdigit(valueAsText[i]) != 0 || (valueAsText[i] == '-' && i == 0))
			result << valueAsText[i];
	}

	fInternalWhat = static_cast<uint32>(atoi(result.String()));
}

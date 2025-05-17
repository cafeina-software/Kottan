/*
 * Copyright 2019-2021 Andi Machovec <andi.machovec@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 *
 */

#include "editwindow.h"
#include "kottandefs.h"
#include "visualwindow.h"

#include <LayoutBuilder.h>
#include <Catalog.h>
#include <Application.h>
#include <Path.h>
#include <cassert>
#include <cstdio>
#include <ctime>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "EditWindow"


EditWindow::EditWindow(BRect frame, BMessage *data_message,
	type_code data_type, const char *data_label, int32 data_index, bool creating,
	const void* caller)
	: BWindow(frame, "", B_DOCUMENT_WINDOW_LOOK,B_MODAL_APP_WINDOW_FEEL, B_CLOSE_ON_ESCAPE
		| B_NOT_ZOOMABLE | B_NOT_V_RESIZABLE | B_AUTO_UPDATE_SIZE_LIMITS),
	dataMessage(data_message),
	dataLabel(data_label),
	dataType(data_type),
	dataIndex(data_index),
	isCreating(creating),
	callerMessenger(caller)
{
	SetTitle(isCreating ? B_TRANSLATE("Add new entry") : B_TRANSLATE("Edit entry"));

	fEditView = new EditView(data_message, dataType, dataLabel, dataIndex, isCreating);
	fCancelButton = new BButton(B_TRANSLATE("Cancel"), new BMessage(EW_BUTTON_CANCEL));
	fSaveButton = new BButton(B_TRANSLATE("Save"), new BMessage(EW_BUTTON_SAVE));
	fSaveButton->SetEnabled(false);

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_SMALL_SPACING)
		.SetInsets(B_USE_SMALL_INSETS)
		.Add(fEditView)
		.AddStrut(B_USE_BIG_SPACING)
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(fCancelButton)
			.Add(fSaveButton)
			.AddGlue()
		.End()
		.AddGlue(100)
	.Layout();

	AddShortcut('W', B_COMMAND_KEY, new BMessage(EW_BUTTON_CANCEL));
}

EditWindow::~EditWindow()
{
}

void
EditWindow::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		case EW_BUTTON_SAVE:
		{
			fEditView->SaveData();
			BMessage reply(msg->what);
			reply.AddBool("create", isCreating);
			if(callerMessenger)
				reply.AddPointer("target", callerMessenger);
			be_app->PostMessage(&reply);
			Quit();
			break;
		}

		case EW_BUTTON_CANCEL:
			be_app->PostMessage(msg);
			Quit();
			break;

		case EV_DATA_CHANGED:
			fEditView->ValidateData();
			fSaveButton->SetEnabled(fEditView->IsSaveable());
			break;

		case EV_REF_REQUESTED:
		{
			Hide();
			BMessage request(msg->what);
			BMessenger messenger(this);
			request.AddMessenger("target", messenger);
			be_app->PostMessage(&request); // Call the open panel from App
			break;
		}

		case EV_MEASUREMENT_REQUESTED:
		{
			BMessage request(msg->what);
			request.AddString(KottanFieldName, dataLabel);
			request.AddUInt32(KottanFieldType, dataType);
			request.AddInt32(KottanFieldIndex, dataIndex);
			request.AddPointer("target", this);
			be_app->PostMessage(&request);

			Hide();
			break;
		}

		case EV_GET_CURRENT_TIME:
		{
			time_t now = std::time(NULL);
			fEditView->SetDataFor(fEditView->Type(), &now);
			PostMessage(EV_DATA_CHANGED);
			break;
		}

		// Received an answer from the open panel: a file ref
		case EW_REFS_RECEIVED:
		{
			if(IsHidden())
				Show();

			entry_ref ref;
			if(msg->FindRef("refs", &ref) == B_OK) {
				fEditView->SetDataFor(fEditView->Type(), &ref);
				PostMessage(EV_DATA_CHANGED);
			}
			break;
		}

		case DM_DATA_SEND:
		{
			if(IsHidden())
				Show();

			type_code type = static_cast<type_code>(msg->GetUInt32(KottanFieldType, B_ANY_TYPE));

			if(type == B_POINT_TYPE) {
				BPoint point;
				msg->FindPoint("point", &point);
				fEditView->SetDataFor(fEditView->Type(), &point);
			}
			else if(type == B_SIZE_TYPE) {
				BSize size;
				msg->FindSize("size", &size);
				fEditView->SetDataFor(fEditView->Type(), &size);
			}
			else if(type == B_RECT_TYPE) {
				BRect frame;
				msg->FindRect("frame", &frame);
				fEditView->SetDataFor(fEditView->Type(), &frame);
			}

			break;
		}

		// Received an answer from the open panel: user canceled
		case DM_CLOSE_REQUESTED:
		case B_CANCEL:
		{
			if(IsHidden())
				Show();

			break;
		}

		default:
		{
			BWindow::MessageReceived(msg);
			break;
		}
	}

}





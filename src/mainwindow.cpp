/*
 * Copyright 2019-2021 Andi Machovec <andi.machovec@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 *
 */

#include "app.h"
#include "gettype.h"
#include "importerwindow.h"
#include "kottandefs.h"
#include "mainwindow.h"
#include "whatwindow.h"

#include <Alert.h>
#include <FindDirectory.h>
#include <LayoutBuilder.h>
#include <Catalog.h>
#include <Application.h>
#include <private/interface/ColumnListView.h>
#include <private/interface/ColumnTypes.h>
#include <Entry.h>
#include <Path.h>
#include <stdio.h>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MainWindow"


MainWindow::MainWindow(BRect geometry)
	:
	BWindow(geometry, kAppName, B_DOCUMENT_WINDOW, B_ASYNCHRONOUS_CONTROLS)
{
	//initialize GUI objects
	fTopMenuBar = new BMenuBar("topmenubar");
	fMessageInfoView = new MessageView();
	fDataView = new DataView();

	//define menu layout
	BLayoutBuilder::Menu<>(fTopMenuBar)
		.AddMenu(B_TRANSLATE("File"))
			.AddItem(B_TRANSLATE("Open" B_UTF8_ELLIPSIS), MW_OPEN_MESSAGEFILE, 'O')
			.AddItem(B_TRANSLATE("Reload"), MW_RELOAD_FROM_FILE, 'R')
			.AddSeparator()
			.AddItem(B_TRANSLATE("Save"), MW_SAVE_MESSAGEFILE, 'S')
			.AddItem(B_TRANSLATE("Save as" B_UTF8_ELLIPSIS), MW_SAVE_MESSAGEFILE_AS, 'S', B_COMMAND_KEY | B_SHIFT_KEY)
			.AddSeparator()
			.AddItem(B_TRANSLATE("Close"), MW_CLOSE_MESSAGEFILE, 'W')
			.AddSeparator()
			.AddItem(B_TRANSLATE("Quit"), B_QUIT_REQUESTED, 'Q')
		.End()
		.AddMenu(B_TRANSLATE("Edit"))
			.AddItem(B_TRANSLATE("Add affine transformation" B_UTF8_ELLIPSIS), MW_ADD_AFFINE_TX)
            .AddItem(B_TRANSLATE("Add alignment" B_UTF8_ELLIPSIS), MW_ADD_ALIGNMENT)
            .AddItem(B_TRANSLATE("Add boolean" B_UTF8_ELLIPSIS), MW_ADD_BOOL)
            .AddItem(B_TRANSLATE("Add color" B_UTF8_ELLIPSIS), MW_ADD_COLOR)
            .AddMenu(B_TRANSLATE("Add integer number"))
                .AddItem(B_TRANSLATE("Signed integer (8 bits)" B_UTF8_ELLIPSIS), MW_ADD_INT8)
                .AddItem(B_TRANSLATE("Signed integer (16 bits)" B_UTF8_ELLIPSIS), MW_ADD_INT16)
                .AddItem(B_TRANSLATE("Signed integer (32 bits)" B_UTF8_ELLIPSIS), MW_ADD_INT32)
                .AddItem(B_TRANSLATE("Signed integer (64 bits)" B_UTF8_ELLIPSIS), MW_ADD_INT64)
                .AddSeparator()
                .AddItem(B_TRANSLATE("Unsigned integer (8 bits)" B_UTF8_ELLIPSIS), MW_ADD_UINT8)
                .AddItem(B_TRANSLATE("Unsigned integer (16 bits)" B_UTF8_ELLIPSIS), MW_ADD_UINT16)
                .AddItem(B_TRANSLATE("Unsigned integer (32 bits)" B_UTF8_ELLIPSIS), MW_ADD_UINT32)
                .AddItem(B_TRANSLATE("Unsigned integer (64 bits)" B_UTF8_ELLIPSIS), MW_ADD_UINT64)
                .AddSeparator()
                .AddItem(B_TRANSLATE("Offset (off_t)" B_UTF8_ELLIPSIS), MW_ADD_OFF_T)
                .AddItem(B_TRANSLATE("Size (size_t)" B_UTF8_ELLIPSIS), MW_ADD_SIZE_T)
                .AddItem(B_TRANSLATE("Size or error code (ssize_t)" B_UTF8_ELLIPSIS), MW_ADD_SSIZE_T)
            .End()
            .AddMenu(B_TRANSLATE("Add floating-point number"))
                .AddItem(B_TRANSLATE("Single precision (float)" B_UTF8_ELLIPSIS), MW_ADD_FLOAT)
                .AddItem(B_TRANSLATE("Double precision (double)" B_UTF8_ELLIPSIS), MW_ADD_DOUBLE)
            .End()
            .AddMenu(B_TRANSLATE("Add measurement"))
                .AddItem(B_TRANSLATE("Point" B_UTF8_ELLIPSIS), MW_ADD_POINT)
                .AddItem(B_TRANSLATE("Size" B_UTF8_ELLIPSIS), MW_ADD_SIZE)
                .AddItem(B_TRANSLATE("Rect" B_UTF8_ELLIPSIS), MW_ADD_RECT)
            .End()
            .AddMenu(B_TRANSLATE("Add reference"))
                .AddItem(B_TRANSLATE("Reference to file entry (entry_ref)" B_UTF8_ELLIPSIS), MW_ADD_ENTRY_REF)
                .AddItem(B_TRANSLATE("Reference to file node (node_ref)" B_UTF8_ELLIPSIS), MW_ADD_NODE_REF)
            .End()
            .AddItem(B_TRANSLATE("Add string" B_UTF8_ELLIPSIS), MW_ADD_STRING)
			.AddSeparator()
			.AddItem(B_TRANSLATE("Import message file" B_UTF8_ELLIPSIS), MW_IMPORT_MESSAGE)
		.End()
		.AddMenu(B_TRANSLATE("Message"))
            .AddItem(B_TRANSLATE("Set type (\'what\' field)" B_UTF8_ELLIPSIS), MW_MESSAGE_OPEN_SET_WHAT_DIALOG, 'T')
            .AddItem(B_TRANSLATE("Make empty" B_UTF8_ELLIPSIS), MW_MESSAGE_MAKE_EMPTY)
			.AddItem(B_TRANSLATE("Information" B_UTF8_ELLIPSIS), MW_MESSAGE_INFORMATION, 'I')
        .End()
		.AddMenu(B_TRANSLATE("View"))
			.AddItem(B_TRANSLATE("Data viewer panel"), MW_DATA_PANEL_VISIBLE)
		.End()
		.AddMenu(B_TRANSLATE("Help"))
			.AddItem(B_TRANSLATE("About" B_UTF8_ELLIPSIS), MW_MENU_ABOUT)
		.End()
	.End();

	fTopMenuBar->FindItem(MW_SAVE_MESSAGEFILE)->SetEnabled(false);
	fTopMenuBar->FindItem(MW_RELOAD_FROM_FILE)->SetEnabled(false);
	fTopMenuBar->FindItem(MW_CLOSE_MESSAGEFILE)->SetEnabled(false);
	fTopMenuBar->FindItem(MW_DATA_PANEL_VISIBLE)->SetMarked(!fDataView->IsHidden());
	fTopMenuBar->FindItem(MW_MESSAGE_INFORMATION)->SetEnabled(false); // Not yet implemented

	//define main layout
	BLayoutBuilder::Group<>(this, B_VERTICAL,0)
		.SetInsets(0)
		.Add(fTopMenuBar)
		.AddSplit(B_VERTICAL, B_USE_SMALL_SPACING)
			.SetInsets(-1,-1,-1,-1)
			.Add(fMessageInfoView, 0.5f)
			.Add(fDataView, 0.2f)
		.End()
	.Layout();

	fUnsaved = false;

}


MainWindow::~MainWindow()
{
}


void
MainWindow::MessageReceived(BMessage *msg)
{

	const char *notsaved_alert_text =
		B_TRANSLATE(
		"The message data was changed but not saved. Do you really want to open another file?"
		);
	const char *notsaved_alert_cancel = B_TRANSLATE("Cancel");
	const char *notsaved_alert_continue = B_TRANSLATE("Open file");
	const char *notsaved_alert_close = B_TRANSLATE("Close file");

	if(msg->WasDropped())
	{
		if(fUnsaved)
		{
			if(continue_action(notsaved_alert_text,
							   notsaved_alert_cancel,
							   notsaved_alert_continue))
			{
				msg->what = MW_REF_MESSAGEFILE;
			}
			else
			{
				msg->what = MW_DO_NOTHING;
			}

		}
		else
		{
			msg->what = MW_REF_MESSAGEFILE;
		}
	}

	switch(msg->what)
	{
		// Help/About was clicked
		case MW_MENU_ABOUT:
		{
			be_app->PostMessage(B_ABOUT_REQUESTED);
			break;
		}

		// Open file menu was selected
		case MW_OPEN_MESSAGEFILE:
		{
			if(fUnsaved) { // Ask for confirmation if there are pending changes
				if(!continue_action(notsaved_alert_text, notsaved_alert_cancel,
				notsaved_alert_continue))
					break;
			}

			BMessage request(msg->what);
			request.AddMessenger(KottanFieldMsgr, this);
			be_app->PostMessage(&request);
			break;
		}

		// Save file menu was selected
		case MW_SAVE_MESSAGEFILE:
		case MW_SAVE_MESSAGEFILE_AS:
		{
			be_app->PostMessage(msg);
			break;
		}

		// Close a currently opened file
		case MW_CLOSE_MESSAGEFILE:
		{
			// Ask for confirmation when there are pending changes
			if(fUnsaved) {
				BString saveOrDiscard(B_TRANSLATE("The message data was changed but not saved. "
				"Do you really want to close the file? Any unsaved changes will be lost."));
				if(!continue_action(saveOrDiscard, notsaved_alert_cancel,
				notsaved_alert_close))
					break;
			}
			be_app->PostMessage(msg);
			break;
		}

		//message file was supplied via file dialog or drag&drop
		case MW_REF_MESSAGEFILE:
		{
			//get filename from file ref
			entry_ref ref;
			msg->FindRef("refs", &ref);

			BMessage inspect_message(MW_INSPECTMESSAGEFILE);
			inspect_message.AddRef("msgfile",&ref);
			be_app->PostMessage(&inspect_message);

			break;
		}

		//get back the data message from the app object
		case MW_OPEN_REPLY:
		{
			bool open_success;
			msg->FindBool("success", &open_success);

			fMessageInfoView->Clear();

			if (open_success)
			{
				void *data_msg_pointer;
				msg->FindPointer("data_msg_pointer", &data_msg_pointer);

				BMessage *data_message = static_cast<BMessage*>(data_msg_pointer);
				fMessageInfoView->SetDataMessage(data_message);
				fTopMenuBar->FindItem(MW_RELOAD_FROM_FILE)->SetEnabled(true);
				fTopMenuBar->FindItem(MW_CLOSE_MESSAGEFILE)->SetEnabled(true);

				// Set the window's title with the file path (if it was sent)
				BString appTitle(kAppName), filePath;
				if(msg->FindString("filePath", &filePath) == B_OK)
					appTitle << ": " << filePath;
				SetTitle(appTitle);
			}
			else
			{
				const char *error_text;
				msg->FindString("error_text", &error_text);
				BAlert *message_open_alert = new BAlert("Kottan",
												error_text,
												"OK");
				message_open_alert->Go();
			}

			break;
		}

		// Reply after the file was closed
		case MW_CLOSE_REPLY:
		{
			bool success = msg->GetBool("success", false);
			if(success) {
				fUnsaved = false;

				// Update controls
				fMessageInfoView->Clear();

				// Reset title
				SetTitle(kAppName);

				// Reset menus
				fTopMenuBar->FindItem(MW_SAVE_MESSAGEFILE)->SetEnabled(false);
				fTopMenuBar->FindItem(MW_RELOAD_FROM_FILE)->SetEnabled(false);
				fTopMenuBar->FindItem(MW_CLOSE_MESSAGEFILE)->SetEnabled(false);
			}
			break;
		}

		case MV_ROW_CLICKED: // Data member was double clicked
		{
			BRow *selected_row = fMessageInfoView->CurrentSelection();


			BStringField *type_field = static_cast<BStringField*>(selected_row->GetField(2));
			if (type_field == NULL)
			{
				break;
			}

			BString selected_row_typename(type_field->String());
			if (selected_row_typename != "B_MESSAGE_TYPE")
			{
				//get index path to data of selected field
				BMessage selection_path_msg(MW_ROW_SELECTED);
				BRow *parent_row;
				BRow *current_row = selected_row;

				while (fMessageInfoView->FindParent(current_row, &parent_row, NULL))
				{
					int32 field_index =
									static_cast<BIntegerField*>(current_row->GetField(0))->Value();

					selection_path_msg.AddInt32("selection_path",field_index);
					current_row = parent_row;
				}

				int32 top_parent_index =
									static_cast<BIntegerField*>(current_row->GetField(0))->Value();
				selection_path_msg.AddInt32("selection_path",top_parent_index);

				be_app->PostMessage(&selection_path_msg);
			}

			break;
		}

		case MV_SELECTION_CHANGED: // Data member has just been selected
		{
			BRow* selectedRow = fMessageInfoView->CurrentSelection();
			BStringField* typeField = ((BStringField*)selectedRow->GetField(2));
			if(!typeField)
				break;

			if(strcmp(typeField->String(), "B_MESSAGE_TYPE") != 0) {
				//get index path to data of selected field
				BMessage selection_path_msg(MW_ROW_SELECTED_OPEN_HERE);
				BRow *parent_row;
				BRow *current_row = selectedRow;

				while (fMessageInfoView->FindParent(current_row, &parent_row, NULL))
				{
					int32 field_index = static_cast<BIntegerField*>(current_row->GetField(0))->Value();

					selection_path_msg.AddInt32("selection_path",field_index);
					current_row = parent_row;
				}

				int32 top_parent_index = static_cast<BIntegerField*>(current_row->GetField(0))->Value();
				selection_path_msg.AddInt32("selection_path",top_parent_index);
				selection_path_msg.AddPointer("target", fDataView);

				be_app->PostMessage(&selection_path_msg);
			}
			else {
				fDataView->Clear();
			}
			break;
		}

		case MW_WAS_EDITED:
		{
			switch_unsaved_state(true);

			break;
		}

		case MW_WAS_SAVED:
		{
			BString filePath;
			if(msg->FindString("filePath", &filePath) == B_OK) {
				// Set the window's title with the file path
				// fprintf(stderr, "Title is %s.\n", filePath.String());
				BString appTitle(kAppName);
				appTitle << ": " << filePath.String();
				SetTitle(appTitle);
			}
			switch_unsaved_state(false);
			break;
		}

		//message file was changed
		case MW_CONFIRM_RELOAD:
		{
			if (continue_action(
				B_TRANSLATE("The message file has changed. Do you want to reload it?"),
				B_TRANSLATE("Cancel"),
				B_TRANSLATE("Reload")))
			{
				//trigger reload
				PostMessage(MW_RELOAD_FROM_FILE);
			}

			break;
		}

		// notify App() to reload the message from file
		case MW_RELOAD_FROM_FILE:
		{
			if (fUnsaved)
			{
				if (!continue_action(
					B_TRANSLATE("The message data was changed but not saved. Do you really want to reload?"),
					B_TRANSLATE("Cancel"),
					B_TRANSLATE("Reload")))
				{
					break;
				}
			}
			be_app->PostMessage(msg);

			break;
		}

		// update MessageView with newly loaded data
		case MW_UPDATE_MESSAGEVIEW:
		{
			fMessageInfoView->UpdateData();
			fDataView->Clear();
			switch_unsaved_state(false);
			break;
		}

		// Add an entry of type...
		case MW_ADD_AFFINE_TX:
		case MW_ADD_ALIGNMENT:
		case MW_ADD_BOOL:
		case MW_ADD_COLOR:
		case MW_ADD_INT8:
		case MW_ADD_INT16:
		case MW_ADD_INT32:
		case MW_ADD_INT64:
		case MW_ADD_UINT8:
		case MW_ADD_UINT16:
		case MW_ADD_UINT32:
		case MW_ADD_UINT64:
		case MW_ADD_OFF_T:
		case MW_ADD_SIZE_T:
		case MW_ADD_SSIZE_T:
		case MW_ADD_FLOAT:
		case MW_ADD_DOUBLE:
		case MW_ADD_ENTRY_REF:
		case MW_ADD_NODE_REF:
		case MW_ADD_STRING:
		case MW_ADD_POINT:
		case MW_ADD_SIZE:
		case MW_ADD_RECT:
		{
			BMessage editorData(MW_CREATE_ENTRY_REQUESTED);
			editorData.AddBool("create", true);
			editorData.AddUInt32(KottanFieldType, static_cast<uint32>(TypeCodeForCommand(msg->what)));
			be_app->PostMessage(&editorData);
			break;
		}

		case MW_IMPORT_MESSAGE:
		{
			ImporterWindow* window = new ImporterWindow(BRect());
			window->CenterIn(Frame());
			window->Show();
			break;
		}

		case MW_MESSAGE_OPEN_SET_WHAT_DIALOG:
			be_app->PostMessage(MW_MESSAGE_OPEN_SET_WHAT_DIALOG);
			break;

		case MW_MESSAGE_MAKE_EMPTY:
		{
			if(continue_action(B_TRANSLATE("Do you want to delete all the entries of this message?"),
			notsaved_alert_cancel, B_TRANSLATE("Delete all")))
				be_app->PostMessage(MW_MESSAGE_MAKE_EMPTY);
			break;
		}

		// Call to summon a EditWindow from the MainWindow-owned DataView
		case DW_ROW_CLICKED:
		{
			BMessage request(msg->what);
			request.AddInt32(KottanFieldIndex, msg->GetInt32(KottanFieldIndex, 0));
			request.AddString(KottanFieldName, msg->GetString(KottanFieldName));
			request.AddUInt32(KottanFieldType, msg->GetUInt32(KottanFieldType, B_ANY_TYPE));
			request.AddPointer("window", this);
			be_app->PostMessage(&request);
			break;
		}

		case DW_ROW_REMOVE_REQUESTED:
		{
			if((new BAlert("", B_TRANSLATE("Do you want to remove this item?"),
			B_TRANSLATE("Remove"), B_TRANSLATE("Keep")))->Go() == 0) {
				be_app->PostMessage(msg);
				// Quit();
			}
			break;
		}

		case MW_DATA_PANEL_VISIBLE:
		case DW_BUTTON_CLOSE:
		{
			ToggleDataViewVisibility();
			break;
		}

		//do nothing and don´t forward to base class
		case MW_DO_NOTHING:
			break;

		//forward all unhandled messages to the base class
		default:
		{
			BWindow::MessageReceived(msg);
			break;
		}
	}

}


bool
MainWindow::QuitRequested()
{

	if (fUnsaved)
	{
		if (!continue_action(
			B_TRANSLATE("The message data was changed but not saved. Do you really want to quit?"),
			B_TRANSLATE("Cancel"),
			B_TRANSLATE("Quit")))
		{
			return false;
		}
	}

	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;

}


bool
MainWindow::continue_action(const char *alert_text,
							const char *button_label_cancel,
							const char *button_label_continue)
{
	BAlert *not_saved_alert = new BAlert(
		"",
		alert_text,
		button_label_cancel,
		button_label_continue,
		NULL,
		B_WIDTH_AS_USUAL,
		B_WARNING_ALERT);

	if (not_saved_alert->Go() == 0)
	{
		return false;
	}

	return true;

}


void
MainWindow::switch_unsaved_state(bool unsaved_state)
{

	fTopMenuBar->FindItem(MW_SAVE_MESSAGEFILE)->SetEnabled(unsaved_state);

	if (unsaved_state)
	{
		if(!fUnsaved)  // do nothing if unsaved state is already set
		{
			BString title(Title());
			title.Prepend("*");
			SetTitle(title.String());

			fUnsaved = true;
		}
	}
	else
	{
		if (fUnsaved)
		{
			BString title(Title());
			title.RemoveChars(0,1);
			SetTitle(title.String());

			fUnsaved = false;
		}
	}

}

void
MainWindow::ToggleDataViewVisibility()
{
	if(fDataView->IsHidden()) {
		fTopMenuBar->FindItem(MW_DATA_PANEL_VISIBLE)->SetMarked(true);
		fDataView->Show();
	}
	else  {
		fTopMenuBar->FindItem(MW_DATA_PANEL_VISIBLE)->SetMarked(false);
		fDataView->Hide();
	}
}

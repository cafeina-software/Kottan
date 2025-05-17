/*
 * Copyright 2019-2021 Andi Machovec <andi.machovec@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 *
 */

#include "app.h"
#include "importerwindow.h"
#include "kottandefs.h"
#include "mainwindow.h"
#include "datawindow.h"
#include "editwindow.h"
#include "msginfowindow.h"
#include "whatwindow.h"

#include <AboutWindow.h>
#include <Catalog.h>
#include <Resources.h>
#include <AppFileInfo.h>
#include <Path.h>
#include <File.h>
#include <IconUtils.h>
#include <FindDirectory.h>
#include <NodeMonitor.h>
#include <Roster.h>
#include <stdio.h>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "App"


const char* kAppName = "Kottan";
const char* kAppSignature = "application/x-vnd.BlueSky-Kottan";
BBitmap* trashIcon;
BBitmap* removeIcon;

App::App()
	:
	BApplication(kAppSignature)

{
	fDataMessage = new BMessage();
	fMessageList = new BObjectList<IndexMessage>(20, false);
	fMessageFile = new BFile();
	fDataWindow = NULL;

	/* File panels stuff */
	BPath userDirectoryPath;
	find_directory(B_USER_DIRECTORY, &userDirectoryPath);
	BEntry userDirectoryEntry(userDirectoryPath.Path());
	entry_ref userDirectoryRef;
	userDirectoryEntry.GetRef(&userDirectoryRef);
	fGenericFilter = new GenericFileFilter;
	fMessageFilter = new MessageFileFilter;
	fOpenPanel = new BFilePanel(B_OPEN_PANEL, NULL, &userDirectoryRef, B_FILE_NODE, false, NULL, NULL);
	fSavePanel = new BFilePanel(B_SAVE_PANEL, NULL, &userDirectoryRef, B_FILE_NODE, false, NULL, NULL);

	/* Init shared resources */
	InitSharedResources();
}


App::~App()
{
	if(fVisualWindow && fVisualWindow->IsLocked())
		fVisualWindow->Quit();
	if(fDataWindow && fDataWindow->IsLocked())
		fDataWindow->Quit();
	if(fMainWindow && fMainWindow->IsLocked())
		fMainWindow->Quit();

	delete fDataMessage;
	delete fMessageFile;
	delete fOpenPanel;
	delete fSavePanel;
	delete fGenericFilter;
	delete fMessageFilter;

	FreeSharedResources();
}


void
App::MessageReceived(BMessage *msg)
{

	switch(msg->what)
	{
		// Open panel requested to open a file
		case MW_OPEN_MESSAGEFILE:
		{
			BMessenger messenger;  // The result will be delivered to it
			msg->FindMessenger(KottanFieldMsgr, &messenger);
			ShowFilePanel(fOpenPanel, &messenger, new BMessage(MW_REF_MESSAGEFILE), fMessageFilter);
			break;
		}

		//receive file reference, read the file and extract into message
		case MW_INSPECTMESSAGEFILE:
		{

			stop_watching(be_app_messenger); //stop watching file nodes

			msg->FindRef("msgfile", &fMessageFileRef);

			status_t fileopen_result = fMessageFile->SetTo(&fMessageFileRef, B_READ_ONLY);

			BString error_text;
			bool message_read_success = false;

			if (fileopen_result == B_OK)
			{
				status_t unflatten_result = fDataMessage->Unflatten(fMessageFile);

				if (unflatten_result == B_OK)
				{
					message_read_success=true;
				}
				else
				{
					error_text = B_TRANSLATE("Error reading the message from the file!");
				}
			}
			else
			{
				error_text = B_TRANSLATE("Error opening the message file!");
			}

			BMessage open_reply_msg(MW_OPEN_REPLY);
			open_reply_msg.AddBool("success", message_read_success);

			if (message_read_success)
			{
				open_reply_msg.AddPointer("data_msg_pointer", fDataMessage);

				// start watching the file for changes
				BEntry entry(&fMessageFileRef);
				node_ref nref;
				entry.GetNodeRef(&nref);
				watch_node(&nref, B_WATCH_STAT|B_WATCH_INTERIM_STAT, be_app_messenger);

				// add the file path to set the title with it
				BPath filePath;
				entry.GetPath(&filePath);
				open_reply_msg.AddString("filePath", filePath.Path());
			}
			else
			{
				open_reply_msg.AddString("error_text", error_text.String());
			}

			fMainWindow->PostMessage(&open_reply_msg);

			break;
		}

		// get info about clicked row and open data window
		case MW_ROW_SELECTED:
		{
			get_selection_data(msg);

			BMessage *dw_message;

			if (fMessageList->CountItems() > 0)
			{
				dw_message = fMessageList->FirstItem()->message;
			}
			else
			{
				dw_message = fDataMessage;
			}

			fDataWindow = new DataWindow(BRect(0,0,400,300),
										dw_message,
										fSelectedName,
										fSelectedType,
								        fSelectedItemCount);

			fDataWindow->CenterIn(fMainWindow->Frame());
			fDataWindow->MoveBy(0, 128);
			fDataWindow->MoveOnScreen(B_MOVE_IF_PARTIALLY_OFFSCREEN);
			//fDataWindow->AddToSubset(fMainWindow);
			fDataWindow->Show();

			break;
		}

		// Get info about clicked row inside the window (data panel)
		case MW_ROW_SELECTED_OPEN_HERE:
		{
			get_selection_data(msg);

			BMessage *dw_message;

			if (fMessageList->CountItems() > 0)
			{
				dw_message = fMessageList->FirstItem()->message;
			}
			else
			{
				dw_message = fDataMessage;
			}

			DataView* view = (DataView*)msg->GetPointer("target");
			if(view) {
				if(!fSelectedName || strlen(fSelectedName) == 0)
					fSelectedName = msg->GetString(KottanFieldName);
				view->SetTo(dw_message, fSelectedName, fSelectedType, fSelectedItemCount);
			}

			break;
		}

		// get info about clicked row in DataWindow and open EditWindow
		case DW_ROW_CLICKED:
		{
			int32 field_index;
			msg->FindInt32(KottanFieldIndex, &field_index);

			BMessage *ew_message;

			if (fMessageList->CountItems() > 0)
			{
				ew_message = fMessageList->FirstItem()->message;
			}
			else
			{
				ew_message = fDataMessage;
			}

			BWindow* window = (BWindow*)msg->GetPointer("window");
			EditWindow *edit_window = new EditWindow(BRect(0,0,0,0), ew_message,
				fSelectedType, fSelectedName, field_index, false, window);

			// Get the window to position the editor window.
			// The positioning fails if we derive the window from the looper
			// using target.Target(&looper).
			if(window)
				edit_window->CenterIn(window->Frame());
			fDataWindow->SetFeel(B_NORMAL_WINDOW_FEEL);
			//edit_window->AddToSubset(fDataWindow);
			edit_window->Show();

			break;
		}

		case DW_ROW_REMOVE_REQUESTED:
		{
			int32 field_index;
			BString field_name;
			type_code field_type;
			if(msg->FindInt32(KottanFieldIndex, &field_index) == B_OK &&
			msg->FindString(KottanFieldName, &field_name) == B_OK &&
			msg->FindUInt32(KottanFieldType, static_cast<uint32*>(&field_type)) == B_OK) {
				fDataMessage->RemoveData(field_name, field_index);
				fMainWindow->PostMessage(MW_UPDATE_MESSAGEVIEW); // Update message view
				fMainWindow->PostMessage(MW_WAS_EDITED); // Mark title bar as "has pending changes"
			}

			break;
		}

		// Opens the EditWindow to create a new entry (rather than editing an existing one)
		case MW_CREATE_ENTRY_REQUESTED:
		{
			type_code type;
			msg->FindUInt32(KottanFieldType, static_cast<uint32*>(&type));
			bool creationFlag = msg->GetBool("create");

			EditWindow* editorWindow = new EditWindow(BRect(), fDataMessage,
				type, "" /* Ignored */, -1 /* Ignored */, creationFlag,
				fMainWindow);
			editorWindow->CenterIn(fMainWindow->Frame());
			editorWindow->Show();
			break;
		}

		// save edited data to the main data message
		case EW_BUTTON_SAVE:
		{
			fDataWindow->SetFeel(B_MODAL_APP_WINDOW_FEEL);

			if (fMessageList->CountItems() > 0)
			{
				for( int32 i = 1; i < fMessageList->CountItems(); ++i)
				{
					fMessageList->ItemAt(i)->message->ReplaceMessage(
														fMessageList->ItemAt(i-1)->field_name,
														fMessageList->ItemAt(i-1)->field_index,
														fMessageList->ItemAt(i-1)->message);
				}

				fDataMessage->ReplaceMessage(fMessageList->LastItem()->field_name,
											fMessageList->LastItem()->field_index,
											fMessageList->LastItem()->message);
			}

			void* target = NULL;
			if(msg->FindPointer("target", &target) == B_OK) {// Call to update views
				static_cast<BWindow*>(target)->PostMessage(DW_UPDATE); // Either main window or a data window
			}

			if(msg->GetBool(KottanFlagCreate)) // If creation mode, update main msg view
				fMainWindow->PostMessage(MW_UPDATE_MESSAGEVIEW);
			fMainWindow->PostMessage(MW_WAS_EDITED); // Mark window title as modified
			break;
		}

		// edit window closed, make data window modal again
		case EW_BUTTON_CANCEL:
		{
			fDataWindow->SetFeel(B_MODAL_APP_WINDOW_FEEL);
			break;
		}


		// save message data to file
		case MW_SAVE_MESSAGEFILE:
		{
			if(!HasFile()) {
				PostMessage(MW_SAVE_MESSAGEFILE_AS);
				break;
			}

			fMessageFile->SetTo(&fMessageFileRef, B_WRITE_ONLY|B_ERASE_FILE);
			fDataMessage->Flatten(fMessageFile);
			fMainWindow->PostMessage(MW_WAS_SAVED);
			break;
		}

		case MW_SAVE_MESSAGEFILE_AS:
		{
			BMessenger messenger(this);
			ShowFilePanel(fSavePanel, &messenger, new BMessage(B_SAVE_REQUESTED), fMessageFilter);
			break;
		}

		case B_SAVE_REQUESTED:
		{
			entry_ref directoryRef;
			BString name;

			if(msg->FindRef("directory", &directoryRef) != B_OK ||
			msg->FindString("name", &name) != B_OK) {
				break;
			}

			BDirectory directory(&directoryRef);
			BEntry fileEntry(&directory, name.String());

			uint32 fileFlags = B_WRITE_ONLY | B_ERASE_FILE;
			if(!fileEntry.Exists())
				fileFlags |= B_CREATE_FILE;
			BFile newFile(&directory, name.String(), fileFlags);
			if(newFile.InitCheck() != B_OK) {
				break;
			}

			status_t result = fDataMessage->Flatten(&newFile);
			if(result == B_OK) { // On success...
				// update data members
				fMessageFile->Unset();
				fMessageFile->SetTo(&directory, name.String(), B_READ_ONLY);
				fileEntry.GetRef(&fMessageFileRef);

				// update monitoring target
				node_ref nref;
				fileEntry.GetNodeRef(&nref);
				watch_node(&nref, B_WATCH_STAT | B_WATCH_INTERIM_STAT, be_app_messenger);

				// send notification to window
				BPath entryPath(&directory, name.String());
				BMessage reply(MW_WAS_SAVED);
				reply.AddString("filePath", entryPath.Path());
				fMainWindow->PostMessage(&reply);
			}
			break;
		}

		// Close file requested
		case MW_CLOSE_MESSAGEFILE:
		{
			// Stop watching the node of the file closed
			node_ref nref;
			BEntry(&fMessageFileRef).GetNodeRef(&nref);
			watch_node(&nref, B_STOP_WATCHING, be_app_messenger);

			// Update data
			fMessageFile->Unset();
			fDataMessage->MakeEmpty();

			// Notify the window
			BMessage reply(MW_CLOSE_REPLY);
			reply.AddBool("success", true);
			fMainWindow->PostMessage(&reply);
			break;
		}

		// triggered by the node monitor if one of the stat parameters changes
		case B_NODE_MONITOR:
		{
			int32 stat_changed_flags;
			msg->FindInt32("fields", &stat_changed_flags);

			// only compare messages when the file was modified
			if ((stat_changed_flags & B_STAT_MODIFICATION_TIME) != 0)
			{
				BMessage *temp_msg = new BMessage();
				fMessageFile->SetTo(&fMessageFileRef, B_READ_ONLY);
				temp_msg->Unflatten(fMessageFile);

				//only request reload if the data in the message has actually changed
				if (!(temp_msg->HasSameData(*fDataMessage, false, true)))
				{
					fMainWindow->PostMessage(MW_CONFIRM_RELOAD);
				}

				delete temp_msg;
			}

			break;
		}

		// reload message data from file and update main and data window
		case MW_RELOAD_FROM_FILE:
		{
			fMessageFile->SetTo(&fMessageFileRef, B_READ_ONLY);
			fDataMessage->Unflatten(fMessageFile);

			fMainWindow->PostMessage(MW_UPDATE_MESSAGEVIEW);

			if (fDataWindow != NULL)
			{
				fDataWindow->PostMessage(DW_UPDATE);
			}

			break;
		}

		// Opens the dialog to change the message type ('what')
		case MW_MESSAGE_OPEN_SET_WHAT_DIALOG:
		{
			WhatWindow* whatwnd = new WhatWindow(BRect(), fDataMessage);
			whatwnd->CenterIn(fMainWindow->Frame());
			whatwnd->Show();
			break;
		}

		// Applies the change of the message type ('what')
		case WCMD_SAVE_WHAT_REQUESTED:
		{
			uint32 what = 0;
			if(msg->FindUInt32("what", &what) == B_OK) {
				fDataMessage->what = what;
				fMainWindow->PostMessage(MW_WAS_EDITED);
			}
			break;
		}

		// Deletes all the data members of the message
		case MW_MESSAGE_MAKE_EMPTY:
		{
			// Immediate effect;
			fDataMessage->MakeEmpty();
			if(fMessageList->CountItems() > 0)
				fMessageList->MakeEmpty();

			fMainWindow->PostMessage(MW_UPDATE_MESSAGEVIEW); // Update message view
			fMainWindow->PostMessage(MW_WAS_EDITED); // Mark title bar as "has pending changes"
			if(fDataWindow)
				fDataWindow->Close();
			break;
		}

		// Called to open a message information dialog box of the current message
		case MW_MESSAGE_INFORMATION:
		{
			MsgInfoWindow* window = new MsgInfoWindow(BRect(), fDataMessage);
			window->CenterIn(fMainWindow->Frame());
			window->Show();
			break;
		}

		// Used by the importer dialog box to call an open panel
		case IMP_OPEN_REQUESTED:
		{
			BMessenger messenger;
			msg->FindMessenger("target", &messenger);
			ShowFilePanel(fOpenPanel, &messenger, new BMessage(IMP_OPEN_REPLY), fMessageFilter);
			fOpenPanel->Window()->CenterIn(fMainWindow->Frame());
			break;
		}

		// The request for import a message was received, it will be dealt here
		case IMP_SAVE_REQUESTED:
		{
			entry_ref ref;
			if(msg->FindRef("refs", &ref) == B_OK && BEntry(&ref).Exists()) {
				bool memberMode = msg->GetBool(KottanFlagImportMember);
				BFile file(&ref, B_READ_ONLY);
				if(file.InitCheck() != B_OK)
					break;
				BMessage message;
				message.Unflatten(&file);

				const void* data = NULL;
				if(memberMode)
					data = (const void*)msg->GetString(KottanFieldName);

				status_t result = ImportMessage(&message, memberMode, data);
				if(result == B_OK) { // Call to update UI on success
					fMainWindow->PostMessage(MW_UPDATE_MESSAGEVIEW); // Update message view
					fMainWindow->PostMessage(MW_WAS_EDITED); // Mark title bar as "has pending changes"
				}
			}
			break;
		}

		// Request to call the Open Panel to retrieve a file entry
		case EV_REF_REQUESTED:
		{
			BMessenger messenger;
			msg->FindMessenger(KottanFieldMsgr, &messenger);
			ShowFilePanel(fOpenPanel, &messenger, new BMessage(EW_REFS_RECEIVED), fGenericFilter);
			break;
		}

		// Perform a request to the Visual Window
		case EV_MEASUREMENT_REQUESTED:
		{
			type_code type = static_cast<type_code>(msg->GetUInt32(KottanFieldType, B_ANY_TYPE));

			BPoint point;
			BSize size;
			BRect rect = fMainWindow->Frame();
			if(type == B_POINT_TYPE)
				point = fDataMessage->GetPoint(
					msg->GetString(KottanFieldName),
					msg->GetInt32(KottanFieldIndex, 0), BPoint());
			else if(type == B_SIZE_TYPE)
				size = fDataMessage->GetSize(msg->GetString(KottanFieldName),
					msg->GetInt32(KottanFieldIndex, 0), BSize());
			else if(type == B_RECT_TYPE)
				rect = fDataMessage->GetRect(msg->GetString(KottanFieldName),
					msg->GetInt32(KottanFieldIndex, 0), BRect());

			fVisualWindow->SetTo(rect, size, point, type, (BLooper*)msg->GetPointer("target"));
			if(fMainWindow)
				fVisualWindow->CenterIn(fMainWindow->Frame());
			fVisualWindow->Show();
			break;
		}

		// delegate all unhandled messages to the base class
		default:
		{
			BApplication::MessageReceived(msg);
			break;
		}
	}

}


void
App::AboutRequested()
{

	BAboutWindow *aboutwindow = new BAboutWindow(kAppName, kAppSignature);

	const char *authors[] =
	{
		"Andi Machovec (BlueSky)",
		NULL
	};

	BString code_contributors;
	BString icon_designers;
	BString translators;

	code_contributors << B_TRANSLATE("Code contributions by:");
	code_contributors << "\n"
					  << "Humdinger\n"
					  << "Jaidyn Ann (JadedCtrl)\n";

	icon_designers 	<< B_TRANSLATE("Icon design by:");
	icon_designers 	<< "\n"
					<< "zuMi\n";

	translators << B_TRANSLATE("Translations by:");
	translators << "\n"
				<< "Begasus\n"
				<< "Alex Hitech\n"
				<< "Davidmp\n"
				<< "Florentina Mușat\n"
				<< "cafeina\n"
				<< "Fredrik Modéen\n"
				<< "Emir Sarı\n"
				<< "Amor\n"
				<< "Humdinger\n"
				<< "jt15s\n"
				<< "munecito\n"
				<< "Briseur\n"
				<< "Loïc\n"
				<< "tmtfx\n"
				<< "FaBE\n"
				<< "mazbrili\n"
				<< "zumikebbe\n"
				<< "Johan Wagenheim\n"
				<< "Victor Domingos\n"
				<< "al-popa\n"
				<< "itvanya\n"
				<< "butyoutried\n"
				<< "viktroniko\n";

	BString extra_info;
	extra_info.Append(code_contributors);
	extra_info.Append("\n");
	extra_info.Append(icon_designers);
	extra_info.Append("\n");
	extra_info.Append(translators);

	BResources *appresource = BApplication::AppResources();
	size_t size;
	version_info *appversion = (version_info *)appresource->LoadResource('APPV',1,&size);
	BString version_string;
	version_string<<appversion->major;
	version_string+=".";
	version_string<<appversion->middle;
	version_string+=".";
	version_string<<appversion->minor;

	aboutwindow->AddCopyright(2019, "Andi Machovec");
	aboutwindow->AddAuthors(authors);
	aboutwindow->SetVersion(version_string.String());
	aboutwindow->AddDescription(B_TRANSLATE("An editor for archived BMessages"));
	aboutwindow->AddExtraInfo(extra_info.String());
	const char* extraCreds [] = {
		B_TRANSLATE("The icon \'R_TrashFullIcon\' is part of Haiku and licensed under MIT license."),
		B_TRANSLATE("The icon \'Action_Stop\' is part of Haiku and licensed under MIT license."),
		NULL
	};
	aboutwindow->AddText(B_TRANSLATE("Extra credits:"), extraCreds);

	if(fMainWindow)
		aboutwindow->CenterIn(fMainWindow->Frame());
	aboutwindow->Show();

}


bool
App::QuitRequested()
{

	//save window frame
	BRect mainwindow_frame = fMainWindow->Frame();

	BFile *settings_file = new BFile(fSettingsFileName, B_READ_WRITE);

	BMessage settings_message;
	settings_message.Unflatten(settings_file);
	settings_message.ReplaceRect("mainwindow_frame", mainwindow_frame);
	settings_file->Seek(0, SEEK_SET); //rewind file position to beginning
	settings_message.Flatten(settings_file);

	delete settings_file;

	return true;
}


void
App::ReadyToRun()
{

	//determine settings file name
	BPath settings_path;
	find_directory(B_USER_SETTINGS_DIRECTORY, &settings_path);
	fSettingsFileName << settings_path.Path() << "/Kottan_settings";

	//get main window frame from settings file (if there is any)
	BRect mainwindow_frame;

	BFile *settings_file = new BFile(fSettingsFileName, B_READ_WRITE|B_CREATE_FILE);
	BMessage settings_message;
	status_t unflatten_result = settings_message.Unflatten(settings_file);

	bool frame_retrieved=false;

	if (unflatten_result == B_OK)
	{
		status_t frame_result = settings_message.FindRect("mainwindow_frame",
														&mainwindow_frame);
		if (frame_result == B_OK)
		{
			frame_retrieved = true;
		}
	}

	// set default frame and add to settings message
	if (!frame_retrieved)
	{
		mainwindow_frame.Set(100,100,750,500);
		settings_message.AddRect("mainwindow_frame", mainwindow_frame);
		settings_message.Flatten(settings_file);
	}

	// create and show main window
	fMainWindow = new MainWindow(mainwindow_frame);

	if (!frame_retrieved)
	{
		fMainWindow->CenterOnScreen();
	}
	fMainWindow->Show();

	fVisualWindow = new VisualWindow(BRect(), NULL, NULL);

	delete settings_file;

}


void
App::ArgvReceived(int32 argc, char **argv)
{

	// construct file ref for the specified file
	BEntry entry(argv[1]);
	entry_ref ref;
	entry.GetRef(&ref);

	BMessage refMsg(B_REFS_RECEIVED);
	refMsg.AddRef("refs", &ref);

	RefsReceived(&refMsg);

}


void
App::RefsReceived(BMessage *msg)
{

	entry_ref ref;
	msg->FindRef("refs", &ref);

	// send inspect message to open the file
	BMessage inspect_message(MW_INSPECTMESSAGEFILE);
	inspect_message.AddRef("msgfile",&ref);
	PostMessage(&inspect_message);

}


void
App::get_selection_data(BMessage *selection_path_message)
{

	// follow the index path to the selected data
	BMessage *current_message = fDataMessage;

	fMessageList->MakeEmpty();

	int32 path_items_count;
	selection_path_message->GetInfo("selection_path", NULL, &path_items_count);

	type_code current_type;
	char *current_name;
	int32 current_item_count;

	for (int32 path_index = path_items_count-1;  // loop through the index values in
		 path_index >= 0;				   		 // in reverse order
		 --path_index)
	{
		int32 current_index;
		selection_path_message->FindInt32("selection_path", path_index, &current_index);

		current_message->GetInfo(B_ANY_TYPE,
								current_index,
								&current_name,
								&current_type,
								&current_item_count);

		if (current_type == B_MESSAGE_TYPE)
		{
			int32 member_index=0;

			if (current_item_count > 1)
			{
				--path_index;
				selection_path_message->FindInt32("selection_path", path_index, &member_index);
			}

			BMessage *temp_message = new BMessage();
			current_message->FindMessage(current_name, member_index, temp_message);

			IndexMessage *index_message = new IndexMessage();
			index_message->message = new BMessage(*temp_message);
			index_message->field_index = member_index;
			index_message->field_name = current_name;
			fMessageList->AddItem(index_message, 0);

			current_message = temp_message;
		}
	}

	fSelectedName = current_name;
	fSelectedType = current_type;
	fSelectedItemCount = current_item_count;

}

status_t
App::ImportMessage(BMessage* msg, bool memberMode, [[maybe_unused]] const void* data)
{
	if(!msg || (memberMode && !data))
		return B_BAD_VALUE;

	if(memberMode) {
		fDataMessage->AddMessage(reinterpret_cast<const char*>(data), msg);
	}
	else {
		char* name;
		type_code type;
		int32 countfound;
		for(int32 i = 0; msg->GetInfo(B_ANY_TYPE, i, &name, &type, &countfound) == B_OK; i++) {
			for(int32 j = 0; j < countfound; j++) {
				const void* data = NULL;
				ssize_t length = 0;
				msg->FindData(name, type, j, &data, &length);
				unsigned char* buffer = new unsigned char[length];
				memcpy(buffer, data, length);
				fDataMessage->AddData(name, type, buffer, length);
				delete[] buffer;
			}
		}
	}
	return B_OK;
}

// #pragma mark - App::Private

void
App::InitSharedResources()
{
	trashIcon = new BBitmap(BRect(0, 0, 31, 31), B_RGBA32);
	App::LoadIcon(1004, &trashIcon);

	// trashIcon = new BBitmap(BRect(0, 0, 31, 31), B_RGBA32);
	// entry_ref trkref;
	// be_roster->FindApp("application/x-vnd.Be-TRAK", &trkref);
	// BFile trackerFile(&trkref, B_READ_ONLY);
	// BResources trackerRes(&trackerFile, false);
	// size_t bitmapSize = 0;
	// const void* bitmapData = trackerRes.LoadResource(B_VECTOR_ICON_TYPE, 1004, &bitmapSize);
	// BIconUtils::GetVectorIcon(reinterpret_cast<const uint8*>(bitmapData), bitmapSize, trashIcon);

	removeIcon = new BBitmap(BRect(0, 0, 31, 31), B_RGBA32);
	App::LoadIcon(138, &removeIcon);
}

void
App::FreeSharedResources()
{
	delete trashIcon;
	delete removeIcon;
}

void
App::LoadIcon(int32 id, BBitmap** outBitmap)
{
	BResources* resources = BApplication::AppResources();
	size_t bitmapSize = 0;
	const void* bitmapData = resources->LoadResource(B_VECTOR_ICON_TYPE, id, &bitmapSize);
	BIconUtils::GetVectorIcon(reinterpret_cast<const uint8*>(bitmapData), bitmapSize, *outBitmap);
}

void
App::ShowFilePanel(BFilePanel* panel, BMessenger* target, BMessage* message, BRefFilter* refFilter)
{
	panel->SetTarget(*target);
	panel->SetMessage(message);
	panel->SetRefFilter(refFilter);
	panel->Show();
}

// #pragma mark - main

int
main(int argc, char** argv)
{

	App *haiku_app = new App();
	haiku_app->Run();

	delete haiku_app;
	return 0;

}

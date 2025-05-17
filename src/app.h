/*
 * Copyright 2019-2021 Andi Machovec <andi.machovec@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 *
 */

#ifndef APP_H
#define APP_H

#include "visualwindow.h"
#include <Application.h>
#include <FilePanel.h>
#include <Message.h>
#include <ObjectList.h>
#include <File.h>
#include <String.h>


class DataWindow;
class MainWindow;

extern const char* kAppName;
extern BBitmap* trashIcon;
extern BBitmap* removeIcon;

class IndexMessage {
public:
	BMessage 	*message;
	int32		field_index;
	const char  *field_name;
};

class GenericFileFilter : public BRefFilter
{
public:
	virtual bool Filter(const entry_ref*, BNode*, struct stat_beos*, const char*) {
		return true;
	}
};

class MessageFileFilter : public BRefFilter
{
public:
	virtual bool Filter(const entry_ref* ref, BNode* node,
	struct stat_beos* stat, const char* mimeType) {
		return 	node->IsDirectory() ||
				IsFileFlattenedMessage(*ref);
	}
private:
	bool IsFileFlattenedMessage(entry_ref ref) {
		BFile file(&ref, B_READ_ONLY);
		if(file.InitCheck() != B_OK)
			return false;

		BMessage* data = new BMessage;
		if(!data) // It's not like the file is not a flattened message...
			return false; // but there was a memory issue
		status_t status = data->Unflatten(&file);

		file.Unset();
		delete data;
		return status == B_OK;
	}
};

class App : public BApplication {
public:
	App();
	~App();
	void MessageReceived(BMessage *msg);
	void AboutRequested();
	bool QuitRequested();
	void ReadyToRun();
	void ArgvReceived(int32 argc, char **argv);
	void RefsReceived(BMessage* msg);

	bool HasFile() { return fMessageFile->InitCheck() == B_OK; }
private:
		void		InitSharedResources();
		void		FreeSharedResources();
		static void LoadIcon(int32 id, BBitmap** outBitmap);

		void 		get_selection_data(BMessage *selection_path_message);
		status_t 	ImportMessage(BMessage* msg, bool memberMode,
						[[maybe_unused]] const void* data);
		void 		ShowFilePanel(BFilePanel* panel, BMessenger* target,
						BMessage* message, BRefFilter* refFilter);

		MainWindow					*fMainWindow;
		DataWindow					*fDataWindow;
		VisualWindow				*fVisualWindow;
		BFilePanel					*fOpenPanel;
		BFilePanel					*fSavePanel;

		BMessage					*fDataMessage;
		BObjectList<IndexMessage>	*fMessageList;
		BFile						*fMessageFile;
		entry_ref					fMessageFileRef;
		BString						fSettingsFileName;

		GenericFileFilter			*fGenericFilter;
		MessageFileFilter			*fMessageFilter;

		const char 					*fSelectedName;
		type_code 					fSelectedType;
		int32 						fSelectedItemCount;
};

#endif


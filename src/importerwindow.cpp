/*
 * Copyright 2025 Cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 *
 */
#include <Box.h>
#include <Button.h>
#include <Catalog.h>
#include <FilePanel.h>
#include <LayoutBuilder.h>
#include <Path.h>
#include <RadioButton.h>
#include <stdio.h>
#include "app.h"
#include "importerwindow.h"
#include "kottandefs.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ImporterWindow"

ImporterWindow::ImporterWindow(BRect frame)
: BWindow(frame, B_TRANSLATE("Message importer"), B_DOCUMENT_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL,
	B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS)
{
	fTcPathString = new BTextControl("", "", new BMessage(IMP_PATH_MODIFIED));
	fTcPathString->SetModificationMessage(new BMessage(IMP_PATH_MODIFIED));
	fBtBrowse = new BButton(B_TRANSLATE("Browse"), new BMessage(IMP_OPEN_REQUESTED));
	fTcMessageName = new BTextControl(B_TRANSLATE("Name"), "", new BMessage(IMP_NAME_MODIFIED));
	fTcMessageName->SetModificationMessage(new BMessage(IMP_NAME_MODIFIED));
	fRbModeMember = new BRadioButton(B_TRANSLATE("Import as a member object"), new BMessage(IMP_MODE_MEMBER));
	fRbModeMember->SetValue(B_CONTROL_ON);
	fRbModeContents = new BRadioButton(B_TRANSLATE("Import only its data members"), new BMessage(IMP_MODE_CONTENTS_ONLY));
	fBtSave = new BButton(B_TRANSLATE("Import"), new BMessage(IMP_SAVE_REQUESTED));
	fBtSave->SetEnabled(CanImport());
	fBtCancel = new BButton(B_TRANSLATE("Cancel"), new BMessage(IMP_CLOSE_REQUESTED));

	BView* childView = new BView(NULL, B_SUPPORTS_LAYOUT);
	BLayoutBuilder::Group<>(childView, B_VERTICAL)
		.SetInsets(B_USE_SMALL_INSETS, 0, B_USE_SMALL_INSETS, B_USE_SMALL_INSETS)
		.Add(fRbModeMember)
		.AddGroup(B_HORIZONTAL)
			.AddStrut(4.0f)
			.Add(fTcMessageName)
		.End()
		.Add(fRbModeContents)
	.End();

	BBox* childBox = new BBox("");
	childBox->SetLabel(B_TRANSLATE("Import mode"));
	childBox->AddChild(childView);

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_SMALL_INSETS)
		.AddGroup(B_HORIZONTAL)
			.Add(fTcPathString)
			.Add(fBtBrowse)
		.End()
		.Add(childBox)
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(fBtSave)
			.Add(fBtCancel)
			.AddGlue()
		.End()
	.End();
}


void
ImporterWindow::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
		case IMP_MODE_MEMBER:
		case IMP_MODE_CONTENTS_ONLY:
			fTcMessageName->SetEnabled(msg->what == IMP_MODE_MEMBER);
			[[fallthrough]];
		case IMP_PATH_MODIFIED:
		case IMP_NAME_MODIFIED:
			fBtSave->SetEnabled(CanImport());
			break;
		case IMP_OPEN_REQUESTED:
		{
			BMessage request(msg->what);
			request.AddMessenger("target", this);
			be_app->PostMessage(&request);
			Hide();
			break;
		}
		case B_CANCEL:
			Show();
			break;
		case IMP_OPEN_REPLY:
		{
			if(IsHidden())
				Show();

			entry_ref ref;
			if(msg->FindRef("refs", &ref) != B_OK)
				break;
			BPath path;
			BEntry(&ref).GetPath(&path);
			fTcPathString->SetText(path.Path());
			break;
		}
		case IMP_SAVE_REQUESTED:
		{
			BMessage request(msg->what);
			entry_ref ref;
			BEntry(fTcPathString->Text()).GetRef(&ref);
			request.AddRef("refs", &ref);
			request.AddBool("import_as_member", fRbModeMember->Value() == B_CONTROL_ON);
			if(fRbModeMember->Value() == B_CONTROL_ON)
				request.AddString(KottanFieldName, fTcMessageName->Text());
			be_app->PostMessage(&request);
			Quit();
			break;
		}
		case IMP_CLOSE_REQUESTED:
			Quit();
			break;
		default:
			return BWindow::MessageReceived(msg);
	}
}

bool
ImporterWindow::CanImport()
{
	if(fTcPathString->TextLength() == 0)
		return false;

	BEntry fileEntry(fTcPathString->Text());

	if(fRbModeMember->Value() == B_CONTROL_ON)
		return fileEntry.Exists() && fileEntry.IsFile() && fTcMessageName->TextLength() > 0;
	else
		return fileEntry.Exists() && fileEntry.IsFile();
}

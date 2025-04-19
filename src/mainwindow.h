/*
 * Copyright 2019-2021 Andi Machovec <andi.machovec@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 *
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <File.h>
#include <Window.h>
#include <MenuBar.h>
#include <FilePanel.h>

#include "datawindow.h"
#include "messageview.h"


enum
{
	MW_MENU_ABOUT ='mw00',
	MW_OPEN_MESSAGEFILE,
	MW_SAVE_MESSAGEFILE,
	MW_SAVE_MESSAGEFILE_AS,
	MW_REF_MESSAGEFILE,
	MW_REF_MESSAGEFILE_IMPORT,
	MW_ENTERED_MESSAGEFILE,
	MW_INSPECTMESSAGEFILE,
	MW_OPEN_REPLY,
	MW_ROW_SELECTED,
	MW_ROW_SELECTED_OPEN_HERE,
	MW_WAS_EDITED,
	MW_WAS_SAVED,
	MW_DO_NOTHING,
	MW_RELOAD_FROM_FILE,
	MW_CONFIRM_RELOAD,
	MW_UPDATE_MESSAGEVIEW,
	MW_CLOSE_MESSAGEFILE,
	MW_CLOSE_REPLY,
	MW_CREATE_ENTRY_REQUESTED,
	MW_CREATE_ENTRY_REPLY,

	/* Message -> Add X commands */
	MW_ADD_AFFINE_TX,
	MW_ADD_ALIGNMENT,
	MW_ADD_BOOL,
	MW_ADD_COLOR,
	MW_ADD_INT8,
	MW_ADD_INT16,
	MW_ADD_INT32,
	MW_ADD_INT64,
	MW_ADD_UINT8,
	MW_ADD_UINT16,
	MW_ADD_UINT32,
	MW_ADD_UINT64,
	MW_ADD_OFF_T,
	MW_ADD_SIZE_T,
	MW_ADD_SSIZE_T,
	MW_ADD_FLOAT,
	MW_ADD_DOUBLE,
	MW_ADD_ENTRY_REF,
	MW_ADD_NODE_REF,
	MW_ADD_STRING,
	MW_ADD_POINT,
	MW_ADD_SIZE,
	MW_ADD_RECT,
	MW_IMPORT_MESSAGE,
	MW_IMPORT_MESSAGE_AS,
	MW_MESSAGE_OPEN_SET_WHAT_DIALOG,
	MW_MESSAGE_MAKE_EMPTY,
	MW_MESSAGE_INFORMATION,

	/* View menu */
	MW_DATA_PANEL_VISIBLE,
};

class MainWindow : public BWindow {
public:
	MainWindow(BRect geometry);
	~MainWindow();
	void MessageReceived(BMessage *msg);
	bool QuitRequested();

private:
	bool continue_action(const char *alert_text,
						 const char *button_label_cancel,
						 const char *button_label_continue);
	void switch_unsaved_state(bool unsaved_state);
	void ToggleDataViewVisibility();

	BMenuBar			*fTopMenuBar;
	MessageView			*fMessageInfoView;
	DataView			*fDataView;
	bool				fUnsaved;
};

#endif

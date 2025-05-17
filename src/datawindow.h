/*
 * Copyright 2019-2021 Andi Machovec <andi.machovec@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 *
 */

#ifndef DATAWINDOW_H
#define DATAWINDOW_H

#include <Window.h>
#include <String.h>
#include <private/interface/ColumnListView.h>
#include <private/shared/ToolBar.h>
#include <Button.h>
#include <StringView.h>

enum DataViewDefs {
	DV_ENTRY_SELECTED = 'dv00',
	DV_ENTRY_INVOKED,
	DV_REMOVE_ENTRY_REQUESTED,
	DV_CLOSE_VIEW_REQUESTED,
};

enum
{
	DW_BUTTON_CLOSE = 'dw00',
	DW_ROW_CLICKED,
	DW_ROW_REMOVE_REQUESTED,
	DW_UPDATE
};

class DataView : public BView
{
public:
						DataView();
						DataView(BMessage*, BString, type_code, int32);

			status_t	SetTo(BMessage*, BString, type_code, int32);
			void		Clear();

	virtual	void		AttachedToWindow();
	virtual	void		MessageReceived(BMessage*);

			BToolBar*	TitleBarView() { return fToolbar; }
	BColumnListView*	DataAreaView() { return fDataView; }

			void		SetLabel(const char* name, const char* typeString);
private:
			void		SetupControls();
private:
	BMessage*			fDataMessage;
	BString 			fFieldName;
	type_code			fFieldType;
	int32				fItemCount;

	BStringView*		fDataLabel;
	BColumnListView*	fDataView;
	BView* 				fStatusView;
	BButton*			fCloseButton;
	BToolBar*			fToolbar;
};

class DataWindow : public BWindow {
public:
	DataWindow(BRect frame, BMessage *data_message,
			   BString field_name,
			   type_code field_type,
			   int32 item_count);

	void MessageReceived(BMessage *msg);


private:
	void get_field_data(BMessage *data_message);

	DataView			*fDataView;
	BButton				*fCloseButton;
	BStringView			*fDataLabel;

	BMessage			*fDataMessage;

	BString 			fFieldName;
	type_code			fFieldType;
	int32				fItemCount;
};

#endif

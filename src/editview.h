/*
 * Copyright 2019-2021 Andi Machovec <andi.machovec@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 *
 */

#ifndef EDITVIEW_H
#define EDITVIEW_H

#include <Button.h>
#include <View.h>
#include <Message.h>
#include <GroupLayout.h>
#include <TextControl.h>
#include <StringView.h>
#include <MenuItem.h>
#include <MenuField.h>
#include <PopUpMenu.h>
#include <RadioButton.h>
#include <private/interface/Spinner.h>
#include <private/interface/ColumnListView.h>
#include <private/interface/DecimalSpinner.h>

enum
{
	EV_DATA_CHANGED='ev00',
	EV_REF_REQUESTED,
	EV_MEASUREMENT_REQUESTED
};

class PreviewableView : public BView
{
public:
	PreviewableView(BRect frame, BBitmap* bitmap);
	virtual void Draw(BRect updateRect);
private:
	void proportional_view(BRect viewRect, BRect imageRect, BRect* resultRect);
private:
	BBitmap* fBitmap;
	rgb_color bgColor;
};

class EditView : public BView {
public:
						EditView(BMessage* msg, type_code type, const char* label, int32 index, bool = false);
			bool 		IsEditable();
			bool 		IsSaveable();
			status_t 	SaveData();
			void		SetDataFor(type_code type, const void* data);
	const	type_code	Type() const { return fDataType; }

private:
			void		SetupControls();
			void		InitControlsData();

	BGroupLayout		*fMainLayout;
	bool				fEditable;
	BFont				fDescFont;
	rgb_color			fDescColor;

	BMessage			*fDataMessage;
	type_code			fDataType;
	const char			*fDataLabel;
	int32				fDataIndex;
	bool 				fIsCreating;

	BButton				*fReusableButton1;
	BColumnListView     *fDataViewer;
	BPopUpMenu			*fPopUpMenu;
	BPopUpMenu			*fPopUpMenu2;
	BRadioButton		*fRadioButton1;
	BRadioButton		*fRadioButton2;
	BSpinner			*fIntegerSpinner1;
	BSpinner			*fIntegerSpinner2;
	BSpinner			*fIntegerSpinner3;
	BSpinner			*fIntegerSpinner4;
	BDecimalSpinner		*fDecimalSpinner1;
	BDecimalSpinner		*fDecimalSpinner2;
	BDecimalSpinner		*fDecimalSpinner3;
	BDecimalSpinner		*fDecimalSpinner4;
	BDecimalSpinner		*fDecimalSpinner5;
	BDecimalSpinner		*fDecimalSpinner6;
	BStringView 		*not_editable_text;
	BStringView         *fSvDescription;
	BTextControl		*fTextCtrlName;
	BTextControl		*fTextCtrl1;
	BTextControl		*fTextCtrl2;
	BTextControl		*fTextCtrl3;
	BTextControl		*fTextCtrl4;
};

#endif

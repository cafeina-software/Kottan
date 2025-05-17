/*
 * Copyright 2025 Cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 *
 */
#include <Box.h>
#include <Catalog.h>
#include <CheckBox.h>
#include <LayoutBuilder.h>
#include <private/interface/ColumnTypes.h>
#include <cstdio>
#include "msginfowindow.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MessageInfoWindow"

MsgInfoWindow::MsgInfoWindow(BRect frame, BMessage* data)
: BWindow(frame, B_TRANSLATE("Message information"), B_FLOATING_WINDOW,
	B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS)
{
	fTcWhat = new BTextControl(B_TRANSLATE("Type"), "", NULL);
	fTcWhat->SetEnabled(false);

	fCbIsSystem = new BCheckBox(B_TRANSLATE("Is a system message                              "));
	fCbIsSystem->SetEnabled(false);
	fCbIsReply = new BCheckBox(B_TRANSLATE("Is a reply to another message"));
	fCbIsReply->SetEnabled(false);
	fTcCountNames = new BTextControl(B_TRANSLATE("Number of data items"), "", NULL);
	fTcCountNames->SetEnabled(false);
	fTcFlattenedSize = new BTextControl(B_TRANSLATE("Flattened size"), "", NULL);
	fTcFlattenedSize->SetEnabled(false);

	BView* generalInfoView = new BView(NULL, B_SUPPORTS_LAYOUT);
	BLayoutBuilder::Group<>(generalInfoView, B_VERTICAL)
		.SetInsets(B_USE_SMALL_INSETS, 0, B_USE_SMALL_INSETS, B_USE_SMALL_INSETS)
		.Add(fCbIsSystem)
		.Add(fCbIsReply)
		.Add(fTcCountNames)
		.Add(fTcFlattenedSize)
	.End();

	BBox* generalInfoBox = new BBox("");
	generalInfoBox->SetLabel(B_TRANSLATE("General information"));
	generalInfoBox->AddChild(generalInfoView);

	fCbWasDelivered = new BCheckBox(B_TRANSLATE("Was delivered"));
	fCbWasDelivered->SetEnabled(false);
	fCbWasDropped = new BCheckBox(B_TRANSLATE("Was dropped"));
	fCbWasDropped->SetEnabled(false);

	BView* deliveryInfoView = new BView(NULL, B_SUPPORTS_LAYOUT);
	BLayoutBuilder::Group<>(deliveryInfoView, B_VERTICAL)
		.SetInsets(B_USE_SMALL_INSETS, 0, B_USE_SMALL_INSETS, B_USE_SMALL_INSETS)
		.Add(fCbWasDelivered)
		.Add(fCbWasDropped)
	.End();

	BBox* deliveryInfoBox = new BBox("");
	deliveryInfoBox->SetLabel(B_TRANSLATE("Delivery information"));
	deliveryInfoBox->AddChild(deliveryInfoView);

	fCbHasSpecifiers = new BCheckBox(B_TRANSLATE("Has specifiers"));
	fCbHasSpecifiers->SetEnabled(false);

	// fClvSpecifiers = new BColumnListView("clv_spec", 0);
	// fClvSpecifiers->AddColumn(new BIntegerColumn(B_TRANSLATE("Index"), 50, 50, 50), 0);
	// fClvSpecifiers->AddColumn(new BStringColumn(B_TRANSLATE("Type"), 100, 250, 100, 0), 1);
	// fClvSpecifiers->AddColumn(new BStringColumn(B_TRANSLATE("Property name"), 100, 250, 100, 0), 2);

	BView* scriptingInfoView = new BView(NULL, B_SUPPORTS_LAYOUT);
	BLayoutBuilder::Group<>(scriptingInfoView, B_VERTICAL)
		.SetInsets(B_USE_SMALL_INSETS, 0, B_USE_SMALL_INSETS, B_USE_SMALL_INSETS)
		.Add(fCbHasSpecifiers)
		// .Add(fClvSpecifiers)
	.End();
	BBox* scriptingInfoBox = new BBox("");
	scriptingInfoBox->SetLabel(B_TRANSLATE("Scripting information"));
	scriptingInfoBox->AddChild(scriptingInfoView);

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_SMALL_INSETS)
		.Add(fTcWhat)
		.Add(generalInfoBox)
		.Add(deliveryInfoBox)
		.Add(scriptingInfoBox)
	.End();

	InitUIData(const_cast<const BMessage*>(data));
}

void
MsgInfoWindow::InitUIData(const BMessage* data)
{
	BString whatData;
	whatData.SetToFormat("0x%.1x", data->what);
	fTcWhat->SetText(whatData.String());

	fCbIsSystem->SetValue(data->IsSystem() ? B_CONTROL_ON : B_CONTROL_OFF);
	fCbIsReply->SetValue(data->IsReply() ? B_CONTROL_ON : B_CONTROL_OFF);

	BString countNamesData;
	countNamesData.SetToFormat("%d", data->CountNames(B_ANY_TYPE));
	fTcCountNames->SetText(countNamesData.String());

	BString flattenedSizeData;
	flattenedSizeData.SetToFormat(B_TRANSLATE("%zd bytes"), data->FlattenedSize());
	fTcFlattenedSize->SetText(flattenedSizeData.String());

	fCbWasDelivered->SetValue(data->WasDelivered() ? B_CONTROL_ON : B_CONTROL_OFF);
	fCbWasDropped->SetValue(data->WasDropped() ? B_CONTROL_ON : B_CONTROL_OFF);

	fCbHasSpecifiers->SetValue(data->HasSpecifiers() ? B_CONTROL_ON : B_CONTROL_OFF);

	// unsigned char* buffer = new unsigned char[data->FlattenedSize()];
	// memcpy(buffer, static_cast<const void*>(data), data->FlattenedSize());
	// BMessage* copy = reinterpret_cast<BMessage*>(buffer);

	// int32 index = 0;
	// BMessage specifier;
	// int32 specifierWhat;
	// const char* property;
	// if(copy->GetCurrentSpecifier(&index, &specifier, &specifierWhat, &property) == B_OK) {
		// BRow* row = new BRow();
		// row->SetField(new BIntegerField(index), 0);
		// row->SetField(new BStringField(StringForSpecifierWhat(specifierWhat)), 1);
		// row->SetField(new BStringField(property), 2);
		// fClvSpecifiers->AddRow(row);

		// copy->PopSpecifier();
	// }
	// fClvSpecifiers->ResizeAllColumnsToPreferred();

	// delete[] buffer;
	// copy = NULL;
}

const char*
MsgInfoWindow::StringForSpecifierWhat(uint32 what)
{
	switch(what)
	{
		case B_DIRECT_SPECIFIER: return STRINGIFY(B_DIRECT_SPECIFIER);
		case B_NAME_SPECIFIER: return STRINGIFY(B_NAME_SPECIFIER);
		case B_ID_SPECIFIER: return STRINGIFY(B_ID_SPECIFIER);
		case B_INDEX_SPECIFIER: return STRINGIFY(B_INDEX_SPECIFIER);
		case B_REVERSE_INDEX_SPECIFIER: return STRINGIFY(B_REVERSE_INDEX_SPECIFIER);
		case B_RANGE_SPECIFIER: return STRINGIFY(B_RANGE_SPECIFIER);
		case B_REVERSE_RANGE_SPECIFIER: return STRINGIFY(B_REVERSE_RANGE_SPECIFIER);
		default: return BString().SetToFormat(B_TRANSLATE("0x%x (unknown specifier)"), what);
	}
}


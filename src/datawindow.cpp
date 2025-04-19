/*
 * Copyright 2019-2021 Andi Machovec <andi.machovec@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 *
 */

#include "datawindow.h"
#include "app.h"
#include "gettype.h"
#include "kottandefs.h"

#include <Alert.h>
#include <LayoutBuilder.h>
#include <Catalog.h>
#include <NetworkAddress.h>
#include <private/interface/ColumnTypes.h>
#include <Entry.h>
#include <Path.h>
#include <Application.h>
#include <StatusBar.h>
#include <iomanip>
#include <sstream>
#include <sys/socket.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "DataView"

DataView::DataView()
: BView(NULL, B_SUPPORTS_LAYOUT | B_AUTO_UPDATE_SIZE_LIMITS, NULL),
  fDataMessage(NULL),
  fFieldName(""),
  fFieldType(B_ANY_TYPE),
  fItemCount(0)
{
	SetupControls();
}

DataView::DataView(BMessage* data, BString name, type_code type, int32 count)
: BView(NULL, B_SUPPORTS_LAYOUT | B_AUTO_UPDATE_SIZE_LIMITS, NULL)
{
	SetupControls();
	SetTo(data, name, type, count);
}

status_t
DataView::SetTo(BMessage* data, BString name, type_code type, int32 count)
{
	fDataMessage = data;
    fFieldName = name;
    fFieldType = type;
    fItemCount = count;

	if(!Window()->IsLocked())
		LockLooper();

	if(!data) {
		fDataLabel->SetText("");
		fDataView->Clear();
		if(Window()->IsLocked())
			UnlockLooper();
		return B_NO_INIT;
	}

	if(type == B_MESSAGE_TYPE) {
		fDataView->Clear();
		if(Window()->IsLocked())
			UnlockLooper();
		return B_OK;
	}

	Clear();

	BString label("%fieldName% (%fieldType%)");
	label.ReplaceAll("%fieldName%", fFieldName.String());
	label.ReplaceAll("%fieldType%", get_type(fFieldType).String());
	fDataLabel->SetText(label);

	for(int i = 0; i < count; i++) {
		BString itemData;

		switch(fFieldType)
		{
			case B_AFFINE_TRANSFORM_TYPE:
			{
				const void* ptr = NULL;
				ssize_t length = 0;
				fDataMessage->FindData(fFieldName, fFieldType, i, &ptr, &length);

				double tx, ty, sx, sy, shx, shy;

				/* The code commented below is not working as expected... */
				// BAffineTransform affineTransform;
				// affineTransform.Unflatten(B_AFFINE_TRANSFORM_TYPE, ptr, length);
				// affineTransform.GetAffineParameters(&tx, &ty, NULL, &sx, &sy, &shx, &shy);

				/* so let's try with raw manipulation */
				const double* raw = static_cast<const double*>(ptr);
				tx = raw[4];
				ty = raw[5];
				sx = raw[0];
				sy = raw[3];
				shx = raw[2];
				shy = raw[1];

				itemData << B_TRANSLATE("translation") << "(" << tx << ", " << ty << "); "
						 << B_TRANSLATE("scale") << "(" << sx << ", " << sy << "); "
						 << B_TRANSLATE("shear") << "(" << shx << ", " << shy << ")";
				break;
			}

			case B_ALIGNMENT_TYPE:
			{
				BAlignment alignment = fDataMessage->GetAlignment(fFieldName, i, BAlignment());

				switch(alignment.Horizontal()) {
					case B_ALIGN_LEFT:
						itemData << B_TRANSLATE("left");
						break;
					case B_ALIGN_RIGHT:
						itemData << B_TRANSLATE("right");
						break;
					case B_ALIGN_CENTER:
						itemData << B_TRANSLATE("center");
						break;
					case B_ALIGN_USE_FULL_WIDTH:
						itemData << B_TRANSLATE("full width");
						break;
					case B_ALIGN_HORIZONTAL_UNSET:
						itemData << B_TRANSLATE("(unset)");
						break;
					default:
						itemData << B_TRANSLATE("(invalid)");
						break;
				}

				itemData << ", ";

				switch(alignment.Vertical()) {
					case B_ALIGN_TOP:
						itemData << B_TRANSLATE("top");
						break;
					case B_ALIGN_MIDDLE:
						itemData << B_TRANSLATE("middle");
						break;
					case B_ALIGN_BOTTOM:
						itemData << B_TRANSLATE("bottom");
						break;
					case B_ALIGN_USE_FULL_HEIGHT:
						itemData << B_TRANSLATE("full height");
						break;
					case B_ALIGN_VERTICAL_UNSET:
						itemData << B_TRANSLATE("(unset)");
						break;
					default:
						itemData << B_TRANSLATE("(invalid)");
						break;
				}

				break;
			}

			case B_BOOL_TYPE:
			{
				bool bool_value = fDataMessage->GetBool(fFieldName, i, false);
				if (bool_value)
					itemData = B_TRANSLATE("true");
				else
					itemData = B_TRANSLATE("false");

				break;
			}

			case B_DOUBLE_TYPE:
			{
				std::stringstream convert_stream;
				convert_stream << std::fixed << std::setprecision(4) << fDataMessage->GetDouble(fFieldName, i, 0);
				itemData << BString(convert_stream.str().c_str());
				break;
			}

			case B_FLOAT_TYPE:
			{
				std::stringstream convert_stream;
				convert_stream << std::fixed << std::setprecision(4) << fDataMessage->GetFloat(fFieldName, i, 0);
				itemData << BString(convert_stream.str().c_str());
				break;
			}

			case B_INT8_TYPE:
			{
				itemData << fDataMessage->GetInt8(fFieldName, i, 0);
				break;
			}

			case B_INT16_TYPE:
			{
				itemData << fDataMessage->GetInt16(fFieldName, i, 0);
				break;
			}

			case B_INT32_TYPE:
			{
				itemData << fDataMessage->GetInt32(fFieldName, i, 0);
				break;
			}

			case B_INT64_TYPE:
			{
				itemData << fDataMessage->GetInt64(fFieldName, i, 0);
				break;
			}

			case B_MIME_TYPE:
			{
				const void* ptr = NULL;
				ssize_t length = 0;
				fDataMessage->FindData(fFieldName, fFieldType, i, &ptr, &length);
				char* buffer = new char[length + 1];
				buffer[length] = '\0';
				strncpy(buffer, static_cast<const char*>(ptr), length);
				BString mimeData = buffer;
				itemData << mimeData;
				delete[] buffer;
				break;
			}

			case B_NETWORK_ADDRESS_TYPE:
			{
				const void* ptr = NULL;
				ssize_t length = 0;
				fDataMessage->FindData(fFieldName, fFieldType, i, &ptr, &length);
				sockaddr_storage storage;
				memcpy(&storage, ptr, length);
				BNetworkAddress address(storage);
				BString addrString = address.ToString(true);
				itemData << B_TRANSLATE("family: ")
				         << NetAddressFamilyString(storage.ss_family) << ", "
				         << B_TRANSLATE("length: ")
						 << storage.ss_len << ", "
						 << B_TRANSLATE("address: ")
						 << (addrString.Length() > 0 ? addrString.String() : B_TRANSLATE("(no resolvable address)"));
				break;
			}

			case B_OFF_T_TYPE:
			{
				const void* ptr = NULL;
				ssize_t length = 0;
				fDataMessage->FindData(fFieldName, fFieldType, i, &ptr, &length);

				off_t offset = 0;
				memcpy(&offset, ptr, length);

				itemData << offset;
				break;
			}

			case B_POINT_TYPE:
			{
				BPoint point = fDataMessage->GetPoint(fFieldName, i, BPoint());
				itemData << point.x << ", " << point.y;
				break;
			}

			case B_RECT_TYPE:
			{
				BRect rect = fDataMessage->GetRect(fFieldName, i, BRect());
				itemData << rect.left  << ", " << rect.top << ", "
				         << rect.right << ", " << rect.bottom;
				break;
			}

			case B_REF_TYPE:
			{
				entry_ref ref;
				status_t result = fDataMessage->FindRef(fFieldName, i, &ref);
				if(result == B_OK) {
					BEntry entry(&ref);
					if(entry.Exists()) { // Entry exists: show path
						BPath path(&entry);
						itemData << path.Path();
					}
					else { // Abstract entry: show what we have
						BString data;
						data.SetToFormat(B_TRANSLATE("device: %" B_PRIdDEV ", "
							"directory: %" B_PRIdINO ", name: %s"),
							ref.device, ref.directory, ref.name);
						itemData << data.String();
					}
				}
				break;
			}

			case B_NODE_REF_TYPE:
			{
				node_ref nref;
				status_t result = fDataMessage->FindNodeRef(fFieldName, i, &nref);
				if(result == B_OK) {
					BString data;
					data.SetToFormat(B_TRANSLATE("device: %" B_PRIdDEV ", node: %"
						B_PRIdINO), nref.device, nref.node);
					itemData << data;
				}
				break;
			}

			case B_RGB_COLOR_TYPE:
			{
				// BMessage->FindColor() or GetColor() don´t seem to work on B_RGB_COLOR_TYPE so we
				// have to use the raw FindData() here
				const void *color_ptr;
				ssize_t data_size = sizeof(rgb_color);
				fDataMessage->FindData(fFieldName, B_RGB_COLOR_TYPE, i, &color_ptr, &data_size);
				rgb_color color = *(static_cast<const rgb_color*>(color_ptr));

				itemData << color.red;
				itemData += ", ";
				itemData << color.green;
				itemData += ", ";
				itemData << color.blue;
				itemData += ", ";
				itemData << color.alpha;

				break;
			}

			case B_SIZE_TYPE:
			{
				BSize size = fDataMessage->GetSize(fFieldName, i, BSize());
				itemData << size.width << ", " << size.height;
				break;
			}

			case B_SIZE_T_TYPE:
			{
				const void* ptr = NULL;
				ssize_t length = 0;
				fDataMessage->FindData(fFieldName, fFieldType, i, &ptr, &length);

				size_t size = 0;
				memcpy(&size, ptr, length);

				itemData << size;
				break;
			}

			case B_SSIZE_T_TYPE:
			{
				const void* ptr = NULL;
				ssize_t length = 0;
				fDataMessage->FindData(fFieldName, fFieldType, i, &ptr, &length);

				ssize_t size = 0;
				memcpy(&size, ptr, length);

				itemData << size;
				break;
			}

			case B_STRING_TYPE:
				itemData = fDataMessage->GetString(fFieldName, i, "");
				break;

			case B_UINT8_TYPE:
			{
				itemData << fDataMessage->GetUInt8(fFieldName, i, 0);
				break;
			}

			case B_UINT16_TYPE:
			{
				itemData << fDataMessage->GetUInt16(fFieldName, i, 0);
				break;
			}

			case B_UINT32_TYPE:
			{
				itemData << fDataMessage->GetUInt32(fFieldName, i, 0);
				break;
			}

			case B_UINT64_TYPE:
			{
				itemData << fDataMessage->GetUInt64(fFieldName, i, 0);
				break;
			}

			default:
				itemData << B_TRANSLATE("data cannot be displayed");
				break;
		}

		BRow* row = new BRow();
		row->SetField(new BIntegerField(i), 0);
		row->SetField(new BStringField(itemData), 1);
		fDataView->AddRow(row);
	}

	if(Window()->IsLocked())
		UnlockLooper();
	return B_OK;
}

void
DataView::Clear()
{
	fDataLabel->SetText("");
	fDataView->Clear();
}

void
DataView::AttachedToWindow()
{
	fDataView->SetTarget(this);
	for(const auto& command : {DV_REMOVE_ENTRY_REQUESTED, DV_CLOSE_VIEW_REQUESTED})
		fToolbar->FindButton(command)->SetTarget(this);
}

void
DataView::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
		case DV_ENTRY_SELECTED:
		{
			fToolbar->FindButton(DV_REMOVE_ENTRY_REQUESTED)->SetEnabled(true);
			break;
		}
		case DV_ENTRY_INVOKED:
		{
			// fprintf(stdout, "Invoked\n");
			BRow *selected_row = fDataView->CurrentSelection();
			int32 field_index = static_cast<BIntegerField*>(selected_row->GetField(0))->Value();

			BMessage request(DW_ROW_CLICKED);
			request.AddInt32(KottanFieldIndex, field_index);
			request.AddString(KottanFieldName, fFieldName);
			request.AddUInt32(KottanFieldType, fFieldType);
			Window()->PostMessage(&request);
			break;
		}
		case DV_REMOVE_ENTRY_REQUESTED:
		{
			BRow* selection = fDataView->CurrentSelection();
			if(!selection)
				break;
			BIntegerField* indexField = ((BIntegerField*)selection->GetField(0));

			BMessage request(DW_ROW_REMOVE_REQUESTED);
			request.AddInt32(KottanFieldIndex, indexField->Value());
			request.AddString(KottanFieldName, fFieldName);
			request.AddUInt32(KottanFieldType, fFieldType);
			Window()->PostMessage(&request);
			break;
		}
		case DV_CLOSE_VIEW_REQUESTED:
		{
			BMessage request(DW_BUTTON_CLOSE);
			Window()->PostMessage(&request);
			break;
		}
		default:
			return BView::MessageReceived(msg);
	}
}

void
DataView::SetPanelMode(bool status)
{
	LockLooper();

	if(status) {
		fToolbar->FindButton(DV_CLOSE_VIEW_REQUESTED)->Show();
	}
	else
		fToolbar->FindButton(DV_CLOSE_VIEW_REQUESTED)->Hide();

	UnlockLooper();
}

void
DataView::SetupControls()
{
	fDataLabel = new BStringView("datalabel", "");
	BFont font(be_bold_font);
	fDataLabel->SetFont(&font);

	fToolbar = new BToolBar(B_HORIZONTAL);
	fToolbar->AddView(fDataLabel);
	fToolbar->AddGlue();
	fToolbar->AddAction(DV_REMOVE_ENTRY_REQUESTED, this, trashIcon, B_TRANSLATE("Delete"), B_TRANSLATE("Delete"), false);
	fToolbar->FindButton(DV_REMOVE_ENTRY_REQUESTED)->SetEnabled(false);
	fToolbar->AddAction(DV_CLOSE_VIEW_REQUESTED, this, removeIcon, B_TRANSLATE("Close"), B_TRANSLATE("Close"), false);

	fStatusView = new BView("status_view", B_SUPPORTS_LAYOUT);

	fDataView = new BColumnListView("dataview", 0);
	fDataView->SetSelectionMessage(new BMessage(DV_ENTRY_SELECTED));
	fDataView->SetInvocationMessage(new BMessage(DV_ENTRY_INVOKED));
	fDataView->AddColumn(new BIntegerColumn(B_TRANSLATE("Index"), 70, 10, 100), 0);
	fDataView->AddColumn(new BStringColumn(B_TRANSLATE("Value"), 200, 50, 1000, 0), 1);
	fDataView->AddStatusView(fStatusView);

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(fToolbar)
		.Add(fDataView)
	.Layout();
}

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "DataWindow"


DataWindow::DataWindow(BRect frame,
					BMessage *data_message,
					BString field_name,
					type_code field_type,
					int32 item_count)
	:
	BWindow(frame, B_TRANSLATE("Message data"), B_DOCUMENT_WINDOW_LOOK,
		B_MODAL_APP_WINDOW_FEEL, B_CLOSE_ON_ESCAPE | B_AUTO_UPDATE_SIZE_LIMITS),
	fDataMessage(data_message),
	fFieldName(field_name),
	fFieldType(field_type),
	fItemCount(item_count)
{
	fDataView = new DataView(fDataMessage, fFieldName, fFieldType, fItemCount);

	// fCloseButton = new BButton(B_TRANSLATE("Close"), new BMessage(DW_BUTTON_CLOSE));

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_SMALL_SPACING)
		.SetInsets(B_USE_SMALL_SPACING)
		.Add(fDataView)
		// .Add(fCloseButton)
	.Layout();

}


void
DataWindow::MessageReceived(BMessage *msg)
{

	switch (msg->what)
	{
		case DW_BUTTON_CLOSE:
		{
			Quit();
			break;
		}

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

		case DW_UPDATE:
		{
			fDataView->Clear();
			fDataView->SetTo(fDataMessage, fFieldName, fFieldType, fItemCount);
			break;
		}

		case DW_ROW_REMOVE_REQUESTED:
		{
			if((new BAlert("", B_TRANSLATE("Do you want to remove this item?"),
			B_TRANSLATE("Remove"), B_TRANSLATE("Keep")))->Go() == 0) {
				be_app->PostMessage(msg);
				Quit();
			}
			break;
		}

		default:
		{
			BWindow::MessageReceived(msg);
			break;
		}
	}

}

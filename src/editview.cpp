/*
 * Copyright 2019-2021 Andi Machovec <andi.machovec@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 *
 */

#include "editview.h"
#include <Box.h>
#include <Button.h>
#include <LayoutBuilder.h>
#include <View.h>
#include <FilePanel.h>
#include <Catalog.h>
#include <Path.h>
#include <ios>
#include <private/interface/ColumnTypes.h>
#include <IconUtils.h>
#include <limits>
#include <cctype>
#include <cstdlib>
#include <cstdio>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "EditView"


static double
roundTo(double value, uint32 n)
{
	return floor(value * pow(10.0, n) + 0.5) / pow(10.0, n);
}

static int32
rowIndex(BColumnListView* view, int32 colIndex, const char* name)
{
	if(view->CountRows() < 1)
		return -1;

	bool found = false;
	int32 i = 0;
	while(!found && i < view->CountRows()) {
		if(strcmp(((BStringField*)view->RowAt(i)->GetField(colIndex))->String(), name) == 0)
			found = true;
		else
			i++;
	}

	if(!found)
		return -1;

	return i;
}

// #pragma mark -

PreviewableView::PreviewableView(BRect frame, BBitmap* bitmap)
	: BView(frame, "preview", B_FOLLOW_ALL, B_WILL_DRAW),
	fBitmap(bitmap)
{
	bgColor = ui_color(B_DOCUMENT_BACKGROUND_COLOR);
}

void
PreviewableView::Draw(BRect rect)
{
	SetHighUIColor(B_CONTROL_BORDER_COLOR);
	SetPenSize(2.0f);
	BRect border = BRect(Bounds().left + 1, Bounds().top + 1,
		Bounds().right, Bounds().bottom);
	MovePenTo(Bounds().left, Bounds().top);
	StrokeRect(border);

	SetHighColor(bgColor);
	FillRect(Bounds());

	if(fBitmap) {
		BRect imageRect = fBitmap->Bounds();
		BRect viewRect = Frame().InsetByCopy(4.0f, 4.0f);
		BRect resultRect;
		proportional_view(viewRect, imageRect, &resultRect);

		auto dmode = DrawingMode();
		SetDrawingMode(B_OP_ALPHA);
		DrawBitmap(fBitmap, resultRect);
		SetDrawingMode(dmode);
	}

	Invalidate();
}

void
PreviewableView::BorrowBitmap(BBitmap* bitmap)
{
	fBitmap = bitmap;
}

void
PreviewableView::ReturnBitmap()
{
	fBitmap = NULL;
}

void
PreviewableView::proportional_view(BRect viewRect, BRect imageRect, BRect* resultRect)
{
    /* From LockWorkstation rework: resize image to bounds keeping proportions */
    float rx = (viewRect.Width() / imageRect.Width());
    float ry = (viewRect.Height() / imageRect.Height());
    float rfactor = rx <= ry ? rx : ry; // Resizing ratio

    float w = imageRect.Width() * rfactor;
    float h = imageRect.Height() * rfactor;
    float sx = (viewRect.Width() - w) / 2.0f;
    float sy = (viewRect.Height() - h) / 2.0f;

    resultRect->Set(sx, sy, sx + w, sy + h);
}

// #pragma mark -

EditView::EditView(BMessage* msg, type_code type, const char* label, int32 index, bool creating)
	: BView("editview", B_SUPPORTS_LAYOUT),
	fEditable(true),
	fDataMessage(msg),
	fDataType(type),
	fDataLabel(label),
	fDataIndex(index),
	fIsCreating(creating),
	fPreviewableBitmap(NULL)
{
	fDescFont = be_plain_font;
	fDescFont.SetFace(B_ITALIC_FACE);
	fDescColor = ui_color(B_WINDOW_INACTIVE_TEXT_COLOR);

	if(!Window()->IsLocked())
		LockLooper();

	//create layout
	fMainLayout = new BGroupLayout(B_VERTICAL);
	((BView*)this)->SetLayout(fMainLayout);

	//initialize controls
	fReusableButton1 = new BButton("", new BMessage());
	fDataViewer = new BColumnListView("clv", B_NAVIGABLE, B_FANCY_BORDER);
	fPopUpMenu = new BPopUpMenu("");
	fPopUpMenu2 = new BPopUpMenu("");
	fRadioButton1 = new BRadioButton("", new BMessage(EV_DATA_CHANGED));
	fRadioButton2 = new BRadioButton("", new BMessage(EV_DATA_CHANGED));
	fIntegerSpinner1 = new BSpinner("","",new BMessage(EV_DATA_CHANGED));
	fIntegerSpinner2 = new BSpinner("","",new BMessage(EV_DATA_CHANGED));
	fIntegerSpinner3 = new BSpinner("","",new BMessage(EV_DATA_CHANGED));
	fIntegerSpinner4 = new BSpinner("","",new BMessage(EV_DATA_CHANGED));
	fIntegerSpinner5 = new BSpinner("","",new BMessage(EV_DATA_CHANGED));
	fDecimalSpinner1 = new BDecimalSpinner("","",new BMessage(EV_DATA_CHANGED));
	fDecimalSpinner2 = new BDecimalSpinner("","",new BMessage(EV_DATA_CHANGED));
	fDecimalSpinner3 = new BDecimalSpinner("","",new BMessage(EV_DATA_CHANGED));
	fDecimalSpinner4 = new BDecimalSpinner("","",new BMessage(EV_DATA_CHANGED));
	fDecimalSpinner5 = new BDecimalSpinner("","",new BMessage(EV_DATA_CHANGED));
	fDecimalSpinner6 = new BDecimalSpinner("","",new BMessage(EV_DATA_CHANGED));
	not_editable_text = new BStringView("", B_TRANSLATE("not editable"));
	fSvDescription = new BStringView(NULL, "");
	fTextCtrlName = new BTextControl(B_TRANSLATE("Name"), "", new BMessage(EV_DATA_CHANGED));
	fTextCtrl1 = new BTextControl("","",new BMessage(EV_DATA_CHANGED));
	fTextCtrl2 = new BTextControl("","",new BMessage(EV_DATA_CHANGED));
	fTextCtrl3 = new BTextControl("","",new BMessage(EV_DATA_CHANGED));
	fTextCtrl4 = new BTextControl("","",new BMessage(EV_DATA_CHANGED));

	fTextCtrlName->SetModificationMessage(new BMessage(EV_DATA_CHANGED));
	fTextCtrl1->SetModificationMessage(new BMessage(EV_DATA_CHANGED));
	fTextCtrl2->SetModificationMessage(new BMessage(EV_DATA_CHANGED));
	fTextCtrl3->SetModificationMessage(new BMessage(EV_DATA_CHANGED));
	fTextCtrl4->SetModificationMessage(new BMessage(EV_DATA_CHANGED));

	// Configure controls
	SetupControls(); // Add the controls needed to the layout
	InitControlsData(); // Fill the controls for the specified data type with values

	if(Window()->IsLocked())
		UnlockLooper();
}

EditView::~EditView()
{
	if(fPreviewableBitmap)
		delete fPreviewableBitmap;
}

bool
EditView::IsEditable()
{
	return fEditable;
}

bool
EditView::IsSaveable()
{
	if(fIsCreating)
		return fTextCtrlName->TextLength() > 0;
	else
		return IsEditable();
}

void
EditView::SetDataFor(type_code type, const void* data)
{
	switch(type)
	{
		case B_NODE_REF_TYPE:
		{
			const entry_ref* ref = static_cast<const entry_ref*>(data);
			node_ref nref;
			BEntry(ref).GetNodeRef(&nref);
			BString dev_t_data = BString().SetToFormat("%" B_PRIdDEV, nref.device);
			BString ino_t_data = BString().SetToFormat("%" B_PRIdINO, nref.node);

			if(fIsCreating) { // Pre create the rows when in creation mode
				BRow* deviceRow = new BRow();
				deviceRow->SetField(new BStringField(B_TRANSLATE("Device")), 0);
				deviceRow->SetField(new BStringField(BString().SetToFormat("%" B_PRIdDEV, nref.device)), 1);
				fDataViewer->AddRow(deviceRow);

				BRow* nodeRow = new BRow();
				nodeRow->SetField(new BStringField(B_TRANSLATE("Node")), 0);
				nodeRow->SetField(new BStringField(BString().SetToFormat("%" B_PRIdINO, nref.node)), 1);
				fDataViewer->AddRow(nodeRow);
			}

			((BStringField*)fDataViewer->RowAt(rowIndex(fDataViewer, 0,
				B_TRANSLATE("Device")))->GetField(1))->SetString(dev_t_data);
			((BStringField*)fDataViewer->RowAt(rowIndex(fDataViewer, 0,
				B_TRANSLATE("Node")))->GetField(1))->SetString(ino_t_data);

			Window()->PostMessage(EV_DATA_CHANGED);
			break;
		}

		case B_POINT_TYPE:
		{
			const BPoint* point = static_cast<const BPoint*>(data);
			fDecimalSpinner1->SetValue(point->x);
			fDecimalSpinner2->SetValue(point->y);

			Window()->PostMessage(EV_DATA_CHANGED);
			break;
		}

		case B_RECT_TYPE:
		{
			const BRect* rect = static_cast<const BRect*>(data);
			fDecimalSpinner1->SetValue(rect->left);
			fDecimalSpinner2->SetValue(rect->top);
			fDecimalSpinner3->SetValue(rect->right);
			fDecimalSpinner4->SetValue(rect->bottom);

			Window()->PostMessage(EV_DATA_CHANGED);
			break;
		}

		case B_REF_TYPE:
		{
			const entry_ref* ref = static_cast<const entry_ref*>(data);
			BString dev_t_data = BString().SetToFormat("%" B_PRIdDEV, ref->device);
			BString ino_t_data = BString().SetToFormat("%" B_PRIdINO, ref->directory);
			BEntry entry(ref);
			BPath path;
			entry.GetPath(&path);

			if(fIsCreating) { // Pre create the rows when in creation mode
				BRow* deviceRow = new BRow();
				deviceRow->SetField(new BStringField(B_TRANSLATE("Device")), 0);
				deviceRow->SetField(new BStringField(""), 1);
				fDataViewer->AddRow(deviceRow);

				BRow* directoryRow = new BRow();
				directoryRow->SetField(new BStringField(B_TRANSLATE("Directory")), 0);
				directoryRow->SetField(new BStringField(""), 1);
				fDataViewer->AddRow(directoryRow);

				BRow* nameRow = new BRow();
				nameRow->SetField(new BStringField(B_TRANSLATE("Name")), 0);
				nameRow->SetField(new BStringField(""), 1);
				fDataViewer->AddRow(nameRow);
			}


			((BStringField*)fDataViewer->RowAt(rowIndex(fDataViewer, 0,
				B_TRANSLATE("Device")))->GetField(1))->SetString(dev_t_data);
			((BStringField*)fDataViewer->RowAt(rowIndex(fDataViewer, 0,
				B_TRANSLATE("Directory")))->GetField(1))->SetString(ino_t_data);
			((BStringField*)fDataViewer->RowAt(rowIndex(fDataViewer, 0,
				B_TRANSLATE("Name")))->GetField(1))->SetString(ref->name);

			fTextCtrl1->SetText(path.Path());

			Window()->PostMessage(EV_DATA_CHANGED);
			break;
		}

		case B_SIZE_TYPE:
		{
			const BSize* size = static_cast<const BSize*>(data);
			fDecimalSpinner2->SetValue(size->width);
			fDecimalSpinner1->SetValue(size->height);

			Window()->PostMessage(EV_DATA_CHANGED);
			break;
		}

		case B_TIME_TYPE:
		{
			const time_t* time = static_cast<const time_t*>(data);
			BDateTime datetime;
			datetime.SetTime_t(*time);

			fTextCtrl1->SetText(BString().SetToFormat("%d", datetime.Date().Year()));
			fIntegerSpinner2->SetValue(datetime.Date().Month());
			fIntegerSpinner1->SetValue(datetime.Date().Day());

			fIntegerSpinner3->SetValue(datetime.Time().Hour());
			fIntegerSpinner4->SetValue(datetime.Time().Minute());
			fIntegerSpinner5->SetValue(datetime.Time().Second());

			Window()->PostMessage(EV_DATA_CHANGED);
			break;
		}
	}
}

status_t
EditView::SaveData()
{
	if(!IsEditable())
		return B_NOT_ALLOWED;

	switch(fDataType)
	{
		case B_AFFINE_TRANSFORM_TYPE:
		{
			BAffineTransform affineTransform(fDecimalSpinner3->Value(),
				fDecimalSpinner6->Value(), fDecimalSpinner5->Value(),
				fDecimalSpinner4->Value(), fDecimalSpinner1->Value(),
				fDecimalSpinner2->Value());
			ssize_t length = affineTransform.FlattenedSize();
			unsigned char* buffer = new unsigned char[length];
			memset(buffer, 0, length);
			affineTransform.Flatten(buffer, length);

			if(fIsCreating)
				fDataMessage->AddData(fTextCtrlName->Text(),
					B_AFFINE_TRANSFORM_TYPE, buffer, length, true);
			else
				fDataMessage->ReplaceData(fDataLabel, B_AFFINE_TRANSFORM_TYPE,
					fDataIndex, buffer, length);

			delete[] buffer;
			break;
		}

		case B_ALIGNMENT_TYPE:
		{
			BAlignment alignment;
			int32 hindex = fPopUpMenu->IndexOf(fPopUpMenu->FindMarked());
			int32 vindex = fPopUpMenu2->IndexOf(fPopUpMenu2->FindMarked());
			switch(hindex) {
				case 0:
					alignment.SetHorizontal(B_ALIGN_LEFT);
					break;
				case 1:
					alignment.SetHorizontal(B_ALIGN_RIGHT);
					break;
				case 2:
					alignment.SetHorizontal(B_ALIGN_CENTER);
					break;
				case 5:
					alignment.SetHorizontal(B_ALIGN_USE_FULL_WIDTH);
					break;
				case 4:
				default:
					alignment.SetHorizontal(B_ALIGN_HORIZONTAL_UNSET);
					break;
			}
			switch(vindex) {
				case 0:
					alignment.SetVertical(B_ALIGN_TOP);
					break;
				case 1:
					alignment.SetVertical(B_ALIGN_MIDDLE);
					break;
				case 2:
					alignment.SetVertical(B_ALIGN_BOTTOM);
					break;
				case 5:
					alignment.SetVertical(B_ALIGN_USE_FULL_HEIGHT);
					break;
				case 4:
				default:
					alignment.SetVertical(B_ALIGN_VERTICAL_UNSET);
					break;
			}

			if(fIsCreating)
				fDataMessage->AddAlignment(fTextCtrlName->Text(), alignment);
			else
				fDataMessage->ReplaceAlignment(fDataLabel, fDataIndex, alignment);
			break;
		}

		case B_BOOL_TYPE:
		{
			bool isTrue = fRadioButton1->Value() == B_CONTROL_ON;
			if(fIsCreating)
				fDataMessage->AddBool(fTextCtrlName->Text(), isTrue);
			else
				fDataMessage->ReplaceBool(fDataLabel, fDataIndex, isTrue);
			break;
		}

		case B_CHAR_TYPE:
		{
			char c = static_cast<char>(fIntegerSpinner1->Value());
			if(fIsCreating)
				fDataMessage->AddData(fTextCtrlName->Text(), B_CHAR_TYPE,
					&c, sizeof(c));
			else
				fDataMessage->ReplaceData(fDataLabel, B_CHAR_TYPE, fDataIndex,
					&c, sizeof(c));
			break;
		}

		case B_INT8_TYPE:
		{
			int8 value = static_cast<int8>(fIntegerSpinner1->Value());
			if(fIsCreating)
				fDataMessage->AddInt8(fTextCtrlName->Text(), value);
			else
				fDataMessage->ReplaceInt8(fDataLabel, fDataIndex, value);
			break;
		}

		case B_INT16_TYPE:
		{
			int16 value = static_cast<int16>(fIntegerSpinner1->Value());
			if(fIsCreating)
				fDataMessage->AddInt16(fTextCtrlName->Text(), value);
			else
				fDataMessage->ReplaceInt16(fDataLabel, fDataIndex,value);
			break;
		}

		case B_INT32_TYPE:
		{
			int32 value = fIntegerSpinner1->Value();
			if(fIsCreating)
				fDataMessage->AddInt32(fTextCtrlName->Text(), value);
			else
				fDataMessage->ReplaceInt32(fDataLabel, fDataIndex, value);
			break;
		}

		case B_UINT8_TYPE:
		{
			uint8 value = static_cast<uint8>(fIntegerSpinner1->Value());
			if(fIsCreating)
				fDataMessage->AddUInt8(fTextCtrlName->Text(), value);
			else
				fDataMessage->ReplaceUInt8(fDataLabel, fDataIndex, value);
			break;
		}

		case B_UINT16_TYPE:
		{
			uint16 value = static_cast<uint16>(fIntegerSpinner1->Value());
			if(fIsCreating)
				fDataMessage->AddUInt16(fTextCtrlName->Text(), value);
			else
				fDataMessage->ReplaceUInt16(fDataLabel, fDataIndex, value);
			break;
		}

		case B_UINT32_TYPE:
		{
			uint32 value = static_cast<uint32>(fIntegerSpinner1->Value());
			if(fIsCreating)
				fDataMessage->AddUInt32(fTextCtrlName->Text(), value);
			else
				fDataMessage->ReplaceUInt32(fDataLabel, fDataIndex, value);
			break;
		}

		case B_FLOAT_TYPE:
		{
			float float_data = roundTo(std::atof(fDecimalSpinner1->TextView()->Text()), fDecimalSpinner1->Precision());
			if(fIsCreating)
				fDataMessage->AddFloat(fTextCtrlName->Text(), float_data);
			else
				fDataMessage->ReplaceFloat(fDataLabel, fDataIndex, float_data);
			break;
		}

		case B_DOUBLE_TYPE:
		{
			double double_data = roundTo(std::atof(fDecimalSpinner1->TextView()->Text()), fDecimalSpinner1->Precision());
			if(fIsCreating)
				fDataMessage->AddDouble(fTextCtrlName->Text(), double_data);
			else
				fDataMessage->ReplaceDouble(fDataLabel, fDataIndex, double_data);
			break;
		}

		case B_SIZE_TYPE:
		{
			BSize data_size;
			data_size.height = fDecimalSpinner1->Value();
			data_size.width = fDecimalSpinner2->Value();
			if(fIsCreating)
				fDataMessage->AddSize(fTextCtrlName->Text(), data_size);
			else
				fDataMessage->ReplaceSize(fDataLabel, fDataIndex, data_size);
			break;
		}

		case B_POINT_TYPE:
		{
			BPoint data_point;
			data_point.x = fDecimalSpinner1->Value();
			data_point.y = fDecimalSpinner2->Value();
			if(fIsCreating)
				fDataMessage->AddPoint(fTextCtrlName->Text(), data_point);
			else
				fDataMessage->ReplacePoint(fDataLabel, fDataIndex, data_point);
			break;
		}

		case B_RECT_TYPE:
		{
			BRect data_rect;
			data_rect.left = fDecimalSpinner1->Value();
			data_rect.top = fDecimalSpinner2->Value();
			data_rect.right = fDecimalSpinner3->Value();
			data_rect.bottom = fDecimalSpinner4->Value();
			if(fIsCreating)
				fDataMessage->AddRect(fTextCtrlName->Text(), data_rect);
			else
				fDataMessage->ReplaceRect(fDataLabel, fDataIndex, data_rect);
			break;
		}

		case B_STRING_TYPE:
		{
			if(fIsCreating)
				fDataMessage->AddString(fTextCtrlName->Text(), fTextCtrl1->Text());
			else
				fDataMessage->ReplaceString(fDataLabel, fDataIndex, fTextCtrl1->Text());
			break;
		}

		case B_RGB_COLOR_TYPE:
		{
			rgb_color data_rgbcolor;
			data_rgbcolor.red =  static_cast<uint8>(fIntegerSpinner1->Value());
			data_rgbcolor.green = static_cast<uint8>(fIntegerSpinner2->Value());
			data_rgbcolor.blue = static_cast<uint8>(fIntegerSpinner3->Value());
			data_rgbcolor.alpha = static_cast<uint8>(fIntegerSpinner4->Value());
			if(fIsCreating)
				fDataMessage->AddData(fTextCtrlName->Text(), B_RGB_COLOR_TYPE,
					&data_rgbcolor, sizeof(data_rgbcolor));
			else
				fDataMessage->ReplaceData(fDataLabel, B_RGB_COLOR_TYPE, fDataIndex,
					&data_rgbcolor, sizeof(data_rgbcolor));

			break;
		}

		case B_TIME_TYPE:
		{
			int32 year = atoi(fTextCtrl1->Text());
			BDate date(year, fIntegerSpinner2->Value(), fIntegerSpinner1->Value());
			BTime time(fIntegerSpinner3->Value(), fIntegerSpinner4->Value(), fIntegerSpinner5->Value());
			BDateTime datetime(date, time);
			time_t unixtime = datetime.Time_t();

			if(fIsCreating)
				fDataMessage->AddData(fTextCtrlName->Text(), B_TIME_TYPE,
					&unixtime, sizeof(unixtime));
			else
				fDataMessage->ReplaceData(fDataLabel, B_TIME_TYPE, &unixtime,
					sizeof(unixtime));
			break;
		}

		default:
			return B_BAD_DATA;
	}

	return B_OK;
}

void
EditView::SetupControls()
{
	if(fIsCreating) {
		fMainLayout->AddView(fTextCtrlName);
	}

	switch(fDataType)
	{
		case B_AFFINE_TRANSFORM_TYPE:
		{
			double range_min = std::numeric_limits<double>::lowest();
			double range_max = std::numeric_limits<double>::max();

			fDecimalSpinner1->SetLabel(B_TRANSLATE("X"));
			fDecimalSpinner1->SetRange(range_min, range_max);
			fDecimalSpinner1->SetPrecision(8);
			fDecimalSpinner2->SetLabel(B_TRANSLATE("Y"));
			fDecimalSpinner2->SetRange(range_min, range_max);
			fDecimalSpinner2->SetPrecision(8);

			BView* translationView = new BView("transl_v", B_SUPPORTS_LAYOUT, new BGroupLayout(B_HORIZONTAL));
			((BGroupLayout*)translationView->GetLayout())->SetInsets(B_USE_SMALL_INSETS);
			translationView->GetLayout()->AddView(fDecimalSpinner1);
			translationView->GetLayout()->AddView(fDecimalSpinner2);
			BBox* translationBox = new BBox("");
			translationBox->SetLabel(B_TRANSLATE("Translation"));
			translationBox->AddChild(translationView);

			fDecimalSpinner3->SetLabel(B_TRANSLATE("X"));
			fDecimalSpinner3->SetRange(range_min, range_max);
			fDecimalSpinner3->SetPrecision(8);
			fDecimalSpinner4->SetLabel(B_TRANSLATE("Y"));
			fDecimalSpinner4->SetRange(range_min, range_max);
			fDecimalSpinner4->SetPrecision(8);

			BView* scaleView = new BView("scale_v", B_SUPPORTS_LAYOUT, new BGroupLayout(B_HORIZONTAL));
			((BGroupLayout*)scaleView->GetLayout())->SetInsets(B_USE_SMALL_INSETS);
			scaleView->GetLayout()->AddView(fDecimalSpinner3);
			scaleView->GetLayout()->AddView(fDecimalSpinner4);
			BBox* scaleBox = new BBox("");
			scaleBox->SetLabel(B_TRANSLATE("Scale"));
			scaleBox->AddChild(scaleView);

			fDecimalSpinner5->SetLabel(B_TRANSLATE("X"));
			fDecimalSpinner5->SetRange(range_min, range_max);
			fDecimalSpinner5->SetPrecision(8);
			fDecimalSpinner6->SetLabel(B_TRANSLATE("Y"));
			fDecimalSpinner6->SetRange(range_min, range_max);
			fDecimalSpinner6->SetPrecision(8);

			BView* shearView = new BView("shear_v", B_SUPPORTS_LAYOUT, new BGroupLayout(B_HORIZONTAL));
			((BGroupLayout*)shearView->GetLayout())->SetInsets(B_USE_SMALL_INSETS);
			shearView->GetLayout()->AddView(fDecimalSpinner5);
			shearView->GetLayout()->AddView(fDecimalSpinner6);
			BBox* shearBox = new BBox("");
			shearBox->SetLabel(B_TRANSLATE("Shear"));
			shearBox->AddChild(shearView);

			fMainLayout->AddView(translationBox);
			fMainLayout->AddView(scaleBox);
			fMainLayout->AddView(shearBox);
			break;
		}

		case B_ALIGNMENT_TYPE:
		{
			BLayoutBuilder::Menu<>(fPopUpMenu)
				.AddItem(B_TRANSLATE("Left"), EV_DATA_CHANGED)
				.AddItem(B_TRANSLATE("Right"), EV_DATA_CHANGED)
				.AddItem(B_TRANSLATE("Center"), EV_DATA_CHANGED)
				.AddSeparator()
				.AddItem(B_TRANSLATE("No horizontal alignment"), EV_DATA_CHANGED)
				.AddItem(B_TRANSLATE("Use full width"), EV_DATA_CHANGED)
			.End();

			BLayoutBuilder::Menu<>(fPopUpMenu2)
				.AddItem(B_TRANSLATE("Top"), EV_DATA_CHANGED)
				.AddItem(B_TRANSLATE("Middle"), EV_DATA_CHANGED)
				.AddItem(B_TRANSLATE("Bottom"), EV_DATA_CHANGED)
				.AddSeparator()
				.AddItem(B_TRANSLATE("No vertical alignment"), EV_DATA_CHANGED)
				.AddItem(B_TRANSLATE("Use full height"), EV_DATA_CHANGED)
			.End();

			fPopUpMenu->FindItem(B_TRANSLATE("No horizontal alignment"))->SetMarked(true);
			fPopUpMenu2->FindItem(B_TRANSLATE("No vertical alignment"))->SetMarked(true);

			BView* view = new BView(NULL, B_SUPPORTS_LAYOUT);
			BLayoutBuilder::Grid<>(view)
				.AddMenuField(new BMenuField(B_TRANSLATE("Horizontal alignment"), fPopUpMenu), 0, 0)
				.AddMenuField(new BMenuField(B_TRANSLATE("Vertical alignment"), fPopUpMenu2), 0, 1)
			.End();
			fMainLayout->AddView(view);
			break;
		}

		case B_BOOL_TYPE:
		{
			fRadioButton1->SetLabel(B_TRANSLATE("true"));
			fRadioButton2->SetLabel(B_TRANSLATE("false"));

			BView* view = new BView(NULL, B_SUPPORTS_LAYOUT);
			BLayoutBuilder::Group<>(view, B_VERTICAL)
				.SetExplicitAlignment(BAlignment(B_ALIGN_CENTER, B_ALIGN_MIDDLE))
				.Add(fRadioButton1)
				.Add(fRadioButton2)
			.End();

			fMainLayout->AddView(view);
			break;
		}

		case B_CHAR_TYPE:
		{
			auto range_min = std::numeric_limits<char>::lowest();
			auto range_max = std::numeric_limits<char>::max();

			fIntegerSpinner1->SetRange(range_min, range_max);

			fMainLayout->AddView(fIntegerSpinner1);
			fMainLayout->AddView(fSvDescription);
			break;
		}

		case B_DOUBLE_TYPE:
		{
			double range_min = std::numeric_limits<double>::lowest();
			double range_max = std::numeric_limits<double>::max();

			fDecimalSpinner1->SetRange(range_min, range_max);
			fDecimalSpinner1->SetPrecision(8);
			fDecimalSpinner1->SetStep(0.1);

			BString rangeText = BString("")
				.SetToFormat(B_TRANSLATE("Values from %e to %e." ),
					range_min, range_max);
			fSvDescription->SetText(rangeText.String());
			fSvDescription->SetFont(&fDescFont);
			fSvDescription->SetHighColor(fDescColor);

			fMainLayout->AddView(fDecimalSpinner1);
			fMainLayout->AddView(fSvDescription);

			break;
		}

		case B_FLOAT_TYPE:
		{
			float range_min = std::numeric_limits<float>::lowest();
			float range_max = std::numeric_limits<float>::max();

			fDecimalSpinner1->SetRange(range_min, range_max);
			fDecimalSpinner1->SetPrecision(8);
			fDecimalSpinner1->SetStep(0.1);

			BString rangeText = BString("")
				.SetToFormat(B_TRANSLATE("Values from %e to %e."),
					range_min, range_max);
			fSvDescription->SetText(rangeText.String());
			fSvDescription->SetFont(&fDescFont);
			fSvDescription->SetHighColor(fDescColor);

			fMainLayout->AddView(fDecimalSpinner1);
			fMainLayout->AddView(fSvDescription);

			break;
		}

		//all integer types get the same input field but need different range constraints
		case B_INT8_TYPE:
		case B_INT16_TYPE:
		case B_INT32_TYPE:
		case B_UINT8_TYPE:
		case B_UINT16_TYPE:
		case B_UINT32_TYPE:
		{
			int32 range_min = 0;
			int32 range_max = 0;

			switch(fDataType)
			{
				case B_INT8_TYPE:
					range_min=std::numeric_limits<int8>::lowest();
					range_max=std::numeric_limits<int8>::max();
					break;

				case B_INT16_TYPE:
					range_min=std::numeric_limits<int16>::lowest();
					range_max=std::numeric_limits<int16>::max();
					break;

				case B_INT32_TYPE:
					range_min=std::numeric_limits<int32>::lowest();
					range_max=std::numeric_limits<int32>::max();
					break;

				case B_UINT8_TYPE:
					range_min=std::numeric_limits<uint8>::lowest();
					range_max=std::numeric_limits<uint8>::max();
					break;

				case B_UINT16_TYPE:
					range_min=std::numeric_limits<uint16>::lowest();
					range_max=std::numeric_limits<uint16>::max();
					break;

				case B_UINT32_TYPE:
					range_min=std::numeric_limits<uint32>::lowest();
					range_max=2147483647;
					break;
			}

			fIntegerSpinner1->SetRange(range_min, range_max);

			BString rangeText = BString("")
				.SetToFormat(B_TRANSLATE("Values from %d to %d."),
				static_cast<int32>(range_min), static_cast<int32>(range_max));
			if(fDataType == B_UINT32_TYPE) {
				rangeText.Append(BString("").SetToFormat(B_TRANSLATE("\nValues "
				"from %u to %u cannot be represented\n\tdue to software limitations."),
				static_cast<uint32>(range_max) + 1, (uint32)std::numeric_limits<uint32>::max()));
			}
			fSvDescription->SetText(rangeText.String());
			fSvDescription->SetFont(&fDescFont);
			fSvDescription->SetHighColor(fDescColor);

			fMainLayout->AddView(fIntegerSpinner1);
			fMainLayout->AddView(fSvDescription);

			break;
		}

		case B_NODE_REF_TYPE:
		case B_REF_TYPE:
		{
			fTextCtrl1->TextView()->MakeEditable(false);
			BButton* btBrowse = new BButton(B_TRANSLATE("Browse"), new BMessage(EV_REF_REQUESTED));

			fDataViewer->SetExplicitMinSize(BSize(300, BRow().Height() * 5));
			fDataViewer->AddColumn(new BStringColumn(B_TRANSLATE("Field"),  100, 50, 200, 0), 0);
			fDataViewer->AddColumn(new BStringColumn(B_TRANSLATE("Value"),  100, 50, 200, 0), 1);

			BView* view = new BView(NULL, B_SUPPORTS_LAYOUT);
			BLayoutBuilder::Group<>(view, B_VERTICAL)
				.AddGroup(B_VERTICAL, B_USE_SMALL_SPACING, B_USE_SMALL_SPACING)
					.Add(new BStringView(NULL, B_TRANSLATE("Choose a filesystem entry to edit the data:")))
					.AddGrid(B_USE_SMALL_SPACING, B_USE_SMALL_SPACING)
						.Add(fTextCtrl1, 0, 0)
						.Add(btBrowse, 1, 0)
					.End()
				.End()
				.AddGroup(B_VERTICAL, B_USE_SMALL_SPACING, B_USE_SMALL_SPACING)
					.Add(new BStringView(NULL, B_TRANSLATE("Data preview:")))
					.Add(fDataViewer)
				.End()
			.End();

			fMainLayout->AddView(view);
			break;
		}

		case B_POINT_TYPE:
		{
			fDecimalSpinner1->SetLabel("X:");
			fDecimalSpinner1->SetMaxValue(10000);
			fDecimalSpinner2->SetLabel("Y:");
			fDecimalSpinner2->SetMaxValue(10000);

			fMainLayout->AddView(fDecimalSpinner1);
			fMainLayout->AddView(fDecimalSpinner2);

			fReusableButton1->SetLabel(B_TRANSLATE("Visual editor"));
			BMessage message(EV_MEASUREMENT_REQUESTED);
			fReusableButton1->SetMessage(new BMessage(message));
			fMainLayout->AddView(fReusableButton1);
			break;
		}

		case B_RECT_TYPE:
		{
			fDecimalSpinner1->SetLabel(B_TRANSLATE("Left:"));
			fDecimalSpinner1->SetMaxValue(10000);
			fDecimalSpinner2->SetLabel(B_TRANSLATE("Top:"));
			fDecimalSpinner2->SetMaxValue(10000);
			fDecimalSpinner3->SetLabel(B_TRANSLATE("Right:"));
			fDecimalSpinner3->SetMaxValue(10000);
			fDecimalSpinner4->SetLabel(B_TRANSLATE("Bottom:"));
			fDecimalSpinner4->SetMaxValue(10000);

			fMainLayout->AddView(fDecimalSpinner1);
			fMainLayout->AddView(fDecimalSpinner2);
			fMainLayout->AddView(fDecimalSpinner3);
			fMainLayout->AddView(fDecimalSpinner4);

			fReusableButton1->SetLabel(B_TRANSLATE("Visual editor"));
			BMessage message(EV_MEASUREMENT_REQUESTED);
			fReusableButton1->SetMessage(new BMessage(message));
			fMainLayout->AddView(fReusableButton1);
			break;
		}

		case B_RGB_COLOR_TYPE:
		{
			fIntegerSpinner1->SetLabel(B_TRANSLATE("Red:"));
			fIntegerSpinner1->SetRange(0, 255);
			fIntegerSpinner2->SetLabel(B_TRANSLATE("Green:"));
			fIntegerSpinner2->SetRange(0, 255);
			fIntegerSpinner3->SetLabel(B_TRANSLATE("Blue:"));
			fIntegerSpinner3->SetRange(0, 255);
			fIntegerSpinner4->SetLabel(B_TRANSLATE("Alpha:"));
			fIntegerSpinner4->SetRange(0, 255);

			fMainLayout->AddView(fIntegerSpinner1);
			fMainLayout->AddView(fIntegerSpinner2);
			fMainLayout->AddView(fIntegerSpinner3);
			fMainLayout->AddView(fIntegerSpinner4);

			break;
		}

		case B_SIZE_TYPE:
		{
			fDecimalSpinner1->SetLabel(B_TRANSLATE("Height:"));
			fDecimalSpinner1->SetMaxValue(10000);
			fDecimalSpinner2->SetLabel(B_TRANSLATE("Width:"));
			fDecimalSpinner2->SetMaxValue(10000);

			fMainLayout->AddView(fDecimalSpinner1);
			fMainLayout->AddView(fDecimalSpinner2);

			fReusableButton1->SetLabel(B_TRANSLATE("Visual editor"));
			BMessage message(EV_MEASUREMENT_REQUESTED);
			fReusableButton1->SetMessage(new BMessage(message));
			fMainLayout->AddView(fReusableButton1);

			break;
		}

		// case B_SIZE_T_TYPE:
		// case B_UINT64_TYPE:
		// {
			// fIntegerSpinner1->SetLabel(B_TRANSLATE("Size"));
			// fIntegerSpinner1->SetRange(0, std::numeric_limits<int32>::max()); 	// BSpinner bad design leaves out values

			// BString rangeText = BString("")
				// .SetToFormat(B_TRANSLATE("Values from %d to %d."),
				// static_cast<int32>(0), static_cast<int32>(std::numeric_limits<int32>::max()));
			// if(fDataType == B_UINT32_TYPE) {
				// rangeText.Append(BString("").SetToFormat(B_TRANSLATE("\nValues "
				// "from %lu to %lu cannot be represented\n\tdue to software limitations."),
				// static_cast<uint64>(std::numeric_limits<int32>::max()) + 1,
				// std::numeric_limits<uint64>::max()));
			// }
			// fSvDescription->SetText(rangeText.String());
			// fSvDescription->SetFont(&fDescFont);
			// fSvDescription->SetHighColor(fDescColor);

			// fMainLayout->AddView(fIntegerSpinner1);
			// fMainLayout->AddView(fSvDescription);
			// break;
		// }

		// case B_SSIZE_T_TYPE:
		// case B_INT64_TYPE:
		// {
			// fIntegerSpinner1->SetLabel(B_TRANSLATE("Size"));
			// fIntegerSpinner1->SetRange(std::numeric_limits<int32>::min(),
				// std::numeric_limits<int32>::max()); 	// BSpinner bad design leaves out values

			// BString rangeText = BString("")
				// .SetToFormat(B_TRANSLATE("Values from %d to %d."),
				// static_cast<int32>(std::numeric_limits<int32>::min()),
				// static_cast<int32>(std::numeric_limits<int32>::max()));
			// if(fDataType == B_UINT32_TYPE) {
				// rangeText.Append(BString("").SetToFormat(B_TRANSLATE("\nValues "
				// "from %lu to %lu and from %lu to %lu cannot be represented\n"
				// "\tdue to software limitations."),
				// std::numeric_limits<int64>::lowest(),
				// static_cast<uint64>(std::numeric_limits<int32>::lowest() - 1),
				// static_cast<uint64>(std::numeric_limits<int32>::max()) + 1,
				// std::numeric_limits<uint64>::max()));
			// }
			// fSvDescription->SetText(rangeText.String());
			// fSvDescription->SetFont(&fDescFont);
			// fSvDescription->SetHighColor(fDescColor);

			// fMainLayout->AddView(fIntegerSpinner1);
			// fMainLayout->AddView(fSvDescription);
			// break;
		// }

		case B_STRING_TYPE:
		{
			fTextCtrl1->SetText("");
			fMainLayout->AddView(fTextCtrl1);
			break;
		}

		case B_TIME_TYPE:
		{
			fIntegerSpinner1->SetLabel(B_TRANSLATE("DD"));
			fIntegerSpinner1->SetRange(1, 31);
			fIntegerSpinner2->SetLabel(B_TRANSLATE("MM"));
			fIntegerSpinner2->SetRange(1, 12);
			fTextCtrl1->SetLabel(B_TRANSLATE("YYYY"));
			for(int c = 0; c < 256; c++) {
				if(!(c >= '0' && c <= '9'))
					fTextCtrl1->TextView()->DisallowChar(c);
			}
			fIntegerSpinner3->SetLabel(B_TRANSLATE("HH"));
			fIntegerSpinner3->SetRange(0, 23);
			fIntegerSpinner4->SetLabel(B_TRANSLATE("MM"));
			fIntegerSpinner4->SetRange(0, 59);
			fIntegerSpinner5->SetLabel(B_TRANSLATE("SS"));
			fIntegerSpinner5->SetRange(0, 59);

			BView* timeContainerView = new BView(NULL, B_SUPPORTS_LAYOUT);
			BLayoutBuilder::Grid<>(timeContainerView)
				.Add(fTextCtrl1, 2, 0) /* year */
				.Add(fIntegerSpinner2, 1, 0) /* month */
				.Add(fIntegerSpinner1, 0, 0) /* day */
				.Add(fIntegerSpinner3, 0, 1) /* hour */
				.Add(fIntegerSpinner4, 1, 1) /* minute */
				.Add(fIntegerSpinner5, 2, 1) /* second */
			.End();
			fMainLayout->AddView(timeContainerView);

			fReusableButton1->SetLabel(B_TRANSLATE("Use current date and time"));
			fReusableButton1->SetMessage(new BMessage(EV_GET_CURRENT_TIME));
			fMainLayout->AddView(fReusableButton1);
			break;
		}

		/* Non editable data below */
		case B_VECTOR_ICON_TYPE:
		{
			fPreviewableBitmap = new BBitmap(BRect(0, 0, 300, 300), B_RGBA32);
			const void* data = NULL;
			ssize_t length = 0;
			fDataMessage->FindData(fDataLabel, B_VECTOR_ICON_TYPE, fDataIndex, &data, &length);
			BIconUtils::GetVectorIcon(static_cast<const uint8*>(data), length, fPreviewableBitmap);
			PreviewableView* bitmapView = new PreviewableView(BRect(0, 0, 127, 127), fPreviewableBitmap);
			fSvDescription->SetText(B_TRANSLATE("Preview:"));
			not_editable_text->SetFont(&fDescFont);
			not_editable_text->SetHighColor(fDescColor);

			fMainLayout->AddView(fSvDescription);
			fMainLayout->AddView(bitmapView);
			fMainLayout->AddView(not_editable_text);
			break;
		}

		default:
		{
			not_editable_text->SetFont(&fDescFont);
			not_editable_text->SetHighColor(fDescColor);
			fMainLayout->AddView(not_editable_text);
			fEditable = false;
			break;
		}
	}
}

void
EditView::InitControlsData()
{
	if(!fIsCreating) {
		switch(fDataType)
		{
			case B_AFFINE_TRANSFORM_TYPE:
			{
				const void* ptr = NULL;
				ssize_t length = 0;
				fDataMessage->FindData(fDataLabel, B_AFFINE_TRANSFORM_TYPE,
					fDataIndex, &ptr, &length);

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

				fDecimalSpinner1->SetValue(tx);
				fDecimalSpinner2->SetValue(ty);
				fDecimalSpinner3->SetValue(sx);
				fDecimalSpinner4->SetValue(sy);
				fDecimalSpinner5->SetValue(shx);
				fDecimalSpinner6->SetValue(shy);
				break;
			}
			case B_ALIGNMENT_TYPE:
			{
				BAlignment alignment;
				fDataMessage->FindAlignment(fDataLabel, fDataIndex, &alignment);

				BString menuItemLabel;
				switch(alignment.horizontal) {
					case B_ALIGN_LEFT:
						menuItemLabel = B_TRANSLATE("Left"); break;
					case B_ALIGN_RIGHT:
						menuItemLabel = B_TRANSLATE("Right"); break;
					case B_ALIGN_CENTER:
						menuItemLabel = B_TRANSLATE("Center"); break;
					case B_ALIGN_USE_FULL_WIDTH:
						menuItemLabel = B_TRANSLATE("Use full width"); break;
					case B_ALIGN_HORIZONTAL_UNSET:
					default:
						menuItemLabel = B_TRANSLATE("No horizontal alignment"); break;
				}
				fPopUpMenu->FindItem(menuItemLabel)->SetMarked(true);

				BString menuItemLabel2;
				switch(alignment.vertical) {
					case B_ALIGN_TOP:
						menuItemLabel2 = B_TRANSLATE("Top"); break;
					case B_ALIGN_MIDDLE:
						menuItemLabel2 = B_TRANSLATE("Middle"); break;
					case B_ALIGN_BOTTOM:
						menuItemLabel2 = B_TRANSLATE("Bottom"); break;
					case B_ALIGN_USE_FULL_HEIGHT:
						menuItemLabel2 = B_TRANSLATE("Use full height"); break;
					case B_ALIGN_VERTICAL_UNSET:
					default:
						menuItemLabel2 = B_TRANSLATE("No vertical alignment"); break;
				}
				fPopUpMenu2->FindItem(menuItemLabel2)->SetMarked(true);
				break;
			}
			case B_BOOL_TYPE:
			{
				bool data_bool = false;
				fDataMessage->FindBool(fDataLabel, fDataIndex, &data_bool);
				if(data_bool)
					fRadioButton1->SetValue(B_CONTROL_ON);
				else
					fRadioButton2->SetValue(B_CONTROL_ON);
				break;
			}
			case B_CHAR_TYPE:
			{
				char c;
				const void* ptr = NULL;
				ssize_t length = 0;
				fDataMessage->FindData(fDataLabel, B_CHAR_TYPE, fDataIndex, &ptr, &length);
				c = *(static_cast<const unsigned char*>(ptr));
				fIntegerSpinner1->SetValue(static_cast<int32>(c));

				BString description;
				if(isprint(static_cast<char>(c)) != 0)
					description << B_TRANSLATE("Character: ") << c;
				fSvDescription->SetText(description);
				break;
			}
			case B_DOUBLE_TYPE:
			{
				double data_double = 0.0;
				fDataMessage->FindDouble(fDataLabel, fDataIndex, &data_double);
				fDecimalSpinner1->SetValue(data_double);
				break;
			}
			case B_FLOAT_TYPE:
			{
				float data_float = 0.0f;
				fDataMessage->FindFloat(fDataLabel, fDataIndex, &data_float);
				fDecimalSpinner1->SetValue(data_float);
				break;
			}
			case B_INT8_TYPE:
			{
				int8 data_int8 = 0;
				fDataMessage->FindInt8(fDataLabel, fDataIndex, &data_int8);
				fIntegerSpinner1->SetValue(static_cast<int32>(data_int8));
				break;
			}
			case B_INT16_TYPE:
			{
				int16 data_int16 = 0;
				fDataMessage->FindInt16(fDataLabel, fDataIndex, &data_int16);
				fIntegerSpinner1->SetValue(static_cast<int32>(data_int16));
				break;
			}
			case B_INT32_TYPE:
			{
				int32 data_int32 = 0;
				fDataMessage->FindInt32(fDataLabel, fDataIndex, &data_int32);
				fIntegerSpinner1->SetValue(data_int32);
				break;
			}
			case B_MIME_TYPE:
			{
				const void* ptr = NULL;
				ssize_t length = 0;
				fDataMessage->FindData(fDataLabel, B_MIME_TYPE, fDataIndex, &ptr, &length);
				BString mimeString(static_cast<const char*>(ptr));
				fTextCtrl1->SetText(mimeString);
				break;
			}
			case B_NODE_REF_TYPE:
			{
				node_ref nref;
				fDataMessage->FindNodeRef(fDataLabel, fDataIndex, &nref);

				BRow* deviceRow = new BRow();
				deviceRow->SetField(new BStringField(B_TRANSLATE("Device")), 0);
				deviceRow->SetField(new BStringField(BString().SetToFormat("%" B_PRIdDEV, nref.device)), 1);
				fDataViewer->AddRow(deviceRow);

				BRow* nodeRow = new BRow();
				nodeRow->SetField(new BStringField(B_TRANSLATE("Node")), 0);
				nodeRow->SetField(new BStringField(BString().SetToFormat("%" B_PRIdINO, nref.node)), 1);
				fDataViewer->AddRow(nodeRow);

				fDataViewer->ResizeAllColumnsToPreferred();
				break;
			}
			case B_OFF_T_TYPE:
			{
				off_t offset = 0;
				const void* ptr = NULL;
				ssize_t length = 0;
				fDataMessage->FindData(fDataLabel, B_OFF_T_TYPE, fDataIndex, &ptr, &length);
				memcpy(&offset, ptr, length);
				fIntegerSpinner1->SetValue(static_cast<int32>(offset));
				break;
			}
			case B_POINT_TYPE:
			{
				BPoint data_point(0, 0);
				fDataMessage->FindPoint(fDataLabel, fDataIndex, &data_point);
				fDecimalSpinner1->SetValue(data_point.x);
				fDecimalSpinner2->SetValue(data_point.y);
				break;
			}
			case B_RECT_TYPE:
			{
				BRect data_rect(0, 0, 0, 0);
				fDataMessage->FindRect(fDataLabel, fDataIndex, &data_rect);
				fDecimalSpinner1->SetValue(data_rect.left);
				fDecimalSpinner2->SetValue(data_rect.top);
				fDecimalSpinner3->SetValue(data_rect.right);
				fDecimalSpinner4->SetValue(data_rect.bottom);
				break;
			}
			case B_REF_TYPE:
			{
				entry_ref ref;
				fDataMessage->FindRef(fDataLabel, fDataIndex, &ref);
				BEntry entry(&ref);
				BPath path;
				entry.GetPath(&path);

				if(entry.Exists()) {
					fTextCtrl1->SetText(path.Path());
				}

				BRow* deviceRow = new BRow();
				deviceRow->SetField(new BStringField(B_TRANSLATE("Device")), 0);
				deviceRow->SetField(new BStringField(BString().SetToFormat("%" B_PRIdDEV, ref.device)), 1);
				fDataViewer->AddRow(deviceRow);

				BRow* directoryRow = new BRow();
				directoryRow->SetField(new BStringField(B_TRANSLATE("Directory")), 0);
				directoryRow->SetField(new BStringField(BString().SetToFormat("%" B_PRIdINO, ref.directory)), 1);
				fDataViewer->AddRow(directoryRow);

				BRow* nameRow = new BRow();
				nameRow->SetField(new BStringField(B_TRANSLATE("Name")), 0);
				nameRow->SetField(new BStringField(ref.name), 1);
				fDataViewer->AddRow(nameRow);

				fDataViewer->ResizeAllColumnsToPreferred();
				break;
			}
			case B_RGB_COLOR_TYPE:
			{
				const void *color_ptr = NULL;
				ssize_t data_size = sizeof(rgb_color);
				fDataMessage->FindData(fDataLabel, B_RGB_COLOR_TYPE, fDataIndex, &color_ptr, &data_size);
				rgb_color data_rgbcolor = *(static_cast<const rgb_color*>(color_ptr));
				fIntegerSpinner1->SetValue(data_rgbcolor.red);
				fIntegerSpinner2->SetValue(data_rgbcolor.green);
				fIntegerSpinner3->SetValue(data_rgbcolor.blue);
				fIntegerSpinner4->SetValue(data_rgbcolor.alpha);
				break;
			}
			case B_SIZE_TYPE:
			{
				BSize data_size;
				fDataMessage->FindSize(fDataLabel, fDataIndex, &data_size);
				fDecimalSpinner1->SetValue(data_size.height);
				fDecimalSpinner2->SetValue(data_size.width);
				break;
			}
			// case B_SIZE_T_TYPE:
				// This value should be able to use the standard way of retrieving a unsigned long int value
				// break;
			// case B_SSIZE_T_TYPE:
				// This value should be able to use the standard way of retrieving a long int value
				// break;
			case B_STRING_TYPE:
			{
				const char *data_string;
				fDataMessage->FindString(fDataLabel, fDataIndex, &data_string);
				fTextCtrl1->SetText(data_string);
				break;
			}
			case B_TIME_TYPE:
			{
				const void* ptr = NULL;
				ssize_t length = 0;
				fDataMessage->FindData(fDataLabel, fDataType, fDataIndex, &ptr, &length);
				unsigned char* buffer = new unsigned char[length];
				memcpy(buffer, ptr, length);
				time_t time = *(reinterpret_cast<time_t*>(buffer));
				BDateTime datetime;
				datetime.SetTime_t(time);

				fIntegerSpinner1->SetValue(datetime.Date().Day());
				fIntegerSpinner2->SetValue(datetime.Date().Month());
				fTextCtrl1->SetText(BString().SetToFormat("%d", datetime.Date().Year()));
				fIntegerSpinner3->SetValue(datetime.Time().Hour());
				fIntegerSpinner4->SetValue(datetime.Time().Minute());
				fIntegerSpinner5->SetValue(datetime.Time().Second());

				delete[] buffer;
				break;
			}
			case B_UINT8_TYPE:
			{
				uint8 data_uint8 = 0;
				fDataMessage->FindUInt8(fDataLabel, fDataIndex, &data_uint8);
				fIntegerSpinner1->SetValue(static_cast<int32>(data_uint8));
				break;
			}
			case B_UINT16_TYPE:
			{
				uint16 data_uint16 = 0;
				fDataMessage->FindUInt16(fDataLabel, fDataIndex, &data_uint16);
				fIntegerSpinner1->SetValue(static_cast<int32>(data_uint16));
				break;
			}
			case B_UINT32_TYPE:
			{
				uint32 data_uint32 = 0;
				fDataMessage->FindUInt32(fDataLabel, fDataIndex, &data_uint32);
				fIntegerSpinner1->SetValue(static_cast<int32>(data_uint32));
				break;
			}
			default:
				break;
		}
	}
}

void
EditView::ValidateData()
{
	switch(fDataType)
	{
		case B_CHAR_TYPE:
		{
			char c = static_cast<char>(fIntegerSpinner1->Value());

			BString description;
			if(isprint(static_cast<char>(c)) != 0)
				description << B_TRANSLATE("Character: ") << c;
			else
				description.SetTo(B_TRANSLATE("Not previewable character"));

			fSvDescription->SetText(description);
			break;
		}
		case B_TIME_TYPE:
		{
			switch(fIntegerSpinner2->Value())
			{
				case 1:
				case 3:
				case 5:
				case 7:
				case 8:
				case 10:
				case 12:
					fIntegerSpinner1->SetMaxValue(31);
					break;
				case 4:
				case 6:
				case 9:
				case 11:
					fIntegerSpinner1->SetMaxValue(30);
					break;
				case 2:
				{
					int32 year = atoi(fTextCtrl1->Text());
					bool isLeap = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
					fIntegerSpinner1->SetMaxValue(isLeap ? 29 : 28);
					break;
				}
				default:
					break;
			}

			if(fIntegerSpinner1->Value() > fIntegerSpinner1->MaxValue())
				fIntegerSpinner1->SetValue(fIntegerSpinner1->MaxValue());
			break;
		default:
			break;
		}
	}
}


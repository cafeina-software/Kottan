/*
 * Copyright 2025 Cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 *
 */
#include <Catalog.h>
#include <String.h>
#include <View.h>
#include <Window.h>
#include <stdio.h>

#include "kottandefs.h"
#include "visualwindow.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "VisualWindow"

static BString text("");
static BString setFrameText(B_TRANSLATE("To edit the rect, move or resize the window."));
static BString setSizeText(B_TRANSLATE("To edit the size, resize the window."));
static BString setPointText(B_TRANSLATE("To edit the point, click inside the window."));
static BString saveText(B_TRANSLATE("To save and exit, press Command + S."));
static BString exitText(B_TRANSLATE("To exit without changes, press Command + W."));

VisualView::VisualView(BRect frame, type_code type)
: BView(frame, "visual_view", B_FOLLOW_ALL, B_WILL_DRAW),
  fType(type),
  fIsPointDragging(false)
{
	fTextFont = be_plain_font;

	if(type == B_POINT_TYPE)
		text = setPointText;
	else if(type == B_SIZE_TYPE)
		text = setSizeText;
	else if(type == B_RECT_TYPE)
		text = setFrameText;
}

void
VisualView::Draw(BRect updated)
{
	BView::Draw(updated);

	BRect bgFrame(Bounds());
	SetHighUIColor(B_DOCUMENT_BACKGROUND_COLOR);
	FillRect(bgFrame);

	SetHighUIColor(B_DOCUMENT_TEXT_COLOR, B_LIGHTEN_1_TINT);
	font_height height;
    GetFontHeight(&height);
	BPoint insertPoint;

	insertPoint.x = Bounds().Width() / 2 - (StringWidth(text.String()) / 2);
	insertPoint.y = Bounds().Height() / 2 - (height.ascent + height.leading + height.descent);
	DrawString(text.String(), insertPoint);

	insertPoint.x = Bounds().Width() / 2 - (StringWidth(saveText.String()) / 2);
	insertPoint.y = Bounds().Height() / 2 - (height.leading) / 2;
	DrawString(saveText.String(), insertPoint);

	insertPoint.x = Bounds().Width() / 2 - (StringWidth(exitText.String()) / 2);
	insertPoint.y = Bounds().Height() / 2 + (height.ascent + height.leading + height.descent);
	DrawString(exitText.String(), insertPoint);

	if(fType == B_POINT_TYPE) {
		float size = 25.0f;
		SetPenSize(2);

		/* Paint the shadow */
		auto offset = 2;
		SetHighUIColor(B_SHADOW_COLOR, B_LIGHTEN_2_TINT);
		StrokeEllipse(BRect(BPoint(fPointInternal.x + offset - size / 2, fPointInternal.y + offset - size / 2),
			BSize(size, size)));

		StrokeLine(BPoint(fPointInternal.x + offset, fPointInternal.y + offset - size),
			BPoint(fPointInternal.x + offset, fPointInternal.y + offset - size / 4));
		StrokeLine(BPoint(fPointInternal.x + offset, fPointInternal.y + 1 + offset + size / 4),
			BPoint(fPointInternal.x + offset, fPointInternal.y + offset + size));
		StrokeLine(BPoint(fPointInternal.x + offset - size, fPointInternal.y + offset),
			BPoint(fPointInternal.x + offset - size / 4, fPointInternal.y + offset));
		StrokeLine(BPoint(fPointInternal.x + 1 + offset + size / 4, fPointInternal.y + offset),
			BPoint(fPointInternal.x + offset + size, fPointInternal.y + offset));

		SetPenSize(4);
		StrokeLine(BPoint(fPointInternal.x + offset - 1, fPointInternal.y + offset),
			BPoint(fPointInternal.x + offset + 2, fPointInternal.y));
		StrokeLine(BPoint(fPointInternal.x + offset, fPointInternal.y + offset - 1),
			BPoint(fPointInternal.x + offset, fPointInternal.y + offset + 2));

		/* Paint the actual target pointer */
		offset = 1;
		SetPenSize(2);
		SetHighUIColor(B_FAILURE_COLOR);
		StrokeEllipse(BRect(BPoint(fPointInternal.x - size / 2, fPointInternal.y - size / 2), BSize(size, size)));

		StrokeLine(BPoint(fPointInternal.x + offset, fPointInternal.y + offset - size),
			BPoint(fPointInternal.x + offset, fPointInternal.y + offset - size / 4));
		StrokeLine(BPoint(fPointInternal.x + offset, fPointInternal.y + 1 + offset + size / 4),
			BPoint(fPointInternal.x + offset, fPointInternal.y + offset + size));
		StrokeLine(BPoint(fPointInternal.x + offset - size, fPointInternal.y + offset),
			BPoint(fPointInternal.x + offset - size / 4, fPointInternal.y + offset));
		StrokeLine(BPoint(fPointInternal.x + 1 + offset + size / 4, fPointInternal.y + offset),
			BPoint(fPointInternal.x + offset + size, fPointInternal.y + offset));

		SetPenSize(4);
		StrokeLine(BPoint(fPointInternal.x + offset - 1, fPointInternal.y + offset),
			BPoint(fPointInternal.x + offset + 2, fPointInternal.y));
		StrokeLine(BPoint(fPointInternal.x + offset, fPointInternal.y + offset - 1),
			BPoint(fPointInternal.x + offset, fPointInternal.y + offset + 2));

	}

	Invalidate();
}

void
VisualView::MouseDown(BPoint where)
{
	uint32 buttons;
	BPoint at;
	GetMouse(&at, &buttons, false);

	fIsPointDragging = true;

	if(buttons & B_PRIMARY_MOUSE_BUTTON) {
		fPointInternal = at;

		BMessage data(DM_DATA_RECEIVED);
		data.AddPoint("point", fPointInternal);
		Window()->PostMessage(&data);
	}

	BView::MouseDown(where);
}

void
VisualView::MouseUp(BPoint where)
{
	fIsPointDragging = false;

	BView::MouseUp(where);
}

void
VisualView::MouseMoved(BPoint where, uint32 code, const BMessage* dragMessage)
{
	BPoint at;
	uint32 buttons;
	GetMouse(&at, &buttons, false);

	if(fIsPointDragging && buttons & B_PRIMARY_MOUSE_BUTTON && code & B_INSIDE_VIEW) {
		fPointInternal = where;

		BMessage data(DM_DATA_RECEIVED);
		data.AddPoint("point", fPointInternal);
		Window()->PostMessage(&data);
	}

	BView::MouseMoved(where, code, dragMessage);
}

void
VisualView::KeyDown(const char* bytes, int32 numBytes)
{
	switch(bytes[0])
	{
		case B_UP_ARROW:
		case 'u':
			fPointInternal.y -= 1;
			break;
		case B_DOWN_ARROW:
		case 'd':
			fPointInternal.y += 1;
			break;
		case B_LEFT_ARROW:
		case 'l':
			fPointInternal.x -= 1;
			break;
		case B_RIGHT_ARROW:
		case 'r':
			fPointInternal.x += 1;
			break;
		default:
			BView::KeyDown(bytes, numBytes);
			break;
	}

	BMessage data(DM_DATA_RECEIVED);
	data.AddPoint("point", fPointInternal);
	Window()->PostMessage(&data);
}

void
VisualView::KeyUp(const char* bytes, int32 numBytes)
{
	BView::KeyUp(bytes, numBytes);
}

void
VisualView::SetPoint(BPoint point)
{
	fPointInternal = point;
}

void
VisualView::SetType(type_code type)
{
	fType = type;

	if(type == B_POINT_TYPE)
		text = setPointText;
	else if(type == B_SIZE_TYPE)
		text = setSizeText;
	else if(type == B_RECT_TYPE)
		text = setFrameText;
}

// #pragma mark - VisualWindow

VisualWindow::VisualWindow(BRect frame, BMessage* data, BLooper* target)
: BWindow(frame, "", B_DOCUMENT_WINDOW, B_ASYNCHRONOUS_CONTROLS),
  fLooper(target)
{
	fClientView = new VisualView(Bounds(), B_RECT_TYPE);
	AddChild(fClientView);
	fClientView->MakeFocus(true);

	AddShortcut('S', B_COMMAND_KEY, new BMessage(DM_DATA_SEND));
	AddShortcut('W', B_COMMAND_KEY, new BMessage(DM_CLOSE_REQUESTED));
}

VisualWindow::~VisualWindow()
{
}

void
VisualWindow::SetTo(BRect frame, BSize size, BPoint point, type_code type, BLooper* target)
{
	MoveTo(frame.LeftTop());
	BSize targetSize;
	if(type == B_SIZE_TYPE)
		targetSize = BSize(size.Width(), size.Height());
	else
		targetSize = BSize(frame.Size().Width(), frame.Size().Height());
	ResizeTo(targetSize.Width(), targetSize.Height());

	fType = type;
	fClientView->SetType(fType);

	if(fType == B_POINT_TYPE)
		fClientView->SetPoint(fPoint);

	fLooper = target;

	if(IsLocked())
		Unlock();
}

void
VisualWindow::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
		case DM_DATA_RECEIVED:
		{
			switch(fType)
			{
				case B_POINT_TYPE:
				{
					BPoint point;
					if(msg->FindPoint("point", &point) == B_OK) {
						fPoint = point;
						SetTitleWithData(fType);
					}
					break;
				}
				default:
					break;
			}
			break;
		}
		case DM_DATA_SEND:
		{
			BMessage message(msg->what);
			message.AddUInt32(KottanFieldType, static_cast<uint32>(fType));

			switch(fType)
			{
				case B_RECT_TYPE:
					message.AddRect("frame", Frame());
					break;
				case B_POINT_TYPE:
					message.AddPoint("point", fPoint);
					break;
				case B_SIZE_TYPE:
					message.AddSize("size", Frame().Size());
					break;
				default:
					break;
			}

			if(!message.IsEmpty() && fLooper) {
				fLooper->PostMessage(&message);
			}
			QuitRequested();
			break;
		}
		case DM_CLOSE_REQUESTED:
		case B_QUIT_REQUESTED:
		{
			QuitRequested();
			break;
		}
		default:
			BWindow::MessageReceived(msg);
			break;
	}
}

bool
VisualWindow::QuitRequested()
{
	BMessage message(DM_CLOSE_REQUESTED);
	fLooper->PostMessage(&message);
	Hide();
	return false;
}

void
VisualWindow::FrameMoved(BPoint newPosition)
{
	SetTitleWithData(fType);
	BWindow::FrameMoved(newPosition);
}

void
VisualWindow::FrameResized(float newWidth, float newHeight)
{
	SetTitleWithData(fType);
	BWindow::FrameResized(newWidth, newHeight);
}

void
VisualWindow::Show()
{
	if(!IsLocked())
		Lock();
	EnableUpdates();
	BWindow::Show();
	if(IsLocked())
		Unlock();
}

void
VisualWindow::Hide()
{
	BWindow::Hide();
	DisableUpdates();
}

void
VisualWindow::SetTitleWithData(type_code type)
{
	BString title(B_TRANSLATE("Preview: "));

	switch(type)
	{
		case B_RECT_TYPE:
		{
			title << BString().SetToFormat("BRect(l:%.1f, t:%.1f, r:%.1f, b:%.1f)",
						Frame().left, Frame().top, Frame().right, Frame().bottom);
			break;
		}
		case B_SIZE_TYPE:
		{
			title << BString().SetToFormat("BSize(w:%.1f, h:%.1f)",
						Frame().Size().Width(), Frame().Size().Height());
			break;
		}
		case B_POINT_TYPE:
		{
			title << BString().SetToFormat("BPoint(x:%.1f, y:%.1f)",
						fPoint.x, fPoint.y);
			break;
		}
		default:
			break;
	}

	SetTitle(title);
}

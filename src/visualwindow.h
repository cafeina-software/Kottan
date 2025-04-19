/*
 * Copyright 2025 Cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 *
 */
#ifndef __DUMMY_WINDOW_H__
#define __DUMMY_WINDOW_H__

#include <Window.h>

enum VisualWndCmds {
	DM_DATA_RECEIVED = 'atad',
	DM_DATA_SEND = 'data',
	DM_CLOSE_REQUESTED = 'clse'
};

class VisualView : public BView
{
public:
					VisualView(BRect frame, type_code type);

	virtual void	Draw(BRect updated);
	virtual void	MouseDown(BPoint where);
	virtual	void	MouseUp(BPoint where);
	virtual	void	MouseMoved(BPoint where, uint32 code,
						const BMessage* dragMessage);
	virtual	void	KeyDown(const char* bytes, int32 numBytes);
	virtual	void	KeyUp(const char* bytes, int32 numBytes);

			void	SetPoint(BPoint point);
			void 	SetType(type_code type);
private:
	type_code		fType;
	BFont			fTextFont;
	BPoint			fPointInternal;
	bool			fIsPointDragging;
};

class VisualWindow : public BWindow
{
public:
					VisualWindow(BRect frame, BMessage* data, BLooper* target);
	virtual			~VisualWindow();

			void	SetTo(BRect frame, BSize size, BPoint point, type_code type, BLooper* target);

	virtual void	MessageReceived(BMessage* msg);
	virtual	bool	QuitRequested();
	virtual	void	FrameMoved(BPoint newPosition);
	virtual	void	FrameResized(float newWidth, float newHeight);
	virtual void	Show();
	virtual void 	Hide();

			void	SetTitleWithData(type_code type);
private:
	type_code		fType;
	BLooper*		fLooper;
	BPoint			fPoint;

	VisualView*      fClientView;
};

#endif /* __DUMMY_WINDOW_H__ */

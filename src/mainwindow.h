/*
 * Copyright 2019-2021 Andi Machovec <andi.machovec@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 *
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Window.h>
#include <MenuBar.h>
#include <FilePanel.h>

#include "messageview.h"

enum
{
	MW_MENU_ABOUT ='mw00',
	MW_OPEN_MESSAGEFILE,
	MW_SAVE_MESSAGEFILE,
	MW_REF_MESSAGEFILE,
	MW_ENTERED_MESSAGEFILE,
	MW_INSPECTMESSAGEFILE,
	MW_OPEN_REPLY,
	MW_ROW_SELECTED,
};

class MainWindow : public BWindow {
public:
	MainWindow(float left, float top, float right, float bottom);
	~MainWindow();
	void MessageReceived(BMessage *msg);
	bool QuitRequested();

private:
	void inspect_message_file();
			
	BMenuBar			*fTopMenuBar;
	MessageView			*fMessageInfoView;
	BFilePanel			*fOpenFilePanel;
	
};

#endif

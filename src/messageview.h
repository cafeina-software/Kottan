/*
 * Copyright 2019-2021 Andi Machovec <andi.machovec@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 *
 */

#ifndef MESSAGEVIEW_H
#define MESSAGEVIEW_H

#include <private/interface/ColumnListView.h>
#include <Message.h>


enum
{
	MV_ROW_CLICKED ='mv00',
	MV_SELECTION_CHANGED
};


class MessageView : public BColumnListView {
public:
	MessageView();
	void 			SetDataMessage(BMessage *message);
	virtual	void	MessageDropped(BMessage* msg, BPoint point);
	void 			UpdateData();

private:
	void create_data_rows(BMessage *message, BRow *parent = NULL);
	BMessage *fDataMessage;
};

#endif

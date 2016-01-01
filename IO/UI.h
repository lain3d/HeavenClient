/////////////////////////////////////////////////////////////////////////////
// This file is part of the Journey MMORPG client                           //
// Copyright � 2015 Daniel Allendorf                                        //
//                                                                          //
// This program is free software: you can redistribute it and/or modify     //
// it under the terms of the GNU Affero General Public License as           //
// published by the Free Software Foundation, either version 3 of the       //
// License, or (at your option) any later version.                          //
//                                                                          //
// This program is distributed in the hope that it will be useful,          //
// but WITHOUT ANY WARRANTY; without even the implied warranty of           //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            //
// GNU Affero General Public License for more details.                      //
//                                                                          //
// You should have received a copy of the GNU Affero General Public License //
// along with this program.  If not, see <http://www.gnu.org/licenses/>.    //
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Element.h"
#include "Components\Textfield.h"
#include "Components\Icon.h"

namespace IO
{
	namespace UI
	{
		void draw(float inter);
		void update();

		void sendmouse(vector2d<int16_t> pos);
		void sendmouse(bool pressed, vector2d<int16_t> pos);
		void doubleclick(vector2d<int16_t> pos);
		void sendkey(int32_t keycode, bool pressed);

		void showstatus(Textlabel::Textcolor color, string message);
		void showbuff(int32_t buffid, int32_t duration);
		void cancelbuff(int32_t buffid);
		void focustextfield(Textfield*);
		void dragicon(Icon*);

		void addkeymapping(uint8_t no, uint8_t type, int32_t action);
		void enablegamekeys(bool enable);
		void enable();
		void disable();

		void add(const Element& type);
		void remove(Element::UIType type);

		UIElement* getelement(Element::UIType type);

		template <class T>
		T* getelement(Element::UIType type)
		{
			UIElement* element = getelement(type);
			if (element)
				return reinterpret_cast<T*>(element);
			else
				return nullptr;
		}
	}
}
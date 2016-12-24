// ============================================================================
//  Copyright (C) 2001-2016 by Jozef Starosczyk
//  ixen@copyhandler.com
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Library General Public License
//  (version 2) as published by the Free Software Foundation;
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public
//  License along with this program; if not, write to the
//  Free Software Foundation, Inc.,
//  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// ============================================================================
#ifndef __TSHAREDCOUNT_H__
#define __TSHAREDCOUNT_H__

#include <memory>

namespace chengine
{
	template<class T>
	class TSharedCount
	{
	public:
		explicit TSharedCount(T initialValue = 0) : m_tCounter(initialValue)
		{
		}

		TSharedCount(const TSharedCount& rSrc) = delete;

		// assignment operator
		TSharedCount& operator=(const TSharedCount& rSrc) = delete;

		// conversion from/to T
		explicit operator T() const
		{
			return GetValue();
		}

		TSharedCount& operator=(T newValue)
		{
			SetValue(newValue);
			return *this;
		}

		// get/set value
		T GetValue() const
		{
			return m_tCounter;
		}

		void SetValue(T newValue)
		{
			m_tCounter = newValue;
			m_notifier();
		}

		void Increase()
		{
			Increase(1);
		}

		void Decrease()
		{
			Decrease(1);
		}

		void Increase(T addValue)
		{
			m_tCounter += addValue;
			m_notifier();
		}

		void Decrease(T addValue)
		{
			m_tCounter -= addValue;
			m_notifier();
		}

		boost::signals2::signal<void()>& GetNotifier()
		{
			return m_notifier;
		}

	private:
		T m_tCounter = 0;
		boost::signals2::signal<void()> m_notifier;
	};

	template<class T>
	using TSharedCountPtr = std::shared_ptr<TSharedCount<T>>;
}

#endif

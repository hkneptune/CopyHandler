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
#ifndef __TSHAREDCOUNTMT_H__
#define __TSHAREDCOUNTMT_H__

#include <memory>
#include <boost/thread/mutex.hpp>
#include <boost/signals2/signal.hpp>

namespace chcore
{
	template<class T>
	class TSharedCountMT
	{
	public:
		explicit TSharedCountMT(T initialValue = 0) : m_tCounter(initialValue)
		{
		}

		TSharedCountMT(const TSharedCountMT& rSrc) = delete;

		// assignment operator
		TSharedCountMT& operator=(const TSharedCountMT& rSrc) = delete;

		// conversion from/to T
		operator T() const
		{
			return GetValue();
		}

		TSharedCountMT& operator=(T newValue)
		{
			SetValue(newValue);
			return *this;
		}

		// get/set value
		T GetValue() const
		{
			boost::unique_lock<boost::mutex> lock(m_lock);
			return m_tCounter;
		}

		void SetValue(T newValue)
		{
			{
				boost::unique_lock<boost::mutex> lock(m_lock);
				m_tCounter = newValue;
			}
			m_notifier();
		}

		// increment/decrement
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
			{
				boost::unique_lock<boost::mutex> lock(m_lock);
				m_tCounter += addValue;
			}
			m_notifier();
		}

		void Decrease(T addValue)
		{
			{
				boost::unique_lock<boost::mutex> lock(m_lock);
				m_tCounter -= addValue;
			}
			m_notifier();
		}

		boost::signals2::signal<void()>& GetNotifier()
		{
			return m_notifier;
		}

	private:
		T m_tCounter = 0;
		boost::signals2::signal<void()> m_notifier;
		mutable boost::mutex m_lock;
	};

	template<class T>
	using TSharedCountMTPtr = std::shared_ptr<TSharedCountMT<T>>;
}

#endif

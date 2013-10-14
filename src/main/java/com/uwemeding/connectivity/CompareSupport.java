/*
 * Copyright (c) 2013 Uwe B. Meding <uwe@uwemeding.com>
 *
 * This file is part of UM/ISO
 * This PCA software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Final Term is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with UM/ISO.  If not, see <http://www.gnu.org/licenses/>.
 */
package com.uwemeding.connectivity;

import javax.swing.event.*;

/**
 * CompareSupport Event Handler
 *
 * @author uwe
 */
public abstract class CompareSupport {

	protected final EventListenerList listeners;

	/**
	 * Construct a CompareSupport
	 */
	protected CompareSupport() {
		listeners = new EventListenerList();
	}

	/**
	 * Create a detailed event
	 *
	 * @param type the event type
	 * @param reportType the report type
	 * @param graphNumber the graph number
	 * @param message the message
	 * @param nodes the nodes involved
	 * @return the event
	 */
	public CompareEvent createEvent(int type, int reportType, int graphNumber, String message, Object[] nodes) {
		return new CompareEvent(type, reportType, graphNumber, message, nodes);
	}

	/**
	 * Create an event from a message
	 *
	 * @param message is the message
	 * @return the event
	 */
	public CompareEvent createEvent(String message) {
		return new CompareEvent(message);
	}

	/**
	 * Add a CompareListener
	 *
	 * @param l the CompareListener
	 */
	public void addCompareListener(CompareListener l) {
		listeners.add(CompareListener.class, l);
	}

	/**
	 * See whether we have CompareListeners
	 *
	 * @return true if we have CompareListeners
	 */
	public boolean haveCompareListeners() {
		return (listeners.getListenerCount(CompareListener.class) > 0);
	}

	/**
	 * Get rid of a CompareListener
	 *
	 * @param l the listener you want to remove
	 */
	public void removeCompareListener(CompareListener l) {
		listeners.remove(CompareListener.class, l);
	}

	/**
	 * Get rid of all CompareListeners
	 *
	 * @param l the listener you want to remove
	 */
	public void removeAllCompareListeners() {
		Object[] ll = listeners.getListenerList();
		for (int i = ll.length - 2; i >= 0; i -= 2) {
			if (ll[i] == CompareListener.class) {
				CompareListener l = (CompareListener) ll[i + 1];
				removeCompareListener(l);
			}
		}
	}

	/**
	 * Invoked when the target of the listener has a progress notification.
	 *
	 * @param e a CompareEvent object.
	 */
	protected void fireProgress(CompareEvent event) {
		synchronized (listeners) {
			Object[] ll = listeners.getListenerList();
			for (int i = ll.length - 2; i >= 0; i -= 2) {
				if (ll[i] == CompareListener.class) {
					CompareListener l = (CompareListener) ll[i + 1];
					l.progress(event);
				}
			}
		}
	}

	/**
	 * Invoked when the target of the listener has a warning notification.
	 *
	 * @param e a CompareEvent object.
	 */
	protected void fireWarning(CompareEvent event) {
		synchronized (listeners) {
			Object[] ll = listeners.getListenerList();
			for (int i = ll.length - 2; i >= 0; i -= 2) {
				if (ll[i] == CompareListener.class) {
					CompareListener l = (CompareListener) ll[i + 1];
					l.warning(event);
				}
			}
		}
	}

	/**
	 * Invoked when the target of the listener has a status notification.
	 *
	 * @param e a CompareEvent object.
	 */
	protected void fireStatus(CompareEvent event) {
		synchronized (listeners) {
			Object[] ll = listeners.getListenerList();
			for (int i = ll.length - 2; i >= 0; i -= 2) {
				if (ll[i] == CompareListener.class) {
					CompareListener l = (CompareListener) ll[i + 1];
					l.status(event);
				}
			}
		}
	}
}

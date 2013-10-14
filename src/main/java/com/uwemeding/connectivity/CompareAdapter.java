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

/**
 * CompareAdapter Event Object: The adapter which receives CompareEvents The
 * methods in this class are empty; this class is provided as a convenience for
 * easily creating listeners by extending this class and overriding only the
 * methods of interest.
 *
 * @author uwe
 */
public abstract class CompareAdapter implements CompareListener {

	/**
	 * Invoked when the target of the listener has a progress notification.
	 *
	 * @param e a CompareEvent object.
	 */
	public void progress(CompareEvent e) {
	}

	/**
	 * Invoked when the target of the listener has a cardinalityViolation
	 * notification.
	 *
	 * @param e a CompareEvent object.
	 */
	public void warning(CompareEvent e) {
	}

	/**
	 * Invoked when the target of the listener has a status notification.
	 *
	 * @param e a CompareEvent object.
	 */
	public void status(CompareEvent e) {
	}
}

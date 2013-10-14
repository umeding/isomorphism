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
 * This class is the main interface to the compare engine.
 *
 * @author uwe
 */
public class Compare extends CompareAPI {

	private static final boolean DEBUG = false;

	/**
	 * Constructor
	 */
	public Compare() {
		super.initialize();
	}

	/**
	 * Reset the engine
	 */
	public void reset() throws CompareException {
		initialize();
	}
}

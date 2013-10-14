/*
 * Copyright (c) 2013 Uwe B. Meding <uwe@uwemeding.com>
 *
 * This file is part of UM/ISO
 * This PCA software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Isomorphism is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with UM/ISO.  If not, see <http://www.gnu.org/licenses/>.
 */
package com.uwemeding.connectivity;

/**
 * Compare Exception Object
 *
 * @author uwe
 */
public class CompareException extends RuntimeException {

	public CompareException() {
	}

	public CompareException(String message) {
		super(message);
	}

	public CompareException(String message, Throwable cause) {
		super(message, cause);
	}

	public CompareException(Throwable cause) {
		super(cause);
	}

	public CompareException(String message, Throwable cause, boolean enableSuppression, boolean writableStackTrace) {
		super(message, cause, enableSuppression, writableStackTrace);
	}
}

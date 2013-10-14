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
 * The main program
 *
 * @author uwe
 */
public class Main {

	public static void main(String... av) {

		Compare compare = new Compare();
		// lots of detail
		compare.setTrace(true);

		compare.addCompareListener(new CompareListener() {
			public void progress(CompareEvent e) {
				log("progess", e);
			}

			public void warning(CompareEvent e) {
				log("warning", e);
			}

			public void status(CompareEvent e) {
				log("status", e);
			}
		});

		// --- Try a few things, just to make sure it works

		// Master 
		compare.defineDeviceMaster("M1", new String[]{"a", "b", "c"});

		// Instances
		compare.defineDeviceVertex(0, "M1", "I1", new String[]{"x", "y", "nc"});
		compare.defineDeviceVertex(0, "M1", "I2", new String[]{"x", "x", "y"});

        // Mess the connectivity up a little so we can see it work
		compare.defineDeviceVertex(1, "M1", "I1", new String[]{"x", "y", "nc"});
		compare.defineDeviceVertex(1, "M1", "I2", new String[]{"x", "y", "y"});

		compare.execute();
	}

	private static void log(String prefix, CompareEvent e) {
		System.out.println(prefix + ": " + e.getMessage());
	}
}

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
 * This class is the API to the comparison backend.
 *
 * @author uwe
 */
public class CompareAPI extends CompareSupport {

	public static final int GRAPH1 = 0;
	public static final int GRAPH2 = 1;
	private long env;

	/**
	 * Constructor
	 *
	 * @param xxx text
	 * @throws xxx
	 */
	public CompareAPI() {
		System.loadLibrary("isoc");
		initialize();
	}

	/**
	 * When the finalizer is called for this object we'll forward the cleanup
	 * request to the C-world also
	 */
	@Override
	@SuppressWarnings("FinalizeDeclaration")
	public void finalize() {
		try {
			super.finalize();
		} catch (Throwable ex) {
		}
		finalizeEnv();
	}

	//////////////////////////////////////////////////////
	// Native methods
	//////////////////////////////////////////////////////
	private native synchronized void finalizeEnv();

	public final native synchronized void initialize();

	public final native synchronized void execute();

	public final native synchronized void defineEquate(String name1, String name2);

	public final native synchronized void defineNetAlias(int graphNumber, String name, String[] aliases);

	public final native synchronized void defineDeviceMaster(String name, String[] connections);

	public final native synchronized void defineDeviceVertex(int graphNumber, String name, Object instObject, String[] connections);

	public final native synchronized void setGraphName(int graphNumber, String name);

	public final native synchronized String getGraphName(int graphNumber);

	public final native synchronized int getDeduceNeighbors();

	public final native synchronized void setDeduceNeighbors(int depth);

	public final native synchronized boolean getTrace();

	public final native synchronized void setTrace(boolean mode);
}

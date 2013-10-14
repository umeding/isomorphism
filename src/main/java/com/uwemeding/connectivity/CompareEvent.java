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

import java.util.EventObject;

/**
 * CompareEvent Event Object
 *
 * @author uwe
 */
public class CompareEvent extends EventObject {

	public static final int DEVICE = 0;
	public static final int NET = 1;
	public static final int RPT_GENERAL = 0;
	public static final int RPT_MATCH = 1;
	public static final int RPT_BAD = 2;
	public static final int RPT_NO_MATCH_OTHER = 3;
	public static final int RPT_NO_MATCH_SYMMETRY = 4;
	private int type;
	private int reportType;
	private int graphNumber;
	private String message;
	private Object[] nodes;

	/**
	 * Constructs a CompareEvent object.
	 */
	protected CompareEvent(int type, int reportType, int graphNumber, String message, Object[] nodes) {
		super("");
		this.type = type;
		this.reportType = reportType;
		this.graphNumber = graphNumber;
		this.message = message;
		this.nodes = nodes;
	}

	/**
	 * Constructs a CompareEvent object.
	 *
	 * @param beanInfo the object that is the source of the event
	 * @param instance is user data for the event
	 */
	protected CompareEvent(String message) {
		super("");
		this.type = RPT_GENERAL;
		this.message = message;
	}

	/**
	 * Get the vertex type this event applies to
	 *
	 * @return the vertex type
	 */
	public int getType() {
		return (type);
	}

	/**
	 * Get the report type.
	 *
	 * @return the report type
	 */
	public int getReportType() {
		return (reportType);
	}

	/**
	 * Get the graph number.
	 *
	 * @return the graph number
	 */
	public int getGraphNumber() {
		return (graphNumber);
	}

	/**
	 * Get the message.
	 *
	 * @return the message
	 */
	public String getMessage() {
		return (message);
	}

	/**
	 * Get the nodes.
	 *
	 * @return the nodes
	 */
	public Object[] getNodes() {
		return (nodes);
	}

	/**
	 * Get a string for this event.
	 *
	 * @return the string for this event.
	 */
	@Override
	public String toString() {
		return ("CompareEvent: message=" + message);
	}
}

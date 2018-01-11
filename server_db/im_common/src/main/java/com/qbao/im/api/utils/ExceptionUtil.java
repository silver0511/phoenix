/**
 * 
 */
package com.qbao.im.api.utils;

import java.io.PrintWriter;
import java.io.StringWriter;

/**
 * @author andersen
 *
 */
public class ExceptionUtil {
	public static String printStackTraceToString(Throwable t) {
	    StringWriter sw = new StringWriter();
	    t.printStackTrace(new PrintWriter(sw, true));
	    return sw.getBuffer().toString();
	}
}

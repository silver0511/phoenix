/**
 * 
 */
package com.qbao.im.api.controller;

import javax.ws.rs.Path;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Controller;

import com.qbao.im.api.handler.GroupHandler;

/**
 * @author andersen
 *
 */
@Controller
@Path("/api/manage")
public abstract class ManageController {
	
	private Logger logger = LoggerFactory.getLogger(ManageController.class);
	
	protected void sayLog(){
		logger.info("execute manage function");
	}
	
}

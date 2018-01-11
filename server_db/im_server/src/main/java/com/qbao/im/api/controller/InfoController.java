package com.qbao.im.api.controller;

import javax.ws.rs.GET;
import javax.ws.rs.POST;
import javax.ws.rs.Path;
import javax.ws.rs.Produces;
import javax.ws.rs.core.MediaType;

import org.springframework.stereotype.Controller;

/**
 * 
 * @author andersen
 *
 */
@Controller
@Path("/")
public class InfoController {
	@GET
	@POST
    @Path("main")
    @Produces(MediaType.APPLICATION_JSON)
    public String info(){
		return "ok";
	}
}

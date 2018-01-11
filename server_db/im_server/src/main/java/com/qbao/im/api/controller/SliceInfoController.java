package com.qbao.im.api.controller;

import javax.ws.rs.GET;
import javax.ws.rs.Path;
import javax.ws.rs.Produces;
import javax.ws.rs.QueryParam;
import javax.ws.rs.core.MediaType;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;

import com.qbao.im.api.entity.SliceInfo;
import com.qbao.im.api.handler.SliceInfoHandler;

/**
 * Created by anderson on 2017/9/27.
 */
@Controller
@Path("/si")
public class SliceInfoController {

    private Logger logger = LoggerFactory.getLogger(SliceInfoController.class);
    
    
    @Autowired
	private SliceInfoHandler sliceInfoHandler;

    @GET
    @Path("/i")
    @Produces(MediaType.APPLICATION_JSON)
    public String getInfo(@QueryParam("t")String table, @QueryParam("v")Long value) {
    	return sliceInfoHandler.getSliceInfoBySingleKey(table, value).toString();
    }
}

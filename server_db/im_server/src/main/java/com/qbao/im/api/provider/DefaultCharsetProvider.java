package com.qbao.im.api.provider;


import org.apache.commons.io.IOUtils;
import org.jboss.resteasy.plugins.providers.multipart.InputPart;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Component;
import sun.nio.ch.IOUtil;

import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.container.ContainerRequestFilter;
import javax.ws.rs.ext.Provider;
import java.io.*;

@Provider
//@Component
public class DefaultCharsetProvider implements ContainerRequestFilter{

	private Logger logger = LoggerFactory.getLogger(DefaultCharsetProvider.class);

	@Override
	public void filter(ContainerRequestContext requestContext) throws IOException {
		if(requestContext.getMethod().equals("GET")){
			logger.info("request params:{},url:{}"+requestContext.getUriInfo().getRequestUri(),requestContext.getUriInfo().getQueryParameters().get("userid"));
		}else {
			ByteArrayOutputStream baos = new ByteArrayOutputStream();
			IOUtils.copy(requestContext.getEntityStream(),baos);
			byte[] bytes = baos.toByteArray();
			logger.info("request params:{},url:{}"+requestContext.getUriInfo().getRequestUri(),new String(bytes,"UTF-8"));
		}
		requestContext.setProperty(InputPart.DEFAULT_CHARSET_PROPERTY, "UTF-8");
	}

	public String getPostParam(InputStream inputStream) throws Exception{
		String requestBody = "";
		StringBuilder stringBuilder = new StringBuilder();
		BufferedReader bufferedReader = null;
		try {
			if (inputStream != null) {
				bufferedReader = new BufferedReader(new InputStreamReader(inputStream));
				char[] charBuffer = new char[128];
				int bytesRead = -1;
				while ((bytesRead = bufferedReader.read(charBuffer)) > 0) {
					stringBuilder.append(charBuffer, 0, bytesRead);
				}
			} else {
				stringBuilder.append("");
			}
		} catch (IOException ex) {
			throw ex;
		} finally {
			if (bufferedReader != null) {
				try {
					bufferedReader.close();
				} catch (IOException ex) {
					throw ex;
				}
			}
		}
		requestBody = stringBuilder.toString();
		return requestBody;
	}
}

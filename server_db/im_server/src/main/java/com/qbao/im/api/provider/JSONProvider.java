package com.qbao.im.api.provider;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Component;

import javax.ws.rs.Consumes;
import javax.ws.rs.Produces;
import javax.ws.rs.WebApplicationException;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.MultivaluedMap;
import javax.ws.rs.ext.MessageBodyReader;
import javax.ws.rs.ext.MessageBodyWriter;
import javax.ws.rs.ext.Provider;
import java.io.*;
import java.lang.annotation.Annotation;
import java.lang.reflect.Type;

/**
 * Created by tangxiaojun on 2017/5/19.
 */
@Provider
//@Component
//@Produces(MediaType.APPLICATION_JSON)
//@Consumes(MediaType.APPLICATION_JSON)
@Produces("application/x-java-serialized-object")
@Consumes("application/x-java-serialized-object")
public class JSONProvider implements MessageBodyReader<Serializable>, MessageBodyWriter<Serializable>{

    private Logger logger = LoggerFactory.getLogger(JSONProvider.class);

    public static final MediaType APPLICATION_SERIALIZABLE_TYPE = new MediaType("application","x-java-serialized-object");

    public static final String APPLICATION_SERIALIZABLE =  APPLICATION_SERIALIZABLE_TYPE.toString();

    @Override
    public boolean isReadable(Class<?> type, Type genericType, Annotation[] annotations, MediaType mediaType) {
//        return false;
        return Serializable.class.isAssignableFrom(type)
                && APPLICATION_SERIALIZABLE_TYPE.getType().equals(mediaType.getType())
                && APPLICATION_SERIALIZABLE_TYPE.getSubtype().equals(mediaType.getSubtype());
    }

    @Override
    public Serializable readFrom(Class<Serializable> type, Type genericType, Annotation[] annotations, MediaType mediaType, MultivaluedMap<String, String> httpHeaders, InputStream entityStream) throws IOException, WebApplicationException {
//        return null;
        BufferedInputStream bis = new BufferedInputStream(entityStream);
        ObjectInputStream ois = new ObjectInputStream(bis);
        try
        {
            return Serializable.class.cast(ois.readObject());
        }catch (ClassNotFoundException e)
        {
            throw new WebApplicationException(e);
        }

    }

    @Override
    public boolean isWriteable(Class<?> type, Type genericType, Annotation[] annotations, MediaType mediaType) {
//        return false;
        return Serializable.class.isAssignableFrom(type)
                && APPLICATION_SERIALIZABLE_TYPE.getType().equals(mediaType.getType())
                && APPLICATION_SERIALIZABLE_TYPE.getSubtype().equals(mediaType.getSubtype());
    }

    @Override
    public long getSize(Serializable serializable, Class<?> type, Type genericType, Annotation[] annotations, MediaType mediaType) {
        return -1;
    }

    @Override
    public void writeTo(Serializable serializable, Class<?> type, Type genericType, Annotation[] annotations, MediaType mediaType, MultivaluedMap<String, Object> httpHeaders, OutputStream entityStream) throws IOException, WebApplicationException {
        BufferedOutputStream bos = new BufferedOutputStream(entityStream);
        ObjectOutputStream oos = new ObjectOutputStream(bos);
        oos.writeObject(serializable);
        oos.close();
    }
}

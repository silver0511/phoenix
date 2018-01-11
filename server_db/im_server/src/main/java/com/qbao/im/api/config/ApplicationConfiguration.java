package com.qbao.im.api.config;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.config.PropertyPlaceholderConfigurer;
import org.springframework.boot.autoconfigure.EnableAutoConfiguration;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.core.env.Environment;
import org.springframework.core.io.ClassPathResource;
import org.springframework.core.io.FileSystemResource;
import org.springframework.core.io.Resource;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by tangxiaojun on 2017/6/2.
 */
//@Configuration
public class ApplicationConfiguration {

//    @Autowired
//    private Environment environment;

    //@Bean
    public static PropertyPlaceholderConfigurer properties() {
        final PropertyPlaceholderConfigurer ppc = new PropertyPlaceholderConfigurer();
        ppc.setIgnoreResourceNotFound(true);
        final List<Resource> resourceLst = new ArrayList<Resource>();

        if(System.getProperty("NIM_DB_CONF") != null){
            String configFilePath = System.getProperty("NIM_DB_CONF");
            /*String[] configFiles = configFilePath.split(",|;");

            FileSystemResource res =null;
            for (String configFile : configFiles) {
                if (configFile.startsWith("file:")){
                    resourceLst.add(new FileSystemResource(configFile));
                }else {
                    resourceLst.add( new ClassPathResource(configFile));
                }
            }*/
            resourceLst.add(new FileSystemResource(configFilePath.concat("/application.properties")));
            resourceLst.add(new FileSystemResource(configFilePath.concat("/jdbc-common.properties")));
            resourceLst.add(new FileSystemResource(configFilePath.concat("/im_jdbc.properties")));
            resourceLst.add(new FileSystemResource(configFilePath.concat("/redis.properties")));
        }else {
            resourceLst.add(new ClassPathResource("application.properties"));
            resourceLst.add(new ClassPathResource("jdbc-common.properties"));
            resourceLst.add(new ClassPathResource("/resources/${env}/im_jdbc.properties"));
            resourceLst.add(new ClassPathResource("/resources/${env}/redis.properties"));

        }
        ppc.setLocations(resourceLst.toArray(new Resource[]{}));
        return ppc;
//        return null;
    }
}

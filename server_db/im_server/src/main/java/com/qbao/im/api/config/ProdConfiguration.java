package com.qbao.im.api.config;

import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.EnableAspectJAutoProxy;
import org.springframework.context.annotation.ImportResource;
import org.springframework.context.annotation.Profile;

@EnableAspectJAutoProxy
@Configuration
@Profile("prod")
@ImportResource(locations = "classpath:/prod/applicationContext_prod.xml")
public class ProdConfiguration {

}

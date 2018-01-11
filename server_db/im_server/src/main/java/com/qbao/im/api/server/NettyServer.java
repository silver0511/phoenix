package com.qbao.im.api.server;

import com.qbao.im.api.util.SpringContextUtil;
import org.jboss.resteasy.spi.ResteasyDeployment;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.context.ApplicationContext;
import org.springframework.core.env.Environment;
import org.springframework.stereotype.Component;
import org.springframework.stereotype.Controller;

import javax.annotation.PostConstruct;
import javax.annotation.PreDestroy;
import javax.ws.rs.ext.Provider;
import java.util.Collection;

/**
 * Created by tangxiaojun on 2017/5/12.
 */
@Component
@ConfigurationProperties(prefix = "server")
public class NettyServer {

    private Logger logger = LoggerFactory.getLogger(NettyServer.class);

    @Autowired
    public SpringContextUtil springContextUtil;

    public String rootPath = "/";

    public int port;

    public ConfigurableNettyJaxrsServer netty;

    @PostConstruct
    public void start() {
        @SuppressWarnings("static-access")
        ApplicationContext ac = springContextUtil.getApplicationContext();
        ResteasyDeployment dp = new ResteasyDeployment();
        Collection<Object> providers = ac.getBeansWithAnnotation(Provider.class).values();
        Collection<Object> controllers = ac.getBeansWithAnnotation(Controller.class).values();
        if (providers != null) {
            dp.getProviders().addAll(providers);
        }

        dp.getResources().addAll(controllers);
        netty = new ConfigurableNettyJaxrsServer();
        netty.initBootstrap().setOption("reuseAddress", true);
        netty.setDeployment(dp);
        netty.setPort(port);
        netty.setRootResourcePath(this.rootPath);
        netty.setSecurityDomain(null);
        logger.info(port + "netty start");
        netty.start();
    }

    @PreDestroy
    public void cleanUp() {
        logger.error("服务器停止");
        netty.stop();
    }

    public String getRootPath() {
        return rootPath;
    }

    public void setRootPath(String rootPath) {
        this.rootPath = rootPath;
    }

    public int getPort() {
        return port;
    }

    public void setPort(int port) {
        this.port = port;
    }
}

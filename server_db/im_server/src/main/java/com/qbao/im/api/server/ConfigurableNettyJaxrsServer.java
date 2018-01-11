package com.qbao.im.api.server;

import org.jboss.netty.bootstrap.Bootstrap;
import org.jboss.netty.bootstrap.ServerBootstrap;
import org.jboss.netty.channel.ChannelHandler;
import org.jboss.netty.channel.ChannelPipelineFactory;
import org.jboss.netty.channel.group.ChannelGroup;
import org.jboss.netty.channel.group.DefaultChannelGroup;
import org.jboss.netty.channel.socket.nio.NioServerSocketChannelFactory;
import org.jboss.resteasy.core.SynchronousDispatcher;
import org.jboss.resteasy.plugins.server.netty.HttpServerPipelineFactory;
import org.jboss.resteasy.plugins.server.netty.HttpsServerPipelineFactory;
import org.jboss.resteasy.plugins.server.netty.NettyJaxrsServer;
import org.jboss.resteasy.plugins.server.netty.RequestDispatcher;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import javax.net.ssl.SSLContext;
import java.net.InetSocketAddress;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.Executors;

/**
 * Created by tangxiaojun on 2017/5/12.
 */
public class ConfigurableNettyJaxrsServer extends NettyJaxrsServer {

    public Logger logger = LoggerFactory.getLogger(ConfigurableNettyJaxrsServer.class);

    /**
     * 根据cpu核数设置优化nio线程数量
     */
    private final int ioWorkerCount = Runtime.getRuntime().availableProcessors() * 2;
//            private final int ioWorkerCount = 100;
    private final int executorThreadCount = 50;
    private SSLContext sslContext;
    private final int maxRequestSize = 1024 * 1024 * 10;
    private boolean isKeepAlive = true;
    static final ChannelGroup allChannels = new DefaultChannelGroup("NettyJaxrsServer");
    private List<ChannelHandler> channelHandlers = Collections.emptyList();

    public Bootstrap initBootstrap() {
        this.bootstrap = new ServerBootstrap(new NioServerSocketChannelFactory(Executors.newCachedThreadPool(),
                Executors.newCachedThreadPool(), ioWorkerCount));
        return bootstrap;
    }

    public void setBootstrap(ServerBootstrap bootstrap) {
        this.bootstrap = bootstrap;
    }

    @Override
    public void start() {
        deployment.start();
        RequestDispatcher dispatcher = new RequestDispatcher((SynchronousDispatcher) deployment.getDispatcher(),
                deployment.getProviderFactory(), domain);
        if (bootstrap == null) {
            initBootstrap();
        }
        ChannelPipelineFactory factory;
        if (sslContext == null) {
            factory = new HttpServerPipelineFactory(dispatcher, root, executorThreadCount, maxRequestSize, isKeepAlive,
                    channelHandlers);
        } else {
            factory = new HttpsServerPipelineFactory(dispatcher, root, executorThreadCount, maxRequestSize, isKeepAlive,
                    channelHandlers, sslContext);
        }

        bootstrap.setPipelineFactory(factory);
        channel = bootstrap.bind(new InetSocketAddress(port));
        allChannels.add(channel);
    }
}

package com.qbao.im.api.util;

import org.springframework.beans.BeansException;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;
import org.springframework.stereotype.Component;

/**
 * Created by tangxiaojun on 2017/5/12.
 */
@Component
public class SpringContextUtil implements ApplicationContextAware {

    private static ApplicationContext applicationContext = null;

    @Override
    public void setApplicationContext(ApplicationContext applicationContext) throws BeansException {
        SpringContextUtil.applicationContext = applicationContext;
    }

    public static ApplicationContext getApplicationContext() {
        return applicationContext;
    }

    @SuppressWarnings("unchecked")
    public static <T> T getBeanById(String id) {
        return (T) applicationContext.getBean(id);
    }

    @SuppressWarnings("unchecked")
    public static <T> T getBeanByClass(Class<?> c) {
        return (T) applicationContext.getBean(c);
    }
}

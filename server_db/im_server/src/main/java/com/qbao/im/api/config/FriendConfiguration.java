package com.qbao.im.api.config;

import org.springframework.boot.context.properties.ConfigurationProperties;

/**
 * Created by tangxiaojun on 2017/8/22.
 */
@ConfigurationProperties(prefix = "friend")
public class FriendConfiguration {

    private Integer limit;

    public Integer getLimit() {
        return limit;
    }

    public void setLimit(Integer limit) {
        this.limit = limit;
    }
}

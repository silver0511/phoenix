package com.qbao.im.api.entity;

import java.io.Serializable;

/**
 * Created by tangxiaojun on 2017/3/28.
 */
public class BaseEntity implements Serializable {

    private static final long serialVersionUID = 1L;

    private Long id;

    private transient String tabName;

    private Integer pageIndex;

    private Integer pageSize;

    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public String getTabName() {
        return tabName;
    }

    public void setTabName(String tabName) {
        this.tabName = tabName;
    }

    public Integer getPageIndex() {
        return pageIndex;
    }

    public void setPageIndex(Integer pageIndex) {
        this.pageIndex = pageIndex;
    }

    public Integer getPageSize() {
        return pageSize;
    }

    public void setPageSize(Integer pageSize) {
        this.pageSize = pageSize;
    }
}

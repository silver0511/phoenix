package com.qbao.im.api.dao;

import com.qbao.im.api.entity.BatchTest;

import java.util.List;
import java.util.Map;

/**
 * Created by tangxiaojun on 2017/6/20.
 */
public interface IBatch {

    void batchSave(Map<String , Object> map);
}

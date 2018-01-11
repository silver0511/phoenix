package com.qbao.im.api.service.batch;

import com.qbao.im.api.entity.BatchTest;

import java.util.List;

/**
 * Created by tangxiaojun on 2017/6/20.
 */
public interface IBatchService {

    void batch(Long groupId,List<BatchTest> list);
}

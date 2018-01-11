package com.qbao.im.api.service.impl;

import com.qbao.im.api.dao.IBatch;
import com.qbao.im.api.entity.BatchTest;
import com.qbao.im.api.service.batch.IBatchService;
import com.qianbao.framework.datasource.annotation.DataSource;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Created by tangxiaojun on 2017/6/20.
 */
@Service
public class BatchServiceImpl implements IBatchService {

    @Autowired
    private IBatch iBatch;

    @Override
    public void batch(@DataSource(field = "groupId") Long groupId, List<BatchTest> list) {
        Map<String , Object> map = new HashMap<>();
        map.put("tabName",list.get(0).getTabName());
        map.put("list",list);
        this.iBatch.batchSave(map);
    }
}

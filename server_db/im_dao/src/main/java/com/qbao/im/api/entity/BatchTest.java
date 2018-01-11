package com.qbao.im.api.entity;

import com.qbao.im.api.DBEnum;

import java.util.Date;

/**
 * Created by tangxiaojun on 2017/6/20.
 */
public class BatchTest extends BaseEntity {

    private Long groupid;

    private Long userid;

    private Date createtime;

    public Long getGroupid() {
        return groupid;
    }

    public void setGroupid(Long groupid) {
        this.groupid = groupid;
        Long modVal = this.groupid%8;
        String modValStr = modVal+"";
        String tabName = null;
        if(modValStr.length()>1){
            tabName = DBEnum.BATCH_TEST.getValue().concat(modVal+"");
        }else {
            tabName = DBEnum.BATCH_TEST.getValue().concat("0"+modVal+"");
        }
        super.setTabName(tabName);
    }

    public Long getUserid() {
        return userid;
    }

    public void setUserid(Long userid) {
        this.userid = userid;
    }

    public Date getCreatetime() {
        return createtime;
    }

    public void setCreatetime(Date createtime) {
        this.createtime = createtime;
    }

    @Override
    public String getTabName() {
        Long modVal = this.groupid %8;
        String modValStr = modVal+"";
        String tabName = null;
        if(modValStr.length()>1){
            tabName = DBEnum.BATCH_TEST.getValue().concat(modVal+"");
        }else {
            tabName = DBEnum.BATCH_TEST.getValue().concat("0"+modVal+"");
        }
        return tabName;
    }
}

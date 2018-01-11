package com.qbao.im.api.msg;

/**
 * Created by tangxiaojun on 2017/3/23.
 */
public class ClientMessage extends BaseMessage{
    // 对象信息
    private Object data;

    private Integer responseCode = ErrorCode.RET_SUCCESS;

    public Object getData() {
        return data;
    }

    public void setData(Object data) {
        this.data = data;
    }

    public Integer getResponseCode() {
        return responseCode;
    }

    public void setResponseCode(Integer responseCode) {
        this.responseCode = responseCode;
    }

    public static class Builder {
        private Object data;

        private Integer responseCode;

        private String user_id;

        public Builder data(Object data){
            this.data = data;
            return this;
        }

        public Builder responseCode(Integer responseCode){
            this.responseCode = responseCode;
            return this;
        }

        public Builder user_id(String userId){
            this.user_id = userId;
            return this;
        }

        public ClientMessage build(){
            return new ClientMessage(this);
        }
    }

    private ClientMessage(Builder builder){
        super(builder.user_id);
        this.data = builder.data;
        this.responseCode = builder.responseCode;
    }
}

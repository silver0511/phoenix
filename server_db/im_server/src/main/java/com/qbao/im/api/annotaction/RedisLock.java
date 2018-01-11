package com.qbao.im.api.annotaction;

import java.lang.annotation.*;

/**
 * Created by tangxiaojun on 2017/6/22.
 */
@Target({ElementType.METHOD})
@Retention(RetentionPolicy.RUNTIME)
@Inherited
public @interface RedisLock {

    String value();

    long keepMills() default 20*1000;

    LockFailAction action() default  LockFailAction.GIVEUP;

    public enum LockFailAction{
        /**
         * 放弃
         */
        GIVEUP,
        /**
         * 继续
         */
        CONTINUE;
    }

    long sleepMills() default 10;

    long maxSleepMills() default 60*1000;
}

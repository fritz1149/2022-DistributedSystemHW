package com.ds.publisher.shell;

import org.apache.rocketmq.spring.core.RocketMQTemplate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.shell.standard.ShellComponent;
import org.springframework.shell.standard.ShellMethod;
import org.springframework.shell.standard.ShellOption;

@ShellComponent
public class SendMessageCMD {
    @Autowired
    private RocketMQTemplate rocketMQTemplate;
    @ShellMethod("send many message concurrently")
    public String send(
            @ShellOption int num
    ) {
        long startTime=System.currentTimeMillis();
        Runnable sender= () -> rocketMQTemplate.convertAndSend("atopic","hello");
        for (int i=0;i<num;i++){
            new Thread(sender).start();
        }
        long endTime=System.currentTimeMillis();
        return "send "+num +" messages cost "+(endTime-startTime)+"ms";
    }
}

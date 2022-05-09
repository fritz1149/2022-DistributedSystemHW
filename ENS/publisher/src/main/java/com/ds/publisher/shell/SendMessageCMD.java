package com.ds.publisher.shell;

import org.apache.rocketmq.client.exception.MQBrokerException;
import org.apache.rocketmq.client.exception.MQClientException;
import org.apache.rocketmq.common.message.Message;
import org.apache.rocketmq.remoting.exception.RemotingException;
import org.apache.rocketmq.spring.core.RocketMQTemplate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.shell.standard.ShellComponent;
import org.springframework.shell.standard.ShellMethod;
import org.springframework.shell.standard.ShellOption;

import java.nio.charset.StandardCharsets;

@ShellComponent
public class SendMessageCMD {
    @Autowired
    private RocketMQTemplate rocketMQTemplate;
    @ShellMethod("send many message concurrently")
    public String send(
            @ShellOption int num
    ) throws MQBrokerException, RemotingException, InterruptedException, MQClientException {
        long startTime=System.currentTimeMillis();
        Message broad=new Message("broad","broadcasting","broadcasting-message-body".getBytes(StandardCharsets.UTF_8));
        rocketMQTemplate.getProducer().send(broad);
        Runnable sender= () -> rocketMQTemplate.convertAndSend("atopic","hello");
        for (int i=0;i<num;i++){
            new Thread(sender).start();
        }
        long endTime=System.currentTimeMillis();
        return "send "+num +" messages cost "+(endTime-startTime)+"ms";
    }
    @ShellMethod("send a broadcast message")
    public String broadcast(
            @ShellOption(defaultValue = "broad") String topic,
            @ShellOption(defaultValue = "broadcasting-message-body")String content
    ) throws MQBrokerException, RemotingException, InterruptedException, MQClientException {
        Message broad=new Message(topic,"broadcasting",content.getBytes(StandardCharsets.UTF_8));
        rocketMQTemplate.getProducer().send(broad);
        return "broad success";
    }
}

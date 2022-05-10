package com.ds.consumer.shell;

import io.netty.channel.DefaultChannelId;
import org.apache.rocketmq.client.consumer.DefaultMQPushConsumer;
import org.apache.rocketmq.client.consumer.listener.ConsumeConcurrentlyContext;
import org.apache.rocketmq.client.consumer.listener.ConsumeConcurrentlyStatus;
import org.apache.rocketmq.client.consumer.listener.MessageListenerConcurrently;
import org.apache.rocketmq.client.exception.MQClientException;
import org.apache.rocketmq.common.message.MessageExt;
import org.apache.rocketmq.common.utils.NameServerAddressUtils;
import org.springframework.shell.standard.ShellComponent;
import org.springframework.shell.standard.ShellMethod;
import org.springframework.shell.standard.ShellOption;

import java.util.List;

@ShellComponent
public class ReceiveMessageCMD {
    DefaultMQPushConsumer consumer;

    public ReceiveMessageCMD() throws MQClientException {
        consumer = new DefaultMQPushConsumer("consumer-group");
        String namesrvAddr = NameServerAddressUtils.getNameServerAddresses();
        System.out.println("namesrvAddr: " + namesrvAddr);
        // 配置里面nameserver地址好像到不了这
        consumer.setNamesrvAddr("10.134.142.150:9876");
        consumer.registerMessageListener(new MessageListenerConcurrently() {
            @Override
            public ConsumeConcurrentlyStatus consumeMessage(List<MessageExt> msgs, ConsumeConcurrentlyContext context) {
                for(MessageExt msg : msgs)
                    System.out.println(new String(msg.getBody()));
                // 标记该消息已经被成功消费
                return ConsumeConcurrentlyStatus.CONSUME_SUCCESS;
            }
        });
        //防止超时
        DefaultChannelId.newInstance();
        consumer.start();
    }

    @ShellMethod("subscribe a topic")
    public String subscribe(@ShellOption String topic) throws MQClientException {
        consumer.subscribe(topic, "*");
        return "OK";
    }

    @ShellMethod("unsubscribe a topic")
    public String unsubscribe(@ShellOption String topic){
        consumer.unsubscribe(topic);
        return "OK";
    }
}

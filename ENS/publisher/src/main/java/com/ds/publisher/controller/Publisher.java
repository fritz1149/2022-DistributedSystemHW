package com.ds.publisher.controller;

import org.apache.rocketmq.spring.core.RocketMQTemplate;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;

import javax.annotation.Resource;

@RestController
public class Publisher {
    @Resource
    private RocketMQTemplate rocketMQTemplate;
    @GetMapping("/send")
    public String sendMessage(){
        rocketMQTemplate.convertAndSend("atopic","hello?");
        return "OK";
    }
}

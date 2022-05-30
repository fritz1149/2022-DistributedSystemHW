## 分布式系统第3次实验报告

| 姓名   | 学号     |
| ------ | -------- |
| 常程   | 19231149 |
| 李宇轩 | 19231151 |

### raft原理解析
Raft是一种用于替代Paxos的共识算法。相比于Paxos，Raft的目标是提供更清晰的逻辑分工使得算法本身能被更好地理解。

Raft确保如下的特性得以保证：

1. 选举安全：一个term内最多一个leader被选出。
2. Leader只追加：一个Leader永远不会覆写、删除其日志中的条目，只会追加新条目。
3. 日志匹配：如果两个日志包含一个具有相同term和index的条目，则这两个日志中在该index处及之前的所有条目都是相同的。
4. Leader完整性：如果一个日志条目在一个指定的term内被提交，那么在所有term更高的Leader的日志内都会有该条目。
5. 状态机安全性：如果一个Server将一个指定index处的日志条目应用于其状态机，其他Server不会在该index处应用不同的日志条目。

Raft功能的实现主要包括几个部分：Leader选举、日志复制、安全保障、成员变更、日志压缩。

#### Leader选举

初始：一个Server在启动时，初始角色为Follower。

心跳机制：如果一个Follower在“选举超时”之前，都没有收到来自Leader或Candidate的心跳消息，则其转变为Candidate并且开始新一轮选举。

超时机制：每个Follower在每一个term的”选举超时“都是随机的，每个Candidate的”选举超时“也都是随机的，就保证了每个新term总是只有一个Server进入选举状态。

投票规则：

1. 收到多数Server投票则当选（保证了选举安全性质）
2. 投过票的Follower不能在一个term中更换投票对象
3. Follower会给最先向其拉票的（最大term或index不小于自己的）Candidate投票

选举过程：

1. 一个Follower增加自身的term，转变角色为Candidate

2. 向每个其他Server发送拉票RPC

3. 若收到多数Server投票，则当选为Leader，并向所有其他Server发送消息确立地位。

	若收到来自Leader的消息，认证后确定该Leader合法，则转为Follower

	若选举时间超时，且既未当选，也未收到新Leader消息，则开启下一轮选举。

#### 日志复制

日志条项元信息：

1. term：即任期，每次旧Leader失联、开始选举新Leader时都会使term加一；每个日志条项都记录其被提出的term。
2. index：即编号，这个参数是连续的，表示每一个条项都在日志中有绝对的位置。

同步机制：

1. Client向集群内任一Server发送请求，该请求都会被转发至Leader。
2. Leader会将请求追加至自己的日志中。
3. Leader会向Follower发送追加日志的RPC，RPC中会携带追加日志区间之前一个条项的元信息，Follower若能匹配该元信息则追加日志，否则Leader需要将日志区间向前调整并且再次尝试。
4. Follower与Leader冲突的日志，将被Leader的日志覆写。

提交机制：

1. Leader会统计复制到超过半数的Server上的日志条项，将它们记为已提交
2. Follower接到追加日志的RPC时，会读取最后的已提交的日志条项的编号，据此在本机上将应该提交的条项应用到状态机中

#### 安全保障

1. 选举限制：Candidate必须得到超过半数的投票才能上任，而Server只有在Candidate的日志比自己更全时才会投票，又提交的日志条项必须已在半数的Server上复制，这使得上任的Leader一定具有所有已提交的日志条项。

2. 提交之前term的条项的机制：

	只能通过计数方式提交当前term的条项，这种方式不允许提交之前term的条项；一旦提交某条项，其前面的条项默认条件，这种方式允许提交当前term、之前term的条项。

	若允许通过计数方式提交之前term的条项，那么若在某个server上存在某一个条项，其index与刚提交的相同，且当前Leader挂掉，该Server当选，则可能会把刚提交的条项覆盖，违反了raft的一致性原则。

#### 成员变更

问题：在成员变更时，由于群组成员信息很难同时应用到所有Server中，因此可能出现脑裂的情况。

<img src="https://cdn.inlighting.org/blog/2022-02-21-5kRMTZ.png!slim" alt="multi-leader" style="zoom:80%;float:left" />

为了防止这种情况，raft提出了两种方法：Joint Consensus以及单步成员变更。

Joint Consensus：

1. 向集群提出 $C_{old,new}$ 的配置日志，此后所有日志都要得到持有 $C_{old}$ 配置的集群多数派以及持有 $C_{new}$ 配置的集群多数派（持有 $C_{old,new}$ 也算作此列）的确认。（这一步即使发生脑裂，也不会造成分歧）
2. $C_{old,new}$ 日志已经提交后，再提出 $C_{new}$ 配置的日志。（此时已经不具备脑裂的条件了）

单步成员变更：可以证明，在每次只变更一个成员时，旧集群的多数派和新集群的多数派一定存在交集，因此就不可能发生脑裂。

#### 日志压缩

目的：

1. 防止日志无限增长，占据空间。
2. 日志过长时，重放日志耗费时间过长。

制作快照的时机：日志文件超过给定的限制时。

快照规则：

1. 只对已提交的日志制作快照。
2. 快照保留其所含日志区间内最后一条日志条项的元信息。
3. 快照制作后，将其所包含的日志区间删除。
4. 当新节点加入或落后太多的旧节点需要同步时，Leader会向其发送快照以令其快速同步。

### 技术实现&功能解析

#### Leader选举

![image-20220530154303960](C:\Users\Pc\AppData\Roaming\Typora\typora-user-images\image-20220530154303960.png)

每个server以follower的状态启动

![image-20220530154443113](C:\Users\Pc\AppData\Roaming\Typora\typora-user-images\image-20220530154443113.png)

当心跳计时器超时发送拉票请求

![image-20220530155053421](C:\Users\Pc\AppData\Roaming\Typora\typora-user-images\image-20220530155053421.png)

当candidate获得超过半数选票则上台

![image-20220530155116173](C:\Users\Pc\AppData\Roaming\Typora\typora-user-images\image-20220530155116173.png)

处理拉票请求：如果自己任期比请求里的任期小，说明自己过期，下台；如果自己在当前任期还没投过票，就接受拉票请求（谁先来就投给谁）

![image-20220530155235475](C:\Users\Pc\AppData\Roaming\Typora\typora-user-images\image-20220530155235475.png)

#### 日志复制

leader每轮向所有follower发送AppendEntries请求

![image-20220530160418258](C:\Users\Pc\AppData\Roaming\Typora\typora-user-images\image-20220530160418258.png)

![image-20220530160604276](C:\Users\Pc\AppData\Roaming\Typora\typora-user-images\image-20220530160604276.png)

其中prevIndex和prevTerm是追加日志区间前一项的元数据；通过BATCH_SIZE来调整每次请求最多可以追加多少条日志。

#### 请求重定向

![image-20220530161124959](C:\Users\Pc\AppData\Roaming\Typora\typora-user-images\image-20220530161124959.png)

在请求到达follower时，应重定向到leader

#### 应用：计数器

在先前实现的选举服务的基础上，为每个server增加两个RPC，分别是increase和query

increase：将状态机的计数器+1

query：查询状态机的计数器的值

![image-20220530163108742](C:\Users\Pc\AppData\Roaming\Typora\typora-user-images\image-20220530163108742.png)

![image-20220530163127696](C:\Users\Pc\AppData\Roaming\Typora\typora-user-images\image-20220530163127696.png)

#### 实验：不稳定的网络环境

在发送消息的方法中加入随机数，使每条消息以5%的概率丢失。模拟三十秒的时间，总共进行了16轮选举，term增加了13。其中心跳超时时限为0.25秒，一次消息传输时间为0.01秒。

![image-20220530161641090](C:\Users\Pc\AppData\Roaming\Typora\typora-user-images\image-20220530161641090.png)


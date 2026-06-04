# Todo List

## 主代码

[x] 配置转字符串
[x] 真正使用配置调整游戏
[x] 配置读取拆分成不同部分（defaults, io, validator）
[ ] 给预加载界面加个游戏名
[ ] 设计 logo
[ ] 实现加载界面
[ ] 调整日志的时间显示格式
[ ] 实现主界面
[ ] 实现配置界面
[ ] 实现游戏界面
[ ] ~~移动全盘交给 handler，判极简和拖地次数~~
[ ] （后期任务）实现关卡特殊配置
[x] 着急的：给 map_entry 加一个 monostate 并处理带来的一万个问题
[ ] 1 2 3 5 连块形状，6 连块命名、存储、形状
[ ] 消行文本（写 localization）
[x] 翻新 localization，提供按序尝试的模式
[ ] （后期任务）多线程。方案一：逻辑帧操作时设锁，渲染帧先等锁复制然后渲染；方案二：渲染帧设标记位，逻辑帧读取标记位进行传输。影响都是至多一逻辑帧，看情况选择。缺点：要复制的东西可能**有点**多。
[ ] handler 修改接口，明确移动操作是在 core 进行的
[ ] 验证 pc, hpc 等等，实现炸弹垃圾行、死行（可选地：为 mino 添加 attribute 设置）

## UI

[x] 写一个 label 并调试通过
[ ] 实现 interactive 的 hover 和 focus 部分（tab 调焦点、点击调焦点）
[ ] 实现 interactive 可选的 onKeyPressed, onClick 等等
[x] 写 richtext
[ ] 写 color
[ ] 对自定义字体的支持

## 设计

[ ] pp 状物计算
[ ] 段位线
[ ] 成就系统

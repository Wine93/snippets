Echo
===

A simple echo server for learning purpose


## TODO
* [ ] 参考 Nginx 如何处理 half-close 的连接，以及 c->error/c->close 这几个标志位的应用
* [ ] 参考 Nginx 的连接池实现，并在 Echo 中实现
* [ ] 理解 Nginx 的 ngx_http_test_reading 函数应用场景，并应用到 Echo
* [ ] listen 调用中 backlog 参数的理解及实验
* [ ] CLOSE_WAIT 状态过多如何处理，参考 https://huoding.com/2016/01/19/488
* [ ] 如何构造高并发测试?
* [ ] signal vs sigaction?
* [ ] 信号阻塞、忽略相关概念理解?

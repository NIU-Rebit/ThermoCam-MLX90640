# CH341SER Linux Drive

## 安裝

```sh
# 編譯
make

# 安裝驅動
make load

# 卸載驅動
make unload
```

## 問題

錯誤訊息：
```sh
error: unknown type name ‘wait_queue_t’; did you mean ‘wait_event’?
```
解決方法：
```python
# 開頭添加此標頭檔
#include <linux/sched/signal.h>

# 第592行，將此行註解掉
wait_queue_t wait
```

## 參考

[CH341SER_LINUX.ZIP](http://www.wch.cn/download/CH341SER_LINUX_ZIP.html)

[ubuntu 无法识别HL340 串口的解决方法](https://blog.csdn.net/feidaji/article/details/107149891)
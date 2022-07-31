# 从计算机本源深入探寻volatile关键字

## 为什么我们需要volatile

### 保证数据的可见性

假如现在有两个线程分别执行不同的代码，但是他们有同一个共享变量`flag`，其中线程`updater`会执行的代码是将`flag`从`false`修改成`true`，而另外一个线程`reader`会进行`while`循环，当`flag`为`true`的时候跳出循环，代码如下：

```java
import java.util.concurrent.TimeUnit;

class Resource {
    public boolean flag;

    public void update() {
        flag = true;
    }
}

public class Visibility {

    public static void main(String[] args) throws InterruptedException {
        Resource resource = new Resource();
        Thread thread = new Thread(() -> {
            System.out.println(resource.flag);
            try {
                TimeUnit.SECONDS.sleep(1);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            resource.update();
        }, "updater");

        new Thread(() -> {
            System.out.println(resource.flag);
            while (!resource.flag) {

            }
            System.out.println("循环结束");
        }, "reader").start();

        thread.start();
    }
}
```

运行上面的代码你会发现，`reader`线程始终打印不出`循环结束`，也就是说它一只在进行`while`循环，而进行`while`循环的原因就是`resouce.flag=false`，但是线程`updater`在经过1秒之后会进行更新啊！为什么`reader`线程还读取不到呢？


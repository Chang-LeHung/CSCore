# 自己动手写ls命令——Java版

## 介绍

在前面的文章[Linux命令系列之ls——原来最简单的ls这么复杂](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247486970&idx=1&sn=fa7635fbad831a1081ce789b59a45e2a&chksm=cf0c91f3f87b18e5cbaf2e61b3115141f33a480a132546e10021ff48bc0705cbbec4549c5072&token=952740120&lang=zh_CN#rd)当中，我们仔细的介绍了关于ls命令的使用和输出结果，在本篇文章当中我们用Java代码自己实现ls命令，更加深入的了解ls命令。

## 代码实现

### 文件操作的基本原理

如果我们使用Java实现一个简单的ls命令其实并不难，因为Java已经给我们提供了一些比较方便和文件系统相关的api了，困难的是理解api是在做什么事儿！

事实上这些api都是操作系统给我们提供的，然后Java进行了一些列的封装，将这些操作给我们进行提供，我们仔细来看一下封装的层次，首先操作系统会给我们提供很多系统调用用于和设备（磁盘、CPU）进行交互，比如说和文件的交互就是读写数据，当然我们的Java程序也需要这些操作，因此JVM也需要给我们提供这些操作，因此JVM就对系统调用进行了一系列的封装，在Java当中具体的形式就是用native修饰的方法。

<img src="../../images/linux/command/25.png" alt="25" style="zoom:50%;" />

如果你是一个比较有经验Java程序员那么一定见过Java当中的`native`方法，这些方法都是Java给我们封装的底层接口，比如说在`FileInputStream`当中有一个`read`方法，这个方法就是读取文件当中的内容，我们看一下这个方法是如何实现的：

```java
    public int read() throws IOException {
        return read0();
    }
```

这里让大家的感受更加深入一点😂，我在这里贴一张`FileInputStream`的源代码图片：

<img src="../../images/linux/command/26.png" alt="25" style="zoom:50%;" />

从上面的图看当我们调用`FileInputStream`方法的时候确实调用了native方法。我们再来看一些与文件操作相关的api，他们也是使用Java给我们封装的native方法实现的。

<img src="../../images/linux/command/27.png" alt="25" style="zoom:50%;" />

上面主要谈了一些基本的文件操作过程的原理，简要说明了Java将很多系统操作封装成native方法供我们调用，现在我们来看看要想实现ls命令，我们需要哪些api。

### 查看一个目录下面有哪些文件和目录

在Java当中给我们提供了一个类`File`，我们可以使用这个类去得到一个目录下面有哪些文件和目录。

```java
  public void fileTest() {
    File file = new File("./");
    // file.listFiles() 将当前 file 对应的目录下所有的文件和目录都得到
    for (File listFile : file.listFiles()) {
      System.out.println(listFile.getName()); // 将文件或者目录的名字打印
    }
```

### 查看文件和目录的元数据

在Java当中给我们提供了一个工具类查看文件的一些元信息(metadata)，比如说文件的uid（用户id）、gid（用户组id）、文件的大小和文件的链接数目(nlink)。

```java
  Path path = Paths.get(".");
  System.out.println(Files.getAttribute(path, "unix:dev")); // 打印存储当前目录数据的设备的设备id
  System.out.println(Files.getAttribute(path, "unix:ino")); // 打印存储当前目录数据inode号
  System.out.println(Files.getAttribute(path, "unix:mode"));// 打印存储当前目录数据的mode数据 这个数据主要用于表示文件的类型
  System.out.println(Files.getAttribute(path, "unix:uid")); // 打印存储当前目录所属用户的用户id
  System.out.println(Files.getAttribute(path, "unix:gid")); // 打印存储当前目录所属组的组id
  System.out.println(Files.getAttribute(path, "unix:size"));// 打印存储当前目录数据所占的空间大小
  System.out.println(Files.getAttribute(path, "unix:nlink"));// 打印存储当前目录数据的链接数
```

除了上面的方式，我们还可以使用下面的方式去得到文件的元数据：

```java
  public void attrTest02() throws IOException {
    Path path = Paths.get(".");
    PosixFileAttributes attr = Files.readAttributes(path, PosixFileAttributes.class, NOFOLLOW_LINKS);
    System.out.println(attr.owner());
    System.out.println(attr.group());
    System.out.println(attr.isRegularFile());
    System.out.println(attr.isSymbolicLink());
    System.out.println(attr.isDirectory());
    System.out.println(attr.isOther());
    System.out.println(attr.permissions());
    System.out.println(attr.lastAccessTime());
    System.out.println(attr.creationTime());
    System.out.println(attr.lastModifiedTime());
    System.out.println(attr.fileKey());
    System.out.println(attr.size());

  }
```

```java
xxxxx // 这里是用户名
xxxxx // 这里是用户组名
false
false
true
false
[GROUP_READ, OTHERS_EXECUTE, OWNER_WRITE, OWNER_EXECUTE, OTHERS_READ, OWNER_READ, GROUP_EXECUTE]
2022-10-09T18:08:47.791072133Z
2022-10-09T13:10:51Z
2022-10-09T18:08:23.746949182Z
(dev=1000012,ino=16176823)
192

```



```java
package java.nio.file.attribute;

public enum PosixFilePermission {

    /**
     * Read permission, owner.
     */
    OWNER_READ,

    /**
     * Write permission, owner.
     */
    OWNER_WRITE,

    /**
     * Execute/search permission, owner.
     */
    OWNER_EXECUTE,

    /**
     * Read permission, group.
     */
    GROUP_READ,

    /**
     * Write permission, group.
     */
    GROUP_WRITE,

    /**
     * Execute/search permission, group.
     */
    GROUP_EXECUTE,

    /**
     * Read permission, others.
     */
    OTHERS_READ,

    /**
     * Write permission, others.
     */
    OTHERS_WRITE,

    /**
     * Execute/search permission, others.
     */
    OTHERS_EXECUTE;
}

```



```java
package cscore.linux.command;


import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFileAttributes;
import java.nio.file.attribute.PosixFilePermission;
import java.util.Objects;
import java.util.Set;

import static java.nio.file.LinkOption.NOFOLLOW_LINKS;

public class LS {

  public static boolean hasRight(Set<PosixFilePermission> set, PosixFilePermission
                                 permission) {
    return set.contains(permission);
  }

  public static void echoCharacter(Set<PosixFilePermission> set) {
    // user
    if (hasRight(set, PosixFilePermission.OWNER_READ))
      System.out.print('r');
      else
      System.out.print('-');
    if (hasRight(set, PosixFilePermission.OWNER_WRITE))
      System.out.print('w');
    else
      System.out.print('-');
    if (hasRight(set, PosixFilePermission.OWNER_EXECUTE))
      System.out.print('x');
    else
      System.out.print('-');

    // group
    if (hasRight(set, PosixFilePermission.GROUP_READ))
      System.out.print('r');
    else
      System.out.print('-');
    if (hasRight(set, PosixFilePermission.GROUP_WRITE))
      System.out.print('w');
    else
      System.out.print('-');
    if (hasRight(set, PosixFilePermission.GROUP_EXECUTE))
      System.out.print('x');
    else
      System.out.print('-');

    // others
    if (hasRight(set, PosixFilePermission.OTHERS_READ))
      System.out.print('r');
    else
      System.out.print('-');
    if (hasRight(set, PosixFilePermission.OTHERS_WRITE))
      System.out.print('w');
    else
      System.out.print('-');
    if (hasRight(set, PosixFilePermission.OTHERS_EXECUTE))
      System.out.print('x');
    else
      System.out.print('-');
  }

  public static void echoType(PosixFileAttributes attributes) {
    if (attributes.isDirectory())
      System.out.print('d');
    else if (attributes.isRegularFile())
      System.out.print('-');
    else if (attributes.isSymbolicLink())
      System.out.print('l');
    else
      System.out.print('o');
  }

  public static void echoFileInformation(String args) throws IOException {
    Path path = Paths.get(args);
    PosixFileAttributes attributes = Files.readAttributes(path, PosixFileAttributes.class, NOFOLLOW_LINKS);
    echoType(attributes);
    echoCharacter(attributes.permissions());

    System.out.printf("\t%-2d", Files.getAttribute(path, "unix:nlink"));
    System.out.print("\t" + attributes.owner().getName());
    System.out.print("\t" + attributes.group().getName());
    System.out.printf("\t%-5d", attributes.size());
    System.out.printf("\t %10s", attributes.lastAccessTime());
    System.out.println("\t" + path.getFileName());
  }

  public static void main(String[] args) throws IOException {

    File file = new File(args[0]);
    for (File listFile : Objects.requireNonNull(file.listFiles())) {
      echoFileInformation(listFile.toString());
    }
  }
}

```



![24](../../images/linux/command/28.png)
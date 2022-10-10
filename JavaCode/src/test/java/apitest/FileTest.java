package apitest;

import org.junit.Test;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFileAttributes;
import java.util.Objects;

import static java.nio.file.LinkOption.NOFOLLOW_LINKS;

public class FileTest {

  @Test
  public void fileTest() {
    File file = new File("./");
    for (File listFile : Objects.requireNonNull(file.listFiles())) {
      System.out.println(listFile.getName());
    }
  }

  @Test
  public void attrTest() throws IOException {
    Path path = Paths.get(".");
    System.out.println(Files.getAttribute(path, "unix:dev"));
    System.out.println(Files.getAttribute(path, "unix:ino"));
    System.out.println(Files.getAttribute(path, "unix:mode"));
    System.out.println(Files.getAttribute(path, "unix:uid"));
    System.out.println(Files.getAttribute(path, "unix:gid"));
    System.out.println(Files.getAttribute(path, "unix:size"));
    System.out.println(Files.getAttribute(path, "unix:nlink"));
  }

  @Test
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
}

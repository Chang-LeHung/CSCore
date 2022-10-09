package apitest;

import org.junit.Test;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Objects;

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
}
